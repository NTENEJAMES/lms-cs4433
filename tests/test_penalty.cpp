#include <gtest/gtest.h>
#include "Exceptions.h"
#include "LateDays.h"
#include "TenPercentPerDayPolicy.h"

using std::chrono::hours;
using std::chrono::seconds;

// ---- Late penalty (FR-09), maximum mark = 50, so 10% = 5 marks per day ----
TEST(PenaltyTest, OnTimeKeepsMark)            { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(40, 50, 0), 40); }
TEST(PenaltyTest, OneDayLateLosesTenPercent)  { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(40, 50, 1), 35); }
TEST(PenaltyTest, TwoDaysLate)                { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(40, 50, 2), 30); }
TEST(PenaltyTest, ThreeDaysLateIsLastDayWithMark) { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(40, 50, 3), 25); }
TEST(PenaltyTest, FourDaysLateIsZero)         { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(40, 50, 4), 0); }
TEST(PenaltyTest, MarkNeverGoesBelowZero)     { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(10, 50, 3), 0); }
TEST(PenaltyTest, FullMarkOnTime)             { EXPECT_DOUBLE_EQ(TenPercentPerDayPolicy().apply(50, 50, 0), 50); }

TEST(PenaltyTest, InvalidInputThrows) {
    TenPercentPerDayPolicy p;
    EXPECT_THROW(p.apply(51, 50, 0), InvalidInputException);   // above maximum
    EXPECT_THROW(p.apply(-1, 50, 0), InvalidInputException);   // below zero
    EXPECT_THROW(p.apply(10, 0, 0),  InvalidInputException);   // maximum must be > 0
    EXPECT_THROW(p.apply(10, 50, -1), InvalidInputException);  // negative days
}

// ---- Counting late days: every started 24 hours is one day ----
TEST(LateDaysTest, BoundaryValues) {
    TimePoint d = std::chrono::system_clock::now();
    EXPECT_EQ(lateDays(d, d - hours(5)), 0);                    // early
    EXPECT_EQ(lateDays(d, d), 0);                               // exactly at deadline
    EXPECT_EQ(lateDays(d, d + seconds(1)), 1);                  // 1 second late = 1 day
    EXPECT_EQ(lateDays(d, d + hours(24)), 1);                   // exactly 24 h = 1 day
    EXPECT_EQ(lateDays(d, d + hours(24) + seconds(1)), 2);      // 24 h and 1 s = 2 days
    EXPECT_EQ(lateDays(d, d + hours(72)), 3);
    EXPECT_EQ(lateDays(d, d + hours(72) + seconds(1)), 4);
}
