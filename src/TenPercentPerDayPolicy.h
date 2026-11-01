#pragma once
#include "PenaltyPolicy.h"

// HoD rule: lose 10% of the MAXIMUM mark for each late day,
// for up to 3 days. From the 4th day the mark is zero.
// The final mark never goes below zero.
class TenPercentPerDayPolicy : public PenaltyPolicy {
public:
    double apply(double mark, double maxMark, int daysLate) const override;
};
