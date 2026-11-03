#pragma once
#include <optional>
#include <string>
#include "Database.h"

// One prepared SQL statement. Values are always passed with bind...()
// (parameterised queries), never by joining text into the SQL.
class Statement {
public:
    Statement(Database& db, const std::string& sql);
    ~Statement();
    Statement(const Statement&) = delete;
    Statement& operator=(const Statement&) = delete;

    Statement& bindInt(int index, int value);
    Statement& bindText(int index, const std::string& value);
    Statement& bindDouble(int index, double value);
    Statement& bindNull(int index);
    Statement& bindOptionalInt(int index, std::optional<int> value);
    Statement& bindOptionalDouble(int index, std::optional<double> value);
    Statement& bindOptionalText(int index, const std::optional<std::string>& value);

    bool step();                       // true = a row is ready, false = finished
    int columnInt(int index) const;
    std::string columnText(int index) const;
    double columnDouble(int index) const;
    bool isNull(int index) const;

private:
    void check(int code);
    sqlite3* db_;
    sqlite3_stmt* stmt_ = nullptr;
};
