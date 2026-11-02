#pragma once
#include "Database.h"
#include "UserRepository.h"

class SqliteUserRepository : public UserRepository {
public:
    explicit SqliteUserRepository(Database& db) : db_(db) {}
    std::unique_ptr<User> findByUsername(const std::string& username) const override;
    std::unique_ptr<User> findById(int userId) const override;
    int save(const User& user) override;
private:
    std::unique_ptr<User> findOne(const std::string& whereClause, const std::string& text, int number) const;
    Database& db_;
};
