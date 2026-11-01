#include <gtest/gtest.h>
#include "Database.h"

TEST(DatabaseTest, OpensInMemoryAndRunsSql) {
    Database db(":memory:");
    EXPECT_NO_THROW(db.execute("CREATE TABLE t (id INTEGER PRIMARY KEY);"));
}

TEST(DatabaseTest, BadSqlThrowsInsteadOfCrashing) {
    Database db(":memory:");
    EXPECT_THROW(db.execute("NOT VALID SQL"), DatabaseException);
}
