#include "SqliteCourseRepository.h"
#include "Exceptions.h"
#include "Statement.h"

int SqliteCourseRepository::save(const Course& c) {
    if (c.courseId() == 0) {
        Statement st(db_, "INSERT INTO courses(code, title, department_id, lecturer_id) VALUES(?,?,?,?)");
        st.bindText(1, c.code()).bindText(2, c.title()).bindInt(3, c.departmentId()).bindOptionalInt(4, c.lecturerId());
        st.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement st(db_, "UPDATE courses SET code = ?, title = ?, department_id = ?, lecturer_id = ? WHERE course_id = ?");
    st.bindText(1, c.code()).bindText(2, c.title()).bindInt(3, c.departmentId())
      .bindOptionalInt(4, c.lecturerId()).bindInt(5, c.courseId());
    st.step();
    if (db_.changes() == 0) throw InvalidInputException("No such course to update");
    return c.courseId();
}

std::optional<Course> SqliteCourseRepository::findById(int id) const {
    Statement st(db_, "SELECT course_id, code, title, department_id, lecturer_id FROM courses WHERE course_id = ?");
    st.bindInt(1, id);
    if (!st.step()) return std::nullopt;
    std::optional<int> lecturer;
    if (!st.isNull(4)) lecturer = st.columnInt(4);
    return Course(st.columnInt(0), st.columnText(1), st.columnText(2), st.columnInt(3), lecturer);
}

void SqliteCourseRepository::enrol(int studentId, int courseId) {
    Statement st(db_, "INSERT INTO enrolments(student_id, course_id) VALUES(?, ?)");
    st.bindInt(1, studentId).bindInt(2, courseId);
    st.step();
}

bool SqliteCourseRepository::isEnrolled(int studentId, int courseId) const {
    Statement st(db_, "SELECT 1 FROM enrolments WHERE student_id = ? AND course_id = ?");
    st.bindInt(1, studentId).bindInt(2, courseId);
    return st.step();
}
