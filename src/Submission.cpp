#include "Submission.h"
#include "Exceptions.h"

std::string toString(SubmissionStatus s) {
    switch (s) {
        case SubmissionStatus::Submitted: return "SUBMITTED";
        case SubmissionStatus::Late:      return "LATE";
        case SubmissionStatus::Marked:    return "MARKED";
        case SubmissionStatus::Moderated: return "MODERATED";
        case SubmissionStatus::Released:  return "RELEASED";
    }
    throw InvalidInputException("Unknown status");
}

SubmissionStatus statusFromString(const std::string& s) {
    if (s == "SUBMITTED") return SubmissionStatus::Submitted;
    if (s == "LATE")      return SubmissionStatus::Late;
    if (s == "MARKED")    return SubmissionStatus::Marked;
    if (s == "MODERATED") return SubmissionStatus::Moderated;
    if (s == "RELEASED")  return SubmissionStatus::Released;
    throw InvalidInputException("Unknown status: " + s);
}

Submission::Submission(int submissionId, int assignmentId, int studentId, std::string content,
                       TimePoint submittedAt, SubmissionStatus status, int daysLate,
                       std::optional<double> rawMark, std::optional<double> finalMark,
                       std::string feedback, std::optional<TimePoint> releasedAt)
    : submissionId_(submissionId), assignmentId_(assignmentId), studentId_(studentId),
      content_(std::move(content)), submittedAt_(submittedAt), status_(status),
      daysLate_(daysLate), rawMark_(rawMark), finalMark_(finalMark),
      feedback_(std::move(feedback)), releasedAt_(releasedAt) {}

Submission Submission::createNew(int assignmentId, int studentId, std::string content,
                                 TimePoint submittedAt, TimePoint deadline) {
    if (content.empty()) throw InvalidInputException("Submission cannot be empty");
    const int late = lateDays(deadline, submittedAt);
    return Submission(0, assignmentId, studentId, std::move(content), submittedAt,
                      late > 0 ? SubmissionStatus::Late : SubmissionStatus::Submitted,
                      late, std::nullopt, std::nullopt, "", std::nullopt);
}

void Submission::resubmit(std::string content, TimePoint now, TimePoint deadline) {
    if (content.empty()) throw InvalidInputException("Submission cannot be empty");
    if (status_ != SubmissionStatus::Submitted || now > deadline)
        throw InvalidInputException("Resubmission is only allowed before the deadline");
    content_ = std::move(content);
    submittedAt_ = now;
}

void Submission::mark(double rawMark, double maxMark, std::string feedback,
                      const PenaltyPolicy& policy) {
    if (status_ != SubmissionStatus::Submitted && status_ != SubmissionStatus::Late &&
        status_ != SubmissionStatus::Marked)
        throw InvalidInputException("This submission can no longer be marked by the lecturer");
    const double result = policy.apply(rawMark, maxMark, daysLate_);   // validates the mark
    rawMark_ = rawMark;
    finalMark_ = result;
    feedback_ = std::move(feedback);
    status_ = SubmissionStatus::Marked;
}

void Submission::approve() {
    if (status_ != SubmissionStatus::Marked)
        throw InvalidInputException("Only a marked submission can be approved");
    status_ = SubmissionStatus::Moderated;
}

void Submission::adjustFinalMark(double newMark, double maxMark) {
    if (status_ != SubmissionStatus::Marked)
        throw InvalidInputException("Only a marked submission can be adjusted");
    if (maxMark <= 0 || newMark < 0 || newMark > maxMark)
        throw InvalidInputException("Mark must be between 0 and the maximum");
    finalMark_ = newMark;
    status_ = SubmissionStatus::Moderated;
}

void Submission::release(TimePoint when) {
    if (status_ != SubmissionStatus::Moderated)
        throw InvalidInputException("Only a moderated submission can be released");
    releasedAt_ = when;
    status_ = SubmissionStatus::Released;
}
