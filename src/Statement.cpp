#include "Statement.h"

Statement::Statement(Database& db, const std::string& sql) : db_(db.handle()) {
    check(sqlite3_prepare_v2(db_, sql.c_str(), -1, &stmt_, nullptr));
}

Statement::~Statement() { sqlite3_finalize(stmt_); }

void Statement::check(int code) {
    if (code != SQLITE_OK && code != SQLITE_ROW && code != SQLITE_DONE)
        throw DatabaseException(std::string("SQL error: ") + sqlite3_errmsg(db_));
}

Statement& Statement::bindInt(int i, int v)    { check(sqlite3_bind_int(stmt_, i, v)); return *this; }
Statement& Statement::bindDouble(int i, double v) { check(sqlite3_bind_double(stmt_, i, v)); return *this; }
Statement& Statement::bindNull(int i)          { check(sqlite3_bind_null(stmt_, i)); return *this; }
Statement& Statement::bindText(int i, const std::string& v) {
    check(sqlite3_bind_text(stmt_, i, v.c_str(), static_cast<int>(v.size()), SQLITE_TRANSIENT));
    return *this;
}
Statement& Statement::bindOptionalInt(int i, std::optional<int> v) {
    return v ? bindInt(i, *v) : bindNull(i);
}
Statement& Statement::bindOptionalDouble(int i, std::optional<double> v) {
    return v ? bindDouble(i, *v) : bindNull(i);
}
Statement& Statement::bindOptionalText(int i, const std::optional<std::string>& v) {
    return v ? bindText(i, *v) : bindNull(i);
}

bool Statement::step() {
    const int rc = sqlite3_step(stmt_);
    if (rc == SQLITE_ROW) return true;
    if (rc == SQLITE_DONE) return false;
    throw DatabaseException(std::string("SQL error: ") + sqlite3_errmsg(db_));
}

int Statement::columnInt(int i) const { return sqlite3_column_int(stmt_, i); }
double Statement::columnDouble(int i) const { return sqlite3_column_double(stmt_, i); }
bool Statement::isNull(int i) const { return sqlite3_column_type(stmt_, i) == SQLITE_NULL; }
std::string Statement::columnText(int i) const {
    const unsigned char* t = sqlite3_column_text(stmt_, i);
    return t ? reinterpret_cast<const char*>(t) : "";
}
