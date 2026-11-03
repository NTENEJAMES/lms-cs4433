#include <chrono>
#include <gtest/gtest.h>
#include "AuthService.h"
#include "Exceptions.h"
#include "SqliteUserRepository.h"
#include "UserFactory.h"

namespace {
const PasswordHasher kFast(1, 64, 1);   // cheap settings so the tests run quickly
}

// ---------------- PasswordHasher ----------------
TEST(PasswordHasherTest, CorrectPasswordVerifies) {
    auto r = kFast.hash("Secret123");
    EXPECT_TRUE(kFast.verify("Secret123", r.hash, r.salt));
}

TEST(PasswordHasherTest, WrongPasswordFails) {
    auto r = kFast.hash("Secret123");
    EXPECT_FALSE(kFast.verify("secret123", r.hash, r.salt));
    EXPECT_FALSE(kFast.verify("", r.hash, r.salt));
}

TEST(PasswordHasherTest, SameSaltIsNeverReused) {
    auto a = kFast.hash("Secret123");
    auto b = kFast.hash("Secret123");
    EXPECT_NE(a.salt, b.salt);
    EXPECT_NE(a.hash, b.hash);          // same password, different hash
    EXPECT_EQ(a.salt.size(), 32u);      // 16 bytes as hex
    EXPECT_EQ(a.hash.size(), 64u);      // 32 bytes as hex
}

TEST(PasswordHasherTest, HashDoesNotContainThePassword) {
    auto r = kFast.hash("Secret123");
    EXPECT_EQ(r.hash.find("Secret123"), std::string::npos);
    EXPECT_NE(r.hash, "Secret123");
}

TEST(PasswordHasherTest, BadStoredValuesFailSafely) {
    auto r = kFast.hash("Secret123");
    EXPECT_FALSE(kFast.verify("Secret123", "not hex", r.salt));
    EXPECT_FALSE(kFast.verify("Secret123", r.hash, ""));
    EXPECT_FALSE(kFast.verify("Secret123", r.hash, "abcd"));     // salt too short
    EXPECT_THROW(kFast.hash(""), InvalidInputException);
}

// ---------------- AuthService ----------------
class AuthTest : public ::testing::Test {
protected:
    AuthTest() : db(":memory:"), users(db), auth(users, kFast) {
        db.executeFile(LMS_SCHEMA_PATH);
        addUser("stu1", "STUDENT", "Secret123", true);
        addUser("old1", "LECTURER", "Secret123", false);
    }
    void addUser(const std::string& name, const std::string& role, const std::string& pw, bool active) {
        auto h = kFast.hash(pw);
        users.save(*UserFactory::create(role, 0, name, h.hash, h.salt, "Name " + name, active));
    }
    Database db;
    SqliteUserRepository users;
    AuthService auth;
};

TEST_F(AuthTest, ValidLoginReturnsTheRightUser) {
    auto u = auth.login("stu1", "Secret123");
    ASSERT_NE(u, nullptr);
    EXPECT_EQ(u->role(), "STUDENT");
}

TEST_F(AuthTest, WrongPasswordUnknownUserAndEmptyInputAreRefused) {
    EXPECT_EQ(auth.login("stu1", "wrong"), nullptr);
    EXPECT_EQ(auth.login("nobody", "Secret123"), nullptr);
    EXPECT_EQ(auth.login("", "Secret123"), nullptr);
    EXPECT_EQ(auth.login("stu1", ""), nullptr);
}

TEST_F(AuthTest, DeactivatedAccountCannotLogIn) {
    EXPECT_EQ(auth.login("old1", "Secret123"), nullptr);
}

TEST_F(AuthTest, NoPlainTextPasswordIsStored) {
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM users WHERE password_hash = 'Secret123'"), 0);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM users WHERE password_hash LIKE '%Secret123%'"), 0);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM users WHERE salt = '' OR password_hash = ''"), 0);
}

TEST_F(AuthTest, PasswordRuleIsEnforced) {
    EXPECT_THROW(AuthService::validateNewPassword("short"), InvalidInputException);
    EXPECT_NO_THROW(AuthService::validateNewPassword("long-enough"));
}

// NFR-01: login within 2 seconds with 800 students stored (real, default hashing cost)
TEST(AuthPerformanceTest, LoginWithDefaultCostAndEightHundredUsersIsUnderTwoSeconds) {
    Database db(":memory:");
    db.executeFile(LMS_SCHEMA_PATH);
    db.execute("BEGIN;");
    for (int i = 0; i < 800; ++i)
        db.execute("INSERT INTO users(username,password_hash,salt,full_name,role) VALUES('s" +
                   std::to_string(i) + "','00','00','Student','STUDENT');");
    db.execute("COMMIT;");
    SqliteUserRepository users(db);
    PasswordHasher realHasher;   // default (OWASP) settings
    auto h = realHasher.hash("Secret123");
    users.save(*UserFactory::create("STUDENT", 0, "target", h.hash, h.salt, "Target"));
    AuthService auth(users, realHasher);

    const auto start = std::chrono::steady_clock::now();
    auto u = auth.login("target", "Secret123");
    const auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - start).count();
    ASSERT_NE(u, nullptr);
    EXPECT_LT(ms, 2000) << "login took " << ms << " ms";
}
