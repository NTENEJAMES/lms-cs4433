#include "SqliteAuditRepository.h"
#include "Statement.h"
#include "TimeUtil.h"

int SqliteAuditRepository::add(const AuditEntry& e) {
    Statement st(db_, "INSERT INTO audit_log(submission_id, changed_by, changed_at, old_value, new_value, reason) "
                      "VALUES(?,?,?,?,?,?)");
    st.bindInt(1, e.submissionId()).bindInt(2, e.changedBy()).bindText(3, formatIso(e.changedAt()))
      .bindOptionalDouble(4, e.oldValue()).bindDouble(5, e.newValue()).bindText(6, e.reason());
    st.step();
    return static_cast<int>(db_.lastInsertId());
}

std::vector<AuditEntry> SqliteAuditRepository::findBySubmission(int submissionId) const {
    Statement st(db_, "SELECT audit_id, submission_id, changed_by, changed_at, old_value, new_value, reason "
                      "FROM audit_log WHERE submission_id = ? ORDER BY audit_id");
    st.bindInt(1, submissionId);
    std::vector<AuditEntry> result;
    while (st.step()) {
        std::optional<double> old;
        if (!st.isNull(4)) old = st.columnDouble(4);
        result.emplace_back(st.columnInt(0), st.columnInt(1), st.columnInt(2), parseIso(st.columnText(3)),
                            old, st.columnDouble(5), st.columnText(6));
    }
    return result;
}
