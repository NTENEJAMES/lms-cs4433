#pragma once
#include "Database.h"
#include "Statement.h"
#include "SubmissionRepository.h"

class SqliteSubmissionRepository : public SubmissionRepository {
public:
    explicit SqliteSubmissionRepository(Database& db) : db_(db) {}
    int save(const Submission& submission) override;
    std::optional<Submission> findById(int submissionId) const override;
    std::vector<Submission> findByAssignment(int assignmentId) const override;
    std::optional<Submission> findByAssignment(int assignmentId, int studentId) const override;
private:
    static Submission fromRow(const Statement& st);
    Database& db_;
};
