#pragma once
#include <optional>
#include <vector>
#include "Submission.h"

class SubmissionRepository {
public:
    virtual ~SubmissionRepository() = default;
    // Inserts when submissionId is 0, otherwise updates. Returns the id.
    virtual int save(const Submission& submission) = 0;
    virtual std::optional<Submission> findById(int submissionId) const = 0;
    virtual std::vector<Submission> findByAssignment(int assignmentId) const = 0;
    virtual std::optional<Submission> findByAssignment(int assignmentId, int studentId) const = 0;
};
