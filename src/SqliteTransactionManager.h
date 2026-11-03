#pragma once
#include "Database.h"
#include "TransactionManager.h"

class SqliteTransactionManager : public TransactionManager {
public:
    explicit SqliteTransactionManager(Database& db) : db_(db) {}
    void begin() override    { db_.execute("BEGIN;"); }
    void commit() override   { db_.execute("COMMIT;"); }
    void rollback() override { db_.execute("ROLLBACK;"); }
private:
    Database& db_;
};
