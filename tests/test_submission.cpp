#include <gtest/gtest.h>
#include "Exceptions.h"
#include "Submission.h"
#include "TenPercentPerDayPolicy.h"

using std::chrono::hours;

static const TimePoint kDeadline = std::chrono::system_clock::now();
static const TenPercentPerDayPolicy kPolicy;

static Submission onTime() { return Submission::createNew(1, 10, "work", kDeadline - hours(2), kDeadline); }
static Submission late(int h) { return Submission::createNew(1, 10, "work", kDeadline + hours(h), kDeadline); }

TEST(SubmissionTest, OnTimeIsSubmitted) {
    Submission s = onTime();
    EXPECT_EQ(s.status(), SubmissionStatus::Submitted);
    EXPECT_EQ(s.daysLate(), 0);
}

TEST(SubmissionTest, LateIsFlaggedWithDays) {
    Submission s = late(30);   // 30 hours late = 2 started days
    EXPECT_EQ(s.status(), SubmissionStatus::Late);
    EXPECT_EQ(s.daysLate(), 2);
}

TEST(SubmissionTest, EmptyContentRejected) {
    EXPECT_THROW(Submission::createNew(1, 10, "", kDeadline, kDeadline), InvalidInputException);
}

TEST(SubmissionTest, ResubmitBeforeDeadlineReplacesWork) {
    Submission s = onTime();
    s.resubmit("better work", kDeadline - hours(1), kDeadline);
    EXPECT_EQ(s.content(), "better work");
    EXPECT_EQ(s.status(), SubmissionStatus::Submitted);
}

TEST(SubmissionTest, ResubmitAfterDeadlineRefused) {
    Submission s = onTime();
    EXPECT_THROW(s.resubmit("new", kDeadline + hours(1), kDeadline), InvalidInputException);
    Submission l = late(5);
    EXPECT_THROW(l.resubmit("new", kDeadline + hours(6), kDeadline), InvalidInputException);
}

TEST(SubmissionTest, MarkingAppliesLatePenalty) {
    Submission s = late(30);   // 2 days late, max 50: 40 - 10 = 30
    s.mark(40, 50, "Good", kPolicy);
    EXPECT_EQ(s.status(), SubmissionStatus::Marked);
    EXPECT_DOUBLE_EQ(*s.rawMark(), 40);
    EXPECT_DOUBLE_EQ(*s.finalMark(), 30);
    EXPECT_EQ(s.feedback(), "Good");
}

TEST(SubmissionTest, MarkAboveMaximumRejectedAndNothingChanges) {
    Submission s = onTime();
    EXPECT_THROW(s.mark(51, 50, "x", kPolicy), InvalidInputException);
    EXPECT_EQ(s.status(), SubmissionStatus::Submitted);
    EXPECT_FALSE(s.finalMark().has_value());
}

TEST(SubmissionTest, FullLifeCycleToReleased) {
    Submission s = onTime();
    EXPECT_FALSE(s.isVisibleToStudent());
    s.mark(45, 50, "Well done", kPolicy);
    EXPECT_FALSE(s.isVisibleToStudent());          // marked, not yet moderated
    s.approve();
    EXPECT_EQ(s.status(), SubmissionStatus::Moderated);
    EXPECT_FALSE(s.isVisibleToStudent());          // moderated, not yet released
    s.release(kDeadline + hours(100));
    EXPECT_EQ(s.status(), SubmissionStatus::Released);
    EXPECT_TRUE(s.isVisibleToStudent());           // only now can the student see it
}

TEST(SubmissionTest, HodAdjustmentChangesFinalMark) {
    Submission s = onTime();
    s.mark(45, 50, "ok", kPolicy);
    s.adjustFinalMark(40, 50);
    EXPECT_DOUBLE_EQ(*s.finalMark(), 40);
    EXPECT_DOUBLE_EQ(*s.rawMark(), 45);            // lecturer's mark is kept
    EXPECT_EQ(s.status(), SubmissionStatus::Moderated);
}

TEST(SubmissionTest, StepsOutOfOrderAreRefused) {
    Submission s = onTime();
    EXPECT_THROW(s.approve(), InvalidInputException);          // not marked yet
    EXPECT_THROW(s.release(kDeadline), InvalidInputException); // not moderated yet
    s.mark(45, 50, "ok", kPolicy);
    s.approve();
    EXPECT_THROW(s.mark(30, 50, "again", kPolicy), InvalidInputException);   // lecturer locked out
    EXPECT_THROW(s.adjustFinalMark(30, 50), InvalidInputException);
}

TEST(SubmissionTest, StatusTextMatchesDatabase) {
    EXPECT_EQ(toString(SubmissionStatus::Moderated), "MODERATED");
    EXPECT_EQ(statusFromString("LATE"), SubmissionStatus::Late);
    EXPECT_THROW(statusFromString("DONE"), InvalidInputException);
}
