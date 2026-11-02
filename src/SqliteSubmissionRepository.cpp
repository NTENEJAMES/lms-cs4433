#include "SqliteSubmissionRepository.h"
#include "Exceptions.h"
#include "TimeUtil.h"

namespace {
const char* kSelect =
    "SELECT submission_id, assignment_id, student_id, content, submitted_at, status, days_late, "
    "raw_mark, final_mark, feedback, released_at FROM submissions ";
}

Submission SqliteSubmissionRepository::fromRow(const Statement& st) {
    std::optional<double> raw, fin;
    std::optional<TimePoint> released;
    if (!st.isNull(7)) raw = st.columnDouble(7);
    if (!st.isNull(8)) fin = st.columnDouble(8);
    if (!st.isNull(10)) released = parseIso(st.columnText(10));
    return Submission(st.columnInt(0), st.columnInt(1), st.columnInt(2), st.columnText(3),
                      parseIso(st.columnText(4)), statusFromString(st.columnText(5)),
                      st.columnInt(6), raw, fin, st.isNull(9) ? "" : st.columnText(9), released);
}

int SqliteSubmissionRepository::save(const Submission& s) {
    std::optional<std::string> feedback;
    if (!s.feedback().empty()) feedback = s.feedback();
    std::optional<std::string> released;
    if (s.releasedAt()) released = formatIso(*s.releasedAt());

    if (s.submissionId() == 0) {
        Statement st(db_, "INSERT INTO submissions(assignment_id, student_id, content, submitted_at, status, "
                          "days_late, raw_mark, final_mark, feedback, released_at) VALUES(?,?,?,?,?,?,?,?,?,?)");
        st.bindInt(1, s.assignmentId()).bindInt(2, s.studentId()).bindText(3, s.content())
          .bindText(4, formatIso(s.submittedAt())).bindText(5, toString(s.status()))
          .bindInt(6, s.daysLate()).bindOptionalDouble(7, s.rawMark()).bindOptionalDouble(8, s.finalMark())
          .bindOptionalText(9, feedback).bindOptionalText(10, released);
        st.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement st(db_, "UPDATE submissions SET content = ?, submitted_at = ?, status = ?, days_late = ?, "
                      "raw_mark = ?, final_mark = ?, feedback = ?, released_at = ? WHERE submission_id = ?");
    st.bindText(1, s.content()).bindText(2, formatIso(s.submittedAt())).bindText(3, toString(s.status()))
      .bindInt(4, s.daysLate()).bindOptionalDouble(5, s.rawMark()).bindOptionalDouble(6, s.finalMark())
      .bindOptionalText(7, feedback).bindOptionalText(8, released).bindInt(9, s.submissionId());
    st.step();
    if (db_.changes() == 0) throw InvalidInputException("No such submission to update");
    return s.submissionId();
}

std::optional<Submission> SqliteSubmissionRepository::findById(int id) const {
    Statement st(db_, std::string(kSelect) + "WHERE submission_id = ?");
    st.bindInt(1, id);
    if (!st.step()) return std::nullopt;
    return fromRow(st);
}

std::vector<Submission> SqliteSubmissionRepository::findByAssignment(int assignmentId) const {
    Statement st(db_, std::string(kSelect) + "WHERE assignment_id = ? ORDER BY submission_id");
    st.bindInt(1, assignmentId);
    std::vector<Submission> result;
    while (st.step()) result.push_back(fromRow(st));
    return result;
}

std::optional<Submission> SqliteSubmissionRepository::findByAssignment(int assignmentId, int studentId) const {
    Statement st(db_, std::string(kSelect) + "WHERE assignment_id = ? AND student_id = ?");
    st.bindInt(1, assignmentId).bindInt(2, studentId);
    if (!st.step()) return std::nullopt;
    return fromRow(st);
}
