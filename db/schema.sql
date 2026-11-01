PRAGMA foreign_keys = ON;

CREATE TABLE users (
    user_id        INTEGER PRIMARY KEY AUTOINCREMENT,
    username       TEXT    NOT NULL UNIQUE,
    password_hash  TEXT    NOT NULL,
    salt           TEXT    NOT NULL,
    full_name      TEXT    NOT NULL,
    role           TEXT    NOT NULL
                   CHECK (role IN ('ADMIN','LECTURER','HOD','STUDENT')),
    is_active      INTEGER NOT NULL DEFAULT 1 CHECK (is_active IN (0,1))
);

CREATE TABLE departments (
    department_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    name           TEXT    NOT NULL UNIQUE,
    hod_user_id    INTEGER UNIQUE REFERENCES users(user_id)
);

CREATE TABLE courses (
    course_id      INTEGER PRIMARY KEY AUTOINCREMENT,
    code           TEXT    NOT NULL UNIQUE,
    title          TEXT    NOT NULL,
    department_id  INTEGER NOT NULL REFERENCES departments(department_id),
    lecturer_id    INTEGER REFERENCES users(user_id)
);

CREATE TABLE enrolments (
    student_id     INTEGER NOT NULL REFERENCES users(user_id),
    course_id      INTEGER NOT NULL REFERENCES courses(course_id),
    PRIMARY KEY (student_id, course_id)
);

CREATE TABLE assignments (
    assignment_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    course_id      INTEGER NOT NULL REFERENCES courses(course_id),
    title          TEXT    NOT NULL,
    deadline       TEXT    NOT NULL,          -- ISO 8601, e.g. 2026-11-20T23:59:00
    max_mark       INTEGER NOT NULL CHECK (max_mark > 0)
);

CREATE TABLE extensions (
    extension_id   INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id  INTEGER NOT NULL REFERENCES assignments(assignment_id),
    student_id     INTEGER NOT NULL REFERENCES users(user_id),
    new_deadline   TEXT    NOT NULL,
    reason         TEXT,
    granted_by     INTEGER NOT NULL REFERENCES users(user_id),
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE submissions (
    submission_id  INTEGER PRIMARY KEY AUTOINCREMENT,
    assignment_id  INTEGER NOT NULL REFERENCES assignments(assignment_id),
    student_id     INTEGER NOT NULL REFERENCES users(user_id),
    content        TEXT    NOT NULL,
    submitted_at   TEXT    NOT NULL,
    status         TEXT    NOT NULL DEFAULT 'SUBMITTED'
                   CHECK (status IN ('SUBMITTED','LATE','MARKED','MODERATED','RELEASED')),
    days_late      INTEGER NOT NULL DEFAULT 0 CHECK (days_late >= 0),
    raw_mark       REAL    CHECK (raw_mark >= 0),
    final_mark     REAL    CHECK (final_mark >= 0),
    feedback       TEXT,
    released_at    TEXT,
    UNIQUE (assignment_id, student_id)
);

CREATE TABLE audit_log (
    audit_id       INTEGER PRIMARY KEY AUTOINCREMENT,
    submission_id  INTEGER NOT NULL REFERENCES submissions(submission_id),
    changed_by     INTEGER NOT NULL REFERENCES users(user_id),
    changed_at     TEXT    NOT NULL,
    old_value      REAL,                      -- NULL for the first mark
    new_value      REAL    NOT NULL,
    reason         TEXT    NOT NULL
);

CREATE TABLE notifications (
    notification_id INTEGER PRIMARY KEY AUTOINCREMENT,
    user_id         INTEGER NOT NULL REFERENCES users(user_id),
    message         TEXT    NOT NULL,
    created_at      TEXT    NOT NULL,
    is_read         INTEGER NOT NULL DEFAULT 0 CHECK (is_read IN (0,1))
);

-- Indexes for the 800-student performance target (NFR-02)
CREATE INDEX idx_submissions_assignment ON submissions(assignment_id);
CREATE INDEX idx_submissions_student    ON submissions(student_id);
CREATE INDEX idx_enrolments_course      ON enrolments(course_id);
CREATE INDEX idx_assignments_course     ON assignments(course_id);
CREATE INDEX idx_audit_submission       ON audit_log(submission_id);
CREATE INDEX idx_notifications_user     ON notifications(user_id, is_read);
