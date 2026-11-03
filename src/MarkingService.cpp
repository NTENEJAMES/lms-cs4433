#include "MarkingService.h"
#include "Exceptions.h"

MarkingService::MarkingService(SubmissionRepository& submissions, AssignmentRepository& assignments,
                               CourseRepository& courses, AuditRepository& audit,
                               TransactionManager& transactions, const PenaltyPolicy& policy)
    : submissions_(submissions), assignments_(assignments), courses_(courses),
      audit_(audit), transactions_(transactions), policy_(policy) {}

// Loads the submission and checks that this lecturer teaches the course (BR2)
Submission MarkingService::load(int lecturerId, int submissionId, int& maxMark) const {
    auto submission = submissions_.findById(submissionId);
    if (!submission) throw InvalidInputException("Submission not found");
    auto assignment = assignments_.findById(submission->assignmentId());
    if (!assignment) throw InvalidInputException("Assignment not found");
    auto course = courses_.findById(assignment->courseId());
    if (!course || !course->lecturerId() || *course->lecturerId() != lecturerId)
        throw AccessDeniedException("Only the lecturer of this course can mark this submission");
    maxMark = assignment->maxMark();
    return *submission;
}

// The submission and its audit entry are saved together, or not at all
Submission MarkingService::saveWithAudit(Submission submission, std::optional<double> oldFinal,
                                         int lecturerId, TimePoint now, const std::string& reason) {
    const AuditEntry entry(0, submission.submissionId(), lecturerId, now, oldFinal,
                           *submission.finalMark(), reason);
    TransactionGuard tx(transactions_);
    submissions_.save(submission);
    audit_.add(entry);
    tx.commit();
    return *submissions_.findById(submission.submissionId());
}

Submission MarkingService::mark(int lecturerId, int submissionId, double mark,
                                const std::string& feedback, TimePoint now) {
    int maxMark = 0;
    Submission s = load(lecturerId, submissionId, maxMark);
    if (s.status() != SubmissionStatus::Submitted && s.status() != SubmissionStatus::Late)
        throw InvalidInputException("This submission is already marked; use change mark with a reason");
    const std::optional<double> oldFinal = s.finalMark();
    s.mark(mark, maxMark, feedback, policy_);          // checks the mark and applies the penalty
    return saveWithAudit(s, oldFinal, lecturerId, now, "Initial marking");
}

Submission MarkingService::changeMark(int lecturerId, int submissionId, double newMark,
                                      const std::string& reason, TimePoint now) {
    if (reason.empty()) throw InvalidInputException("A reason is required to change a mark");
    int maxMark = 0;
    Submission s = load(lecturerId, submissionId, maxMark);
    if (s.status() != SubmissionStatus::Marked)
        throw InvalidInputException("Only a marked submission that is not yet moderated can be changed");
    const std::optional<double> oldFinal = s.finalMark();
    s.mark(newMark, maxMark, s.feedback(), policy_);   // keeps the existing feedback
    return saveWithAudit(s, oldFinal, lecturerId, now, reason);
}
