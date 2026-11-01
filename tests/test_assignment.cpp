#include <gtest/gtest.h>
#include "Assignment.h"
#include "Course.h"
#include "Department.h"
#include "Exceptions.h"

using std::chrono::hours;

static const TimePoint kDeadline = std::chrono::system_clock::now();

TEST(AssignmentTest, RejectsEmptyTitleAndBadMaxMark) {
    EXPECT_THROW(Assignment(1, 1, "", kDeadline, 50), InvalidInputException);
    EXPECT_THROW(Assignment(1, 1, "Essay", kDeadline, 0), InvalidInputException);
}

TEST(AssignmentTest, NormalDeadlineAppliesWithoutExtension) {
    Assignment a(1, 1, "Essay", kDeadline, 50);
    EXPECT_EQ(a.deadlineFor(10), kDeadline);
    EXPECT_FALSE(a.isPastDeadline(kDeadline, 10));              // exactly at deadline = on time
    EXPECT_TRUE(a.isPastDeadline(kDeadline + std::chrono::seconds(1), 10));
}

TEST(AssignmentTest, ExtensionAppliesOnlyToThatStudent) {
    Assignment a(1, 1, "Essay", kDeadline, 50);
    a.addExtension(10, kDeadline + hours(48));
    EXPECT_EQ(a.deadlineFor(10), kDeadline + hours(48));
    EXPECT_EQ(a.deadlineFor(11), kDeadline);
    EXPECT_FALSE(a.isPastDeadline(kDeadline + hours(24), 10));
    EXPECT_TRUE(a.isPastDeadline(kDeadline + hours(24), 11));
}

TEST(AssignmentTest, ExtensionMustBeLaterThanDeadline) {
    Assignment a(1, 1, "Essay", kDeadline, 50);
    EXPECT_THROW(a.addExtension(10, kDeadline), InvalidInputException);
    EXPECT_THROW(a.addExtension(10, kDeadline - hours(1)), InvalidInputException);
}

TEST(CourseTest, LecturerIsOptionalUntilAssigned) {
    Course c(1, "SC1", "Biology", 1);
    EXPECT_FALSE(c.lecturerId().has_value());
    c.assignLecturer(5);
    EXPECT_EQ(*c.lecturerId(), 5);
    EXPECT_THROW(Course(1, "", "Biology", 1), InvalidInputException);
}

TEST(DepartmentTest, NameRequired) {
    EXPECT_THROW(Department(1, ""), InvalidInputException);
    EXPECT_FALSE(Department(1, "Science").hodUserId().has_value());
}
