#pragma once
#include <memory>
#include <string>
#include "PasswordHasher.h"
#include "UserRepository.h"

class AuthService {
public:
    AuthService(const UserRepository& users, const PasswordHasher& hasher);

    // Returns the user when the username and password are right and the account is active.
    // Returns nullptr in every other case, without saying which part was wrong.
    std::unique_ptr<User> login(const std::string& username, const std::string& password) const;

    // New passwords must be at least 8 characters. Throws InvalidInputException.
    static void validateNewPassword(const std::string& password);

private:
    const UserRepository& users_;
    const PasswordHasher& hasher_;
    PasswordHasher::Result dummy_;   // used so unknown usernames take as long as real ones
};
