#pragma once
#include <optional>
#include <string>
#include "Exceptions.h"

class Department {
public:
    Department(int departmentId, std::string name, std::optional<int> hodUserId = std::nullopt)
        : departmentId_(departmentId), name_(std::move(name)), hodUserId_(hodUserId) {
        if (name_.empty()) throw InvalidInputException("Department name cannot be empty");
    }
    int departmentId() const { return departmentId_; }
    const std::string& name() const { return name_; }
    std::optional<int> hodUserId() const { return hodUserId_; }
private:
    int departmentId_;
    std::string name_;
    std::optional<int> hodUserId_;
};
