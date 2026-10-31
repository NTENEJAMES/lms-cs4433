#include "Database.h"

Database::Database(const std::string& path) {
    if (sqlite3_open(path.c_str(), &db_) != SQLITE_OK) {
        std::string msg = sqlite3_errmsg(db_);
        sqlite3_close(db_);
        throw DatabaseException("Cannot open database: " + msg);
    }
    execute("PRAGMA foreign_keys = ON;");
}

Database::~Database() { sqlite3_close(db_); }

void Database::execute(const std::string& sql) {
    char* err = nullptr;
    if (sqlite3_exec(db_, sql.c_str(), nullptr, nullptr, &err) != SQLITE_OK) {
        std::string msg = err ? err : "unknown error";
        sqlite3_free(err);
        throw DatabaseException("SQL error: " + msg);
    }
}
