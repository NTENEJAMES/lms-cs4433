#include <gtest/gtest.h>
#include "Database.h"

// LMS_SCHEMA_PATH is set in CMakeLists.txt
static void loadSchema(Database& db) {
    db.executeFile(LMS_SCHEMA_PATH);
}

// Helper data: one department, course, assignment, student and lecturer
static void addBasics(Database& db) {
    db.execute("INSERT INTO users(username,password_hash,salt,full_name,role) VALUES('stu1','h','s','Student One','STUDENT');");
    db.execute("INSERT INTO users(username,password_hash,salt,full_name,role) VALUES('lec1','h','s','Lecturer One','LECTURER');");
    db.execute("INSERT INTO departments(name) VALUES('Science');");
    db.execute("INSERT INTO courses(code,title,department_id,lecturer_id) VALUES('SC1','Biology',1,2);");
    db.execute("INSERT INTO assignments(course_id,title,deadline,max_mark) VALUES(1,'Essay','2026-11-20T23:59:00',50);");
}

TEST(SchemaTest, CreatesAllNineTables) {
    Database db(":memory:");
    loadSchema(db);
    EXPECT_EQ(db.countRows(
        "SELECT COUNT(*) FROM sqlite_master WHERE type='table' AND name IN "
        "('users','departments','courses','enrolments','assignments',"
        "'extensions','submissions','audit_log','notifications')"), 9);
}

TEST(SchemaTest, RejectsUnknownRole) {
    Database db(":memory:");
    loadSchema(db);
    EXPECT_THROW(db.execute("INSERT INTO users(username,password_hash,salt,full_name,role) "
                            "VALUES('x','h','s','X','BOSS');"), DatabaseException);
}

TEST(SchemaTest, ForeignKeysAreEnforced) {
    Database db(":memory:");
    loadSchema(db);
    EXPECT_THROW(db.execute("INSERT INTO courses(code,title,department_id) VALUES('C','T',99);"),
                 DatabaseException);
}

TEST(SchemaTest, RejectsNegativeMaxMark) {
    Database db(":memory:");
    loadSchema(db);
    addBasics(db);
    EXPECT_THROW(db.execute("INSERT INTO assignments(course_id,title,deadline,max_mark) "
                            "VALUES(1,'Bad','2026-11-20T23:59:00',0);"), DatabaseException);
}

TEST(SchemaTest, OneSubmissionPerStudentPerAssignment) {
    Database db(":memory:");
    loadSchema(db);
    addBasics(db);
    db.execute("INSERT INTO submissions(assignment_id,student_id,content,submitted_at) "
               "VALUES(1,1,'work','2026-11-19T10:00:00');");
    EXPECT_THROW(db.execute("INSERT INTO submissions(assignment_id,student_id,content,submitted_at) "
                            "VALUES(1,1,'again','2026-11-19T11:00:00');"), DatabaseException);
}

TEST(SchemaTest, RejectsUnknownSubmissionStatus) {
    Database db(":memory:");
    loadSchema(db);
    addBasics(db);
    EXPECT_THROW(db.execute("INSERT INTO submissions(assignment_id,student_id,content,submitted_at,status) "
                            "VALUES(1,1,'w','2026-11-19T10:00:00','DONE');"), DatabaseException);
}

TEST(SchemaTest, MissingFileThrows) {
    Database db(":memory:");
    EXPECT_THROW(db.executeFile("no_such_file.sql"), DatabaseException);
}
