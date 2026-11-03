#pragma once
#include <vector>
#include "AuditEntry.h"

class AuditRepository {
public:
    virtual ~AuditRepository() = default;
    virtual int add(const AuditEntry& entry) = 0;                       // entries are never edited or deleted
    virtual std::vector<AuditEntry> findBySubmission(int submissionId) const = 0;   // oldest first
};
