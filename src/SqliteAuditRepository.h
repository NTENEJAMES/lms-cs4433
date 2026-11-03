#pragma once
#include "AuditRepository.h"
#include "Database.h"

class SqliteAuditRepository : public AuditRepository {
public:
    explicit SqliteAuditRepository(Database& db) : db_(db) {}
    int add(const AuditEntry& entry) override;
    std::vector<AuditEntry> findBySubmission(int submissionId) const override;
private:
    Database& db_;
};
