#pragma once
#include <optional>
#include <string>
#include "Exceptions.h"
#include "LateDays.h"

// One line of the audit log: who changed a mark, when, from what, to what, and why (FR-14)
class AuditEntry {
public:
    AuditEntry(int auditId, int submissionId, int changedBy, TimePoint changedAt,
               std::optional<double> oldValue, double newValue, std::string reason)
        : auditId_(auditId), submissionId_(submissionId), changedBy_(changedBy),
          changedAt_(changedAt), oldValue_(oldValue), newValue_(newValue), reason_(std::move(reason)) {
        if (reason_.empty()) throw InvalidInputException("A reason is required for every mark change");
        if (newValue_ < 0)   throw InvalidInputException("A mark cannot be negative");
    }
    int auditId() const { return auditId_; }
    int submissionId() const { return submissionId_; }
    int changedBy() const { return changedBy_; }
    TimePoint changedAt() const { return changedAt_; }
    std::optional<double> oldValue() const { return oldValue_; }   // empty for the first mark
    double newValue() const { return newValue_; }
    const std::string& reason() const { return reason_; }
private:
    int auditId_;
    int submissionId_;
    int changedBy_;
    TimePoint changedAt_;
    std::optional<double> oldValue_;
    double newValue_;
    std::string reason_;
};
