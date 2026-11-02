#include "TimeUtil.h"
#include <cstdio>
#include "Exceptions.h"

namespace {
// Days since 1970-01-01 for a calendar date (works the same on every platform)
long long daysFromCivil(int y, unsigned m, unsigned d) {
    y -= m <= 2;
    const long long era = (y >= 0 ? y : y - 399) / 400;
    const unsigned yoe = static_cast<unsigned>(y - era * 400);
    const unsigned doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
    const unsigned doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
    return era * 146097 + static_cast<long long>(doe) - 719468;
}

void civilFromDays(long long z, int& y, unsigned& m, unsigned& d) {
    z += 719468;
    const long long era = (z >= 0 ? z : z - 146096) / 146097;
    const unsigned doe = static_cast<unsigned>(z - era * 146097);
    const unsigned yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    y = static_cast<int>(yoe) + static_cast<int>(era) * 400;
    const unsigned doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    const unsigned mp = (5 * doy + 2) / 153;
    d = doy - (153 * mp + 2) / 5 + 1;
    m = mp < 10 ? mp + 3 : mp - 9;
    y += (m <= 2);
}
}  // namespace

std::string formatIso(TimePoint t) {
    long long secs = std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch()).count();
    long long days = secs / 86400;
    long long rem = secs % 86400;
    if (rem < 0) { rem += 86400; --days; }
    int y; unsigned m, d;
    civilFromDays(days, y, m, d);
    char buf[32];
    std::snprintf(buf, sizeof buf, "%04d-%02u-%02uT%02d:%02d:%02d", y, m, d,
                  static_cast<int>(rem / 3600), static_cast<int>(rem % 3600 / 60),
                  static_cast<int>(rem % 60));
    return buf;
}

TimePoint parseIso(const std::string& text) {
    int y, mo, d, h, mi, s;
    char tail = 0;
    if (text.size() != 19 ||
        std::sscanf(text.c_str(), "%4d-%2d-%2dT%2d:%2d:%2d%c", &y, &mo, &d, &h, &mi, &s, &tail) != 6)
        throw InvalidInputException("Invalid date/time: " + text);
    if (mo < 1 || mo > 12 || d < 1 || d > 31 || h > 23 || mi > 59 || s > 59 || h < 0 || mi < 0 || s < 0)
        throw InvalidInputException("Invalid date/time: " + text);
    const long long days = daysFromCivil(y, static_cast<unsigned>(mo), static_cast<unsigned>(d));
    int y2; unsigned m2, d2;
    civilFromDays(days, y2, m2, d2);                  // catches dates such as 30 February
    if (y2 != y || static_cast<int>(m2) != mo || static_cast<int>(d2) != d)
        throw InvalidInputException("Invalid date/time: " + text);
    return TimePoint(std::chrono::seconds(days * 86400LL + h * 3600 + mi * 60 + s));
}
