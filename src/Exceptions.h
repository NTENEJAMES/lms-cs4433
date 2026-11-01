#pragma once
#include <stdexcept>
#include <string>

// Thrown when a user or a caller gives invalid input.
class InvalidInputException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};
