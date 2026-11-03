#include <gtest/gtest.h>
#include "Exceptions.h"
#include "SqliteAssignmentRepository.h"
#include "SqliteCourseRepository.h"
#include "SqliteSubmissionRepository.h"
#include "SqliteUserRepository.h"
#include "SubmissionService.h"
#include "TimeUtil.h"
#include "UserFactory.h"

using std::chrono::hours;
using std::chrono::seconds;

// Fresh database with: course 1 (assignment 1, deadline 2026-11-20 23:59, max 50),
// students "anna" (enrolled), "ben" (enrolled), "carl" (NOT enrolled), lecturer "lec"
class SubmissionServiceTest : public ::testing::Test {
protected:
    SubmissionServiceTest()
        : db(":memory:"), users(db), assignments(db), courses(db), submissions(db),
          service(assignments, courses, submissions) {
        db.executeFile(LMS_SCHEMA_PATH);
        anna = addUser("anna", "STUDENT");
        ben = addUser("ben", "STUDENT");
        carl = addUser("carl", "STUDENT");
        lecturer = addUser("lec", "LECTURER");
        db.execute("INSERT INTO departments(name) VALUES('Science');");
        courseId = courses.save(Course(0, "SC1", "Biology", 1, lecturer));
        courses.enrol(anna, courseId);
        courses.enrol(ben, courseId);
        deadline = parseIso("2026-11-20T23:59:00");
        assignmentId = assignments.save(Assignment(0, courseId, "Essay", deadline, 50));
    }
    int addUser(const std::string& name, const std::string& role) {
        return users.save(*UserFactory::create(role, 0, name, "h", "s", "Name " + name));
    }
    Database db;
    SqliteUserRepository users;
    SqliteAssignmentRepository assignments;
    SqliteCourseRepository courses;
    SqliteSubmissionRepository submissions;
    SubmissionService service;
    int anna, ben, carl, lecturer, courseId, assignmentId;
    TimePoint deadline;
};

TEST_F(SubmissionServiceTest, OnTimeSubmissionIsSavedAsSubmitted) {
    auto s = service.submit(anna, assignmentId, "my essay", deadline - hours(3));
    EXPECT_GT(s.submissionId(), 0);
    EXPECT_EQ(s.status(), SubmissionStatus::Submitted);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM submissions"), 1);
}

TEST_F(SubmissionServiceTest, SubmittingExactlyAtTheDeadlineIsOnTime) {
    EXPECT_EQ(service.submit(anna, assignmentId, "x", deadline).status(), SubmissionStatus::Submitted);
}

TEST_F(SubmissionServiceTest, OneSecondAfterTheDeadlineIsLateByOneDay) {
    auto s = service.submit(anna, assignmentId, "x", deadline + seconds(1));
    EXPECT_EQ(s.status(), SubmissionStatus::Late);
    EXPECT_EQ(s.daysLate(), 1);
}

TEST_F(SubmissionServiceTest, LateWorkIsAcceptedNotRejected) {
    auto s = service.submit(anna, assignmentId, "very late", deadline + hours(24 * 5));
    EXPECT_EQ(s.status(), SubmissionStatus::Late);
    EXPECT_EQ(s.daysLate(), 5);          // the penalty rule will give it zero when marked
}

TEST_F(SubmissionServiceTest, ResubmissionBeforeDeadlineReplacesTheWork) {
    auto first = service.submit(anna, assignmentId, "draft", deadline - hours(10));
    auto second = service.submit(anna, assignmentId, "final", deadline - hours(2));
    EXPECT_EQ(second.submissionId(), first.submissionId());     // same record, replaced
    EXPECT_EQ(second.content(), "final");
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM submissions"), 1);
}

TEST_F(SubmissionServiceTest, ResubmissionAfterDeadlineIsRefusedAndOldWorkKept) {
    service.submit(anna, assignmentId, "original", deadline - hours(1));
    EXPECT_THROW(service.submit(anna, assignmentId, "too late", deadline + hours(1)), InvalidInputException);
    EXPECT_EQ(submissions.findByAssignment(assignmentId, anna)->content(), "original");
}

TEST_F(SubmissionServiceTest, LateSubmissionCannotBeResubmitted) {
    service.submit(anna, assignmentId, "late work", deadline + hours(5));
    EXPECT_THROW(service.submit(anna, assignmentId, "again", deadline + hours(6)), InvalidInputException);
}

TEST_F(SubmissionServiceTest, ExtensionMovesTheDeadlineForThatStudentOnly) {
    assignments.saveExtension(assignmentId, anna, deadline + hours(48), "Illness", lecturer);
    // 24 hours after the normal deadline:
    EXPECT_EQ(service.submit(anna, assignmentId, "x", deadline + hours(24)).status(), SubmissionStatus::Submitted);
    auto bens = service.submit(ben, assignmentId, "x", deadline + hours(24));
    EXPECT_EQ(bens.status(), SubmissionStatus::Late);
}

TEST_F(SubmissionServiceTest, PenaltyIsCountedFromTheExtendedDeadline) {
    assignments.saveExtension(assignmentId, anna, deadline + hours(48), "Illness", lecturer);
    auto s = service.submit(anna, assignmentId, "x", deadline + hours(50));
    EXPECT_EQ(s.daysLate(), 1);          // 2 hours after the extended deadline, not 50 hours
}

TEST_F(SubmissionServiceTest, StudentNotEnrolledIsDenied) {
    EXPECT_THROW(service.submit(carl, assignmentId, "x", deadline - hours(1)), AccessDeniedException);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM submissions"), 0);
}

TEST_F(SubmissionServiceTest, EmptyWorkAndUnknownAssignmentAreRejectedAndNothingIsSaved) {
    EXPECT_THROW(service.submit(anna, assignmentId, "", deadline - hours(1)), InvalidInputException);
    EXPECT_THROW(service.submit(anna, 999, "x", deadline - hours(1)), InvalidInputException);
    EXPECT_EQ(db.countRows("SELECT COUNT(*) FROM submissions"), 0);
}
