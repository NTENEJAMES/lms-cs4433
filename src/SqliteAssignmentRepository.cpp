#include "SqliteAssignmentRepository.h"
#include "Exceptions.h"
#include "Statement.h"
#include "TimeUtil.h"

int SqliteAssignmentRepository::save(const Assignment& a) {
    if (a.assignmentId() == 0) {
        Statement st(db_, "INSERT INTO assignments(course_id, title, deadline, max_mark) VALUES(?,?,?,?)");
        st.bindInt(1, a.courseId()).bindText(2, a.title()).bindText(3, formatIso(a.deadline())).bindInt(4, a.maxMark());
        st.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement st(db_, "UPDATE assignments SET title = ?, deadline = ?, max_mark = ? WHERE assignment_id = ?");
    st.bindText(1, a.title()).bindText(2, formatIso(a.deadline())).bindInt(3, a.maxMark()).bindInt(4, a.assignmentId());
    st.step();
    if (db_.changes() == 0) throw InvalidInputException("No such assignment to update");
    return a.assignmentId();
}

std::optional<Assignment> SqliteAssignmentRepository::findById(int id) const {
    Statement st(db_, "SELECT assignment_id, course_id, title, deadline, max_mark FROM assignments WHERE assignment_id = ?");
    st.bindInt(1, id);
    if (!st.step()) return std::nullopt;
    Assignment a(st.columnInt(0), st.columnInt(1), st.columnText(2), parseIso(st.columnText(3)), st.columnInt(4));

    Statement ext(db_, "SELECT student_id, new_deadline FROM extensions WHERE assignment_id = ?");
    ext.bindInt(1, id);
    while (ext.step()) a.addExtension(ext.columnInt(0), parseIso(ext.columnText(1)));
    return a;
}

void SqliteAssignmentRepository::saveExtension(int assignmentId, int studentId, TimePoint newDeadline,
                                               const std::string& reason, int grantedBy) {
    std::optional<std::string> why;
    if (!reason.empty()) why = reason;
    Statement st(db_, "INSERT INTO extensions(assignment_id, student_id, new_deadline, reason, granted_by) "
                      "VALUES(?,?,?,?,?) "
                      "ON CONFLICT(assignment_id, student_id) DO UPDATE SET "
                      "new_deadline = excluded.new_deadline, reason = excluded.reason, granted_by = excluded.granted_by");
    st.bindInt(1, assignmentId).bindInt(2, studentId).bindText(3, formatIso(newDeadline))
      .bindOptionalText(4, why).bindInt(5, grantedBy);
    st.step();
}
