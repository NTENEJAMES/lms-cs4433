#include "Database.h"
#include <fstream>
#include <sstream>

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

void Database::executeFile(const std::string& path) {
    std::ifstream in(path);
    if (!in) throw DatabaseException("Cannot open SQL file: " + path);
    std::stringstream buffer;
    buffer << in.rdbuf();
    execute(buffer.str());
}

int Database::countRows(const std::string& sql) {
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        throw DatabaseException(std::string("SQL error: ") + sqlite3_errmsg(db_));
    int result = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) result = sqlite3_column_int(stmt, 0);
    sqlite3_finalize(stmt);
    return result;
}
