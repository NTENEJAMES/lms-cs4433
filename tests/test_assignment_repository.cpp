#include <gtest/gtest.h>
#include "Exceptions.h"
#include "SqliteAssignmentRepository.h"
#include "SqliteCourseRepository.h"
#include "SqliteUserRepository.h"
#include "TimeUtil.h"
#include "UserFactory.h"

class AssignmentRepositoryTest : public ::testing::Test {
protected:
    AssignmentRepositoryTest() : db(":memory:"), users(db), courses(db), assignments(db) {
        db.executeFile(LMS_SCHEMA_PATH);
        student = users.save(*UserFactory::create("STUDENT", 0, "stu", "h", "s", "Stu"));
        lecturer = users.save(*UserFactory::create("LECTURER", 0, "lec", "h", "s", "Lec"));
        db.execute("INSERT INTO departments(name) VALUES('Science');");
        courseId = courses.save(Course(0, "SC1", "Biology", 1));
    }
    Database db;
    SqliteUserRepository users;
    SqliteCourseRepository courses;
    SqliteAssignmentRepository assignments;
    int student, lecturer, courseId;
    const TimePoint deadline = parseIso("2026-11-20T23:59:00");
};

TEST_F(AssignmentRepositoryTest, AssignmentRoundTrips) {
    const int id = assignments.save(Assignment(0, courseId, "Essay", deadline, 50));
    auto a = assignments.findById(id);
    ASSERT_TRUE(a.has_value());
    EXPECT_EQ(a->title(), "Essay");
    EXPECT_EQ(a->deadline(), deadline);
    EXPECT_EQ(a->maxMark(), 50);
    EXPECT_FALSE(assignments.findById(999).has_value());
}

TEST_F(AssignmentRepositoryTest, UpdateChangesTheAssignment) {
    const int id = assignments.save(Assignment(0, courseId, "Essay", deadline, 50));
    assignments.save(Assignment(id, courseId, "Essay 2", deadline, 60));
    EXPECT_EQ(assignments.findById(id)->title(), "Essay 2");
    EXPECT_EQ(assignments.findById(id)->maxMark(), 60);
}

TEST_F(AssignmentRepositoryTest, ExtensionsAreLoadedWithTheAssignment) {
    const int id = assignments.save(Assignment(0, courseId, "Essay", deadline, 50));
    assignments.saveExtension(id, student, deadline + std::chrono::hours(48), "Illness", lecturer);
    auto a = assignments.findById(id);
    EXPECT_EQ(a->deadlineFor(student), deadline + std::chrono::hours(48));
    EXPECT_EQ(a->deadlineFor(student + 100), deadline);
}

TEST_F(AssignmentRepositoryTest, SecondExtensionReplacesTheFirst) {
    const int id = assignments.save(Assignment(0, courseId, "Essay", deadline, 50));
    assignments.saveExtension(id, student, deadline + std::chrono::hours(24), "", lecturer);
    assignments.saveExtension(id, student, deadline + std::chrono::hours(72), "More time", lecturer);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM extensions"), 1);
    EXPECT_EQ(assignments.findById(id)->deadlineFor(student), deadline + std::chrono::hours(72));
}

TEST_F(AssignmentRepositoryTest, ExtensionForMissingAssignmentIsRejectedByTheDatabase) {
    EXPECT_THROW(assignments.saveExtension(999, student, deadline + std::chrono::hours(24), "", lecturer),
                 DatabaseException);
}

TEST_F(AssignmentRepositoryTest, CoursesAndEnrolments) {
    EXPECT_FALSE(courses.isEnrolled(student, courseId));
    courses.enrol(student, courseId);
    EXPECT_TRUE(courses.isEnrolled(student, courseId));
    EXPECT_THROW(courses.enrol(student, courseId), DatabaseException);       // already enrolled
    EXPECT_THROW(courses.enrol(student, 999), DatabaseException);            // no such course
    auto c = courses.findById(courseId);
    ASSERT_TRUE(c.has_value());
    EXPECT_FALSE(c->lecturerId().has_value());
    c->assignLecturer(lecturer);
    courses.save(*c);
    EXPECT_EQ(*courses.findById(courseId)->lecturerId(), lecturer);
}
