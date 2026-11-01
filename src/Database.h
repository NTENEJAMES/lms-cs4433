#pragma once
#include <sqlite3.h>
#include <stdexcept>
#include <string>

class DatabaseException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Owns one SQLite connection and closes it automatically.
class Database {
public:
    explicit Database(const std::string& path);   // use ":memory:" in tests
    ~Database();
    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;

    void execute(const std::string& sql);         // for schema scripts
    void executeFile(const std::string& path);    // run an .sql file (e.g. db/schema.sql)
    int  countRows(const std::string& sql);       // runs a SELECT COUNT(*) query (for tests and checks)
    sqlite3* handle() const { return db_; }

private:
    sqlite3* db_ = nullptr;
};
