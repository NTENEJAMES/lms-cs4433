#include "TenPercentPerDayPolicy.h"
#include "Exceptions.h"

double TenPercentPerDayPolicy::apply(double mark, double maxMark, int daysLate) const {
    if (maxMark <= 0)               throw InvalidInputException("Maximum mark must be above 0");
    if (mark < 0 || mark > maxMark) throw InvalidInputException("Mark must be between 0 and the maximum");
    if (daysLate < 0)               throw InvalidInputException("Days late cannot be negative");

    if (daysLate == 0) return mark;
    if (daysLate > 3)  return 0.0;

    double reduced = mark - 0.10 * maxMark * daysLate;
    return reduced < 0 ? 0.0 : reduced;
}
