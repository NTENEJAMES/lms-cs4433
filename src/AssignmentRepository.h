#pragma once
#include <optional>
#include <string>
#include "Assignment.h"

class AssignmentRepository {
public:
    virtual ~AssignmentRepository() = default;
    // Inserts when assignmentId is 0, otherwise updates. Returns the id.
    virtual int save(const Assignment& assignment) = 0;
    // Loads the assignment together with its individual extensions
    virtual std::optional<Assignment> findById(int assignmentId) const = 0;
    // Adds or replaces the extension of one student for one assignment
    virtual void saveExtension(int assignmentId, int studentId, TimePoint newDeadline,
                               const std::string& reason, int grantedBy) = 0;
};
