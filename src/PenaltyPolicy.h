#pragma once

// Strategy interface: how a late submission's mark is reduced.
class PenaltyPolicy {
public:
    virtual ~PenaltyPolicy() = default;

    // Returns the final mark after the late penalty.
    // Throws InvalidInputException for impossible values.
    virtual double apply(double mark, double maxMark, int daysLate) const = 0;
};
