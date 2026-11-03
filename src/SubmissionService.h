#pragma once
#include <string>
#include "AssignmentRepository.h"
#include "CourseRepository.h"
#include "SubmissionRepository.h"

// Use case UC-1: Submit assignment (FR-07, FR-08, FR-09, FR-10)
class SubmissionService {
public:
    SubmissionService(AssignmentRepository& assignments, CourseRepository& courses,
                      SubmissionRepository& submissions);

    // First submission, resubmission (before the deadline) or late submission.
    // 'now' is passed in so the rules can be tested at exact times.
    // Throws InvalidInputException, AccessDeniedException or DatabaseException;
    // when it throws, nothing has been saved.
    Submission submit(int studentId, int assignmentId, const std::string& content, TimePoint now);

private:
    AssignmentRepository& assignments_;
    CourseRepository& courses_;
    SubmissionRepository& submissions_;
};
