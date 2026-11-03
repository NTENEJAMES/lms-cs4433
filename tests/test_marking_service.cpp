#include <gtest/gtest.h>
#include "Exceptions.h"
#include "MarkingService.h"
#include "SqliteAssignmentRepository.h"
#include "SqliteAuditRepository.h"
#include "SqliteCourseRepository.h"
#include "SqliteSubmissionRepository.h"
#include "SqliteTransactionManager.h"
#include "SqliteUserRepository.h"
#include "SubmissionService.h"
#include "TenPercentPerDayPolicy.h"
#include "TimeUtil.h"
#include "UserFactory.h"

using std::chrono::hours;

// Assignment 1 (max 50, deadline 2026-11-20 23:59) in a course taught by "lec".
// "other" is a lecturer of a different course.
class MarkingServiceTest : public ::testing::Test {
protected:
    MarkingServiceTest()
        : db(":memory:"), users(db), assignments(db), courses(db), submissions(db), audit(db), tx(db),
          submit(assignments, courses, submissions),
          marking(submissions, assignments, courses, audit, tx, policy) {
        db.executeFile(LMS_SCHEMA_PATH);
        anna = addUser("anna", "STUDENT");
        lec = addUser("lec", "LECTURER");
        other = addUser("other", "LECTURER");
        db.execute("INSERT INTO departments(name) VALUES('Science');");
        courseId = courses.save(Course(0, "SC1", "Biology", 1, lec));
        courses.enrol(anna, courseId);
        deadline = parseIso("2026-11-20T23:59:00");
        assignmentId = assignments.save(Assignment(0, courseId, "Essay", deadline, 50));
        now = deadline + hours(200);
    }
    int addUser(const std::string& name, const std::string& role) {
        return users.save(*UserFactory::create(role, 0, name, "h", "s", "Name " + name));
    }
    int submitAt(TimePoint when) { return submit.submit(anna, assignmentId, "work", when).submissionId(); }

    Database db;
    SqliteUserRepository users;
    SqliteAssignmentRepository assignments;
    SqliteCourseRepository courses;
    SqliteSubmissionRepository submissions;
    SqliteAuditRepository audit;
    SqliteTransactionManager tx;
    TenPercentPerDayPolicy policy;
    SubmissionService submit;
    MarkingService marking;
    int anna, lec, other, courseId, assignmentId;
    TimePoint deadline, now;
};

TEST_F(MarkingServiceTest, MarkingSetsMarkedAndWritesAnAuditEntry) {
    const int id = submitAt(deadline - hours(2));
    auto s = marking.mark(lec, id, 45, "Well done", now);
    EXPECT_EQ(s.status(), SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*s.finalMark(), 45);
    EXPECT_EQ(s.feedback(), "Well done");
    EXPECT_FALSE(s.isVisibleToStudent());                      // hidden until the HoD releases it

    auto log = audit.findBySubmission(id);
    ASSERT_EQ(log.size(), 1u);
    EXPECT_EQ(log[0].changedBy(), lec);
    EXPECT_EQ(log[0].changedAt(), now);
    EXPECT_FALSE(log[0].oldValue().has_value());               // first mark has no old value
    EXPECT_DOUBLE_EQ(log[0].newValue(), 45);
    EXPECT_EQ(log[0].reason(), "Initial marking");
}

TEST_F(MarkingServiceTest, LatePenaltyIsAppliedAndTheFinalMarkIsLogged) {
    const int id = submitAt(deadline + hours(30));             // 2 days late
    auto s = marking.mark(lec, id, 40, "Late but good", now);
    EXPECT_DOUBLE_EQ(*s.rawMark(), 40);
    EXPECT_DOUBLE_EQ(*s.finalMark(), 30);                      // 40 - 2 x 5
    EXPECT_DOUBLE_EQ(audit.findBySubmission(id)[0].newValue(), 30);
}

TEST_F(MarkingServiceTest, MoreThanThreeDaysLateGivesZero) {
    const int id = submitAt(deadline + hours(24 * 4));         // 4 days late
    EXPECT_DOUBLE_EQ(*marking.mark(lec, id, 50, "x", now).finalMark(), 0);
}

