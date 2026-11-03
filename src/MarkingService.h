#pragma once
#include <string>
#include "AssignmentRepository.h"
#include "AuditRepository.h"
#include "CourseRepository.h"
#include "PenaltyPolicy.h"
#include "SubmissionRepository.h"
#include "TransactionManager.h"

// Use case UC-2: Mark submission (FR-09, FR-11, FR-14)
class MarkingService {
public:
    MarkingService(SubmissionRepository& submissions, AssignmentRepository& assignments,
                   CourseRepository& courses, AuditRepository& audit,
                   TransactionManager& transactions, const PenaltyPolicy& policy);

    // First marking of a Submitted or Late submission. The late penalty is applied.
    Submission mark(int lecturerId, int submissionId, double mark,
                    const std::string& feedback, TimePoint now);

    // Changing a mark that is Marked but not yet moderated. A reason is required.
    Submission changeMark(int lecturerId, int submissionId, double newMark,
                          const std::string& reason, TimePoint now);

private:
    Submission load(int lecturerId, int submissionId, int& maxMark) const;
    Submission saveWithAudit(Submission submission, std::optional<double> oldFinal,
                             int lecturerId, TimePoint now, const std::string& reason);

    SubmissionRepository& submissions_;
    AssignmentRepository& assignments_;
    CourseRepository& courses_;
    AuditRepository& audit_;
    TransactionManager& transactions_;
    const PenaltyPolicy& policy_;
};
