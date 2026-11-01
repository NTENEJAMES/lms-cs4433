#pragma once
#include <optional>
#include <string>
#include "Exceptions.h"

class Course {
public:
    Course(int courseId, std::string code, std::string title, int departmentId,
           std::optional<int> lecturerId = std::nullopt)
        : courseId_(courseId), code_(std::move(code)), title_(std::move(title)),
          departmentId_(departmentId), lecturerId_(lecturerId) {
        if (code_.empty() || title_.empty())
            throw InvalidInputException("Course code and title cannot be empty");
    }
    int courseId() const { return courseId_; }
    const std::string& code() const { return code_; }
    const std::string& title() const { return title_; }
    int departmentId() const { return departmentId_; }
    std::optional<int> lecturerId() const { return lecturerId_; }
    void assignLecturer(int lecturerId) { lecturerId_ = lecturerId; }
private:
    int courseId_;
    std::string code_;
    std::string title_;
    int departmentId_;
    std::optional<int> lecturerId_;
};
