#include <gtest/gtest.h>
#include <memory>
#include <sstream>
#include "Exceptions.h"
#include "Roles.h"

TEST(UserTest, RolesMatchDatabaseValues) {
    EXPECT_EQ(Administrator(1, "a", "h", "s", "A").role(), "ADMIN");
    EXPECT_EQ(Lecturer(2, "l", "h", "s", "L").role(), "LECTURER");
    EXPECT_EQ(HeadOfDepartment(3, "h", "h", "s", "H", 7).role(), "HOD");
    EXPECT_EQ(Student(4, "s", "h", "s", "S").role(), "STUDENT");
}

TEST(UserTest, HeadOfDepartmentKnowsItsDepartment) {
    EXPECT_EQ(HeadOfDepartment(3, "h", "h", "s", "H", 7).departmentId(), 7);
}

TEST(UserTest, EmptyUsernameOrNameIsRejected) {
    EXPECT_THROW(Student(1, "", "h", "s", "Name"), InvalidInputException);
    EXPECT_THROW(Student(1, "user", "h", "s", ""), InvalidInputException);
}

TEST(UserTest, PolymorphismGivesEachRoleItsOwnMenu) {
    std::unique_ptr<User> users[] = {
        std::make_unique<Administrator>(1, "a", "h", "s", "A"),
        std::make_unique<Lecturer>(2, "l", "h", "s", "L"),
        std::make_unique<HeadOfDepartment>(3, "h", "h", "s", "H", 1),
        std::make_unique<Student>(4, "s", "h", "s", "S")};
    std::string menus[4];
    for (int i = 0; i < 4; ++i) {
        std::ostringstream out;
        users[i]->showMenu(out);
        menus[i] = out.str();
    }
    EXPECT_NE(menus[0].find("Manage users"), std::string::npos);
    EXPECT_NE(menus[1].find("Mark submission"), std::string::npos);
    EXPECT_NE(menus[2].find("Release marks"), std::string::npos);
    EXPECT_NE(menus[3].find("Submit assignment"), std::string::npos);
    // role-based access: a student never sees a marking or admin option
    EXPECT_EQ(menus[3].find("Mark submission"), std::string::npos);
    EXPECT_EQ(menus[3].find("Manage users"), std::string::npos);
}
