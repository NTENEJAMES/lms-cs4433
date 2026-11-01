#pragma once
#include <optional>
#include <string>
#include "LateDays.h"
#include "PenaltyPolicy.h"

// Life cycle: Submitted or Late -> Marked -> Moderated -> Released
enum class SubmissionStatus { Submitted, Late, Marked, Moderated, Released };

std::string toString(SubmissionStatus s);                  // "SUBMITTED", "LATE", ...
SubmissionStatus statusFromString(const std::string& s);   // throws InvalidInputException

class Submission {
public:
    // Full constructor, used when loading from the database
    Submission(int submissionId, int assignmentId, int studentId, std::string content,
               TimePoint submittedAt, SubmissionStatus status, int daysLate,
               std::optional<double> rawMark, std::optional<double> finalMark,
               std::string feedback, std::optional<TimePoint> releasedAt);

    // New submission: status is Submitted, or Late if after the deadline
    static Submission createNew(int assignmentId, int studentId, std::string content,
                                TimePoint submittedAt, TimePoint deadline);

    // Life-cycle steps; each throws InvalidInputException if not allowed
    void resubmit(std::string content, TimePoint now, TimePoint deadline);
    void mark(double rawMark, double maxMark, std::string feedback, const PenaltyPolicy& policy);
    void approve();                                         // HoD approves: Marked -> Moderated
    void adjustFinalMark(double newMark, double maxMark);   // HoD adjusts: Marked -> Moderated
    void release(TimePoint when);                           // Moderated -> Released

    bool isVisibleToStudent() const { return status_ == SubmissionStatus::Released; }

    int submissionId() const { return submissionId_; }
    int assignmentId() const { return assignmentId_; }
    int studentId() const { return studentId_; }
    const std::string& content() const { return content_; }
    TimePoint submittedAt() const { return submittedAt_; }
    SubmissionStatus status() const { return status_; }
    int daysLate() const { return daysLate_; }
    std::optional<double> rawMark() const { return rawMark_; }
    std::optional<double> finalMark() const { return finalMark_; }
    const std::string& feedback() const { return feedback_; }
    std::optional<TimePoint> releasedAt() const { return releasedAt_; }

private:
    int submissionId_;
    int assignmentId_;
    int studentId_;
    std::string content_;
    TimePoint submittedAt_;
    SubmissionStatus status_;
    int daysLate_;
    std::optional<double> rawMark_;
    std::optional<double> finalMark_;
    std::string feedback_;
    std::optional<TimePoint> releasedAt_;
};
