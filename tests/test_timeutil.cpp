#include <gtest/gtest.h>
#include "Exceptions.h"
#include "TimeUtil.h"

TEST(TimeUtilTest, KnownDateRoundTrips) {
    const std::string text = "2026-11-20T23:59:00";
    EXPECT_EQ(formatIso(parseIso(text)), text);
}

TEST(TimeUtilTest, EpochIsZero) {
    EXPECT_EQ(parseIso("1970-01-01T00:00:00"), TimePoint(std::chrono::seconds(0)));
    EXPECT_EQ(parseIso("1970-01-02T00:00:00"), TimePoint(std::chrono::seconds(86400)));
}

TEST(TimeUtilTest, LeapDayIsAcceptedOnlyInLeapYears) {
    EXPECT_NO_THROW(parseIso("2028-02-29T10:00:00"));
    EXPECT_THROW(parseIso("2027-02-29T10:00:00"), InvalidInputException);
}

TEST(TimeUtilTest, InvalidTextThrows) {
    EXPECT_THROW(parseIso("garbage"), InvalidInputException);
    EXPECT_THROW(parseIso("2026-13-01T10:00:00"), InvalidInputException);
    EXPECT_THROW(parseIso("2026-02-30T10:00:00"), InvalidInputException);
    EXPECT_THROW(parseIso("2026-11-20T24:00:00"), InvalidInputException);
    EXPECT_THROW(parseIso("2026-11-20 10:00:00"), InvalidInputException);
}

TEST(TimeUtilTest, OneDayLaterDiffersBy86400Seconds) {
    EXPECT_EQ(parseIso("2026-12-01T00:00:00") - parseIso("2026-11-30T00:00:00"), std::chrono::hours(24));
}
