#pragma once
#include <string>
#include "LateDays.h"

// Dates are stored in the database as ISO 8601 text in UTC, e.g. 2026-11-20T23:59:00
std::string formatIso(TimePoint t);
TimePoint parseIso(const std::string& text);   // throws InvalidInputException if invalid
