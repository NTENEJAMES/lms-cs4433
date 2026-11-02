#pragma once
#include <memory>
#include <string>
#include "User.h"

// Repository pattern: the rest of the program never sees SQL.
class UserRepository {
public:
    virtual ~UserRepository() = default;
    // Returns nullptr when there is no such user
    virtual std::unique_ptr<User> findByUsername(const std::string& username) const = 0;
    virtual std::unique_ptr<User> findById(int userId) const = 0;
    // Inserts when userId is 0, otherwise updates. Returns the user's id.
    virtual int save(const User& user) = 0;
};
