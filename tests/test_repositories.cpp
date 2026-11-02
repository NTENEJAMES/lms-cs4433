#include <gtest/gtest.h>
#include "Exceptions.h"
#include "SqliteSubmissionRepository.h"
#include "SqliteUserRepository.h"
#include "TenPercentPerDayPolicy.h"
#include "TimeUtil.h"
#include "UserFactory.h"

// Every test gets a fresh in-memory database with the real schema
class RepositoryTest : public ::testing::Test {
protected:
    RepositoryTest() : db(":memory:"), users(db), submissions(db) { db.executeFile(LMS_SCHEMA_PATH); }

    int addUser(const std::string& name, const std::string& role = "STUDENT") {
        return users.save(*UserFactory::create(role, 0, name, "hash", "salt", "Name " + name));
    }
    // Creates department, course and assignment (id 1 each) and returns the assignment id
    int addAssignment() {
        db.execute("INSERT INTO departments(name) VALUES('Science');");
        db.execute("INSERT INTO courses(code,title,department_id) VALUES('SC1','Biology',1);");
        db.execute("INSERT INTO assignments(course_id,title,deadline,max_mark) "
                   "VALUES(1,'Essay','2026-11-20T23:59:00',50);");
        return 1;
    }
    Database db;
    SqliteUserRepository users;
    SqliteSubmissionRepository submissions;
    const TimePoint deadline = parseIso("2026-11-20T23:59:00");
    const TenPercentPerDayPolicy policy;
};

// ---------------- UserFactory ----------------
TEST_F(RepositoryTest, FactoryRejectsUnknownRole) {
    EXPECT_THROW(UserFactory::create("BOSS", 0, "x", "h", "s", "X"), InvalidInputException);
}

// ---------------- UserRepository ----------------
TEST_F(RepositoryTest, SavedUserCanBeFoundWithCorrectRole) {
    const int id = addUser("lec1", "LECTURER");
    EXPECT_GT(id, 0);
    auto user = users.findByUsername("lec1");
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->role(), "LECTURER");
    EXPECT_NE(dynamic_cast<Lecturer*>(user.get()), nullptr);   // polymorphism: real subclass
    EXPECT_EQ(user->userId(), id);
    EXPECT_EQ(user->passwordHash(), "hash");
}

TEST_F(RepositoryTest, UnknownUserReturnsNull) {
    EXPECT_EQ(users.findByUsername("nobody"), nullptr);
    EXPECT_EQ(users.findById(999), nullptr);
}

TEST_F(RepositoryTest, DuplicateUsernameIsRejected) {
    addUser("same");
    EXPECT_THROW(addUser("same"), DatabaseException);
}

TEST_F(RepositoryTest, UpdateChangesExistingUser) {
    const int id = addUser("stu1");
    users.save(*UserFactory::create("STUDENT", id, "stu1", "newhash", "newsalt", "New Name", false));
    auto user = users.findById(id);
    ASSERT_NE(user, nullptr);
    EXPECT_EQ(user->fullName(), "New Name");
    EXPECT_EQ(user->passwordHash(), "newhash");
    EXPECT_FALSE(user->isActive());
    EXPECT_THROW(users.save(*UserFactory::create("STUDENT", 999, "ghost", "h", "s", "G")), InvalidInputException);
}

TEST_F(RepositoryTest, HeadOfDepartmentLoadsItsDepartment) {
    const int hod = addUser("hod1", "HOD");
    db.execute("INSERT INTO departments(name, hod_user_id) VALUES('Maths', " + std::to_string(hod) + ");");
    auto user = users.findByUsername("hod1");
    auto* h = dynamic_cast<HeadOfDepartment*>(user.get());
    ASSERT_NE(h, nullptr);
    EXPECT_EQ(h->departmentId(), 1);
}

TEST_F(RepositoryTest, SqlInjectionTextIsTreatedAsPlainText) {
    addUser("stu1");
    EXPECT_EQ(users.findByUsername("x' OR '1'='1"), nullptr);
    EXPECT_EQ(users.findByUsername("'; DROP TABLE users; --"), nullptr);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM users"), 1);   // table still there, data intact
}

// ---------------- SubmissionRepository ----------------
TEST_F(RepositoryTest, SavedSubmissionRoundTrips) {
    const int student = addUser("stu1");
    const int assignment = addAssignment();
    Submission s = Submission::createNew(assignment, student, "my essay", deadline + std::chrono::hours(30), deadline);
    const int id = submissions.save(s);
    auto loaded = submissions.findById(id);
    ASSERT_TRUE(loaded.has_value());
    EXPECT_EQ(loaded->content(), "my essay");
    EXPECT_EQ(loaded->status(), SubmissionStatus::Late);
    EXPECT_EQ(loaded->daysLate(), 2);
    EXPECT_EQ(loaded->submittedAt(), s.submittedAt());
    EXPECT_FALSE(loaded->finalMark().has_value());
}

TEST_F(RepositoryTest, MarkingAndReleaseArePersisted) {
    const int student = addUser("stu1");
    const int assignment = addAssignment();
    const int id = submissions.save(Submission::createNew(assignment, student, "work", deadline - std::chrono::hours(1), deadline));

    auto s = submissions.findById(id);
    s->mark(45, 50, "Well done", policy);
    submissions.save(*s);
    s = submissions.findById(id);
    EXPECT_EQ(s->status(), SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*s->finalMark(), 45);
    EXPECT_EQ(s->feedback(), "Well done");

    s->approve();
    s->release(deadline + std::chrono::hours(100));
    submissions.save(*s);
    s = submissions.findById(id);
    EXPECT_EQ(s->status(), SubmissionStatus::Released);
    EXPECT_TRUE(s->isVisibleToStudent());
    EXPECT_EQ(*s->releasedAt(), deadline + std::chrono::hours(100));
}

TEST_F(RepositoryTest, FindByAssignmentListsAndFiltersByStudent) {
    const int a = addUser("a"), b = addUser("b");
    const int assignment = addAssignment();
    submissions.save(Submission::createNew(assignment, a, "A work", deadline, deadline));
    submissions.save(Submission::createNew(assignment, b, "B work", deadline, deadline));
    EXPECT_EQ(submissions.findByAssignment(assignment).size(), 2u);
    auto mine = submissions.findByAssignment(assignment, b);
    ASSERT_TRUE(mine.has_value());
    EXPECT_EQ(mine->content(), "B work");
    EXPECT_FALSE(submissions.findByAssignment(assignment, 999).has_value());
    EXPECT_TRUE(submissions.findByAssignment(999).empty());
}

TEST_F(RepositoryTest, OnlyOneSubmissionPerStudentPerAssignment) {
    const int student = addUser("stu1");
    const int assignment = addAssignment();
    submissions.save(Submission::createNew(assignment, student, "first", deadline, deadline));
    EXPECT_THROW(submissions.save(Submission::createNew(assignment, student, "second", deadline, deadline)),
                 DatabaseException);
}

TEST_F(RepositoryTest, UpdatingMissingSubmissionThrows) {
    Submission ghost(999, 1, 1, "x", deadline, SubmissionStatus::Submitted, 0, std::nullopt, std::nullopt, "", std::nullopt);
    EXPECT_THROW(submissions.save(ghost), InvalidInputException);
}
