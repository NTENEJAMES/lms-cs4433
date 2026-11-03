#pragma once
#include "AssignmentRepository.h"
#include "Database.h"

class SqliteAssignmentRepository : public AssignmentRepository {
public:
    explicit SqliteAssignmentRepository(Database& db) : db_(db) {}
    int save(const Assignment& assignment) override;
    std::optional<Assignment> findById(int assignmentId) const override;
    void saveExtension(int assignmentId, int studentId, TimePoint newDeadline,
                       const std::string& reason, int grantedBy) override;
private:
    Database& db_;
};
