#pragma once
#include <chrono>

using TimePoint = std::chrono::system_clock::time_point;

// Number of late days: every STARTED 24-hour period after the deadline
// counts as one day. On time (or exactly at the deadline) = 0.
int lateDays(TimePoint deadline, TimePoint submitted);
