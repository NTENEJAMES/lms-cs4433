#include "LateDays.h"

int lateDays(TimePoint deadline, TimePoint submitted) {
    if (submitted <= deadline) return 0;
    const auto late = std::chrono::duration_cast<std::chrono::seconds>(submitted - deadline).count();
    const long long day = 24 * 60 * 60;
    return static_cast<int>((late + day - 1) / day);   // round up
}