TEST_F(MarkingServiceTest, MarkAboveMaximumIsRejectedAndNothingIsSaved) {
    const int id = submitAt(deadline - hours(2));
    EXPECT_THROW(marking.mark(lec, id, 51, "x", now), InvalidInputException);
    EXPECT_EQ(submissions.findById(id)->status(), SubmissionStatus::Submitted);
    EXPECT_TRUE(audit.findBySubmission(id).empty());
}

TEST_F(MarkingServiceTest, OnlyTheLecturerOfTheCourseMayMark) {
    const int id = submitAt(deadline - hours(2));
    EXPECT_THROW(marking.mark(other, id, 40, "x", now), AccessDeniedException);
    EXPECT_THROW(marking.mark(anna, id, 40, "x", now), AccessDeniedException);
    EXPECT_EQ(submissions.findById(id)->status(), SubmissionStatus::Submitted);
}

TEST_F(MarkingServiceTest, UnknownSubmissionIsRejected) {
    EXPECT_THROW(marking.mark(lec, 999, 40, "x", now), InvalidInputException);
}

TEST_F(MarkingServiceTest, ChangingAMarkNeedsAReasonAndIsLogged) {
    const int id = submitAt(deadline - hours(2));
    marking.mark(lec, id, 40, "First look", now);
    auto s = marking.changeMark(lec, id, 44, "Missed a correct answer", now + hours(1));
    EXPECT_DOUBLE_EQ(*s.finalMark(), 44);
    EXPECT_EQ(s.feedback(), "First look");                     // feedback is kept

    auto log = audit.findBySubmission(id);
    ASSERT_EQ(log.size(), 2u);
    EXPECT_DOUBLE_EQ(*log[1].oldValue(), 40);
    EXPECT_DOUBLE_EQ(log[1].newValue(), 44);
    EXPECT_EQ(log[1].reason(), "Missed a correct answer");
}

TEST_F(MarkingServiceTest, ChangeWithoutReasonIsRefusedAndNothingChanges) {
    const int id = submitAt(deadline - hours(2));
    marking.mark(lec, id, 40, "x", now);
    EXPECT_THROW(marking.changeMark(lec, id, 44, "", now), InvalidInputException);
    EXPECT_DOUBLE_EQ(*submissions.findById(id)->finalMark(), 40);
    EXPECT_EQ(audit.findBySubmission(id).size(), 1u);
}

TEST_F(MarkingServiceTest, MarkTwiceAndChangeBeforeMarkingAreRefused) {
    const int id = submitAt(deadline - hours(2));
    EXPECT_THROW(marking.changeMark(lec, id, 40, "reason", now), InvalidInputException);   // not marked yet
    marking.mark(lec, id, 40, "x", now);
    EXPECT_THROW(marking.mark(lec, id, 45, "x", now), InvalidInputException);              // use changeMark
}

TEST_F(MarkingServiceTest, LecturerCannotChangeAModeratedMark) {
    const int id = submitAt(deadline - hours(2));
    marking.mark(lec, id, 40, "x", now);
    auto s = *submissions.findById(id);
    s.approve();
    submissions.save(s);
    EXPECT_THROW(marking.changeMark(lec, id, 50, "I changed my mind", now), InvalidInputException);
    EXPECT_DOUBLE_EQ(*submissions.findById(id)->finalMark(), 40);
}

// If writing the audit entry fails, the new mark must not be saved either.
namespace {
class FailingAuditRepository : public AuditRepository {
public:
    int add(const AuditEntry&) override { throw DatabaseException("disk full"); }
    std::vector<AuditEntry> findBySubmission(int) const override { return {}; }
};
}

TEST_F(MarkingServiceTest, MarkAndAuditEntryAreSavedTogetherOrNotAtAll) {
    const int id = submitAt(deadline - hours(2));
    FailingAuditRepository failing;
    MarkingService broken(submissions, assignments, courses, failing, tx, policy);
    EXPECT_THROW(broken.mark(lec, id, 40, "x", now), DatabaseException);
    EXPECT_EQ(submissions.findById(id)->status(), SubmissionStatus::Submitted);   // rolled back
    EXPECT_FALSE(submissions.findById(id)->finalMark().has_value());
}
