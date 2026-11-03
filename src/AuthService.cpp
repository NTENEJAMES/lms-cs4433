#include "AuthService.h"
#include "Exceptions.h"

AuthService::AuthService(const UserRepository& users, const PasswordHasher& hasher)
    : users_(users), hasher_(hasher), dummy_(hasher.hash("not-a-real-password")) {}

std::unique_ptr<User> AuthService::login(const std::string& username, const std::string& password) const {
    if (username.empty() || password.empty()) return nullptr;
    std::unique_ptr<User> user = users_.findByUsername(username);
    if (!user) {
        hasher_.verify(password, dummy_.hash, dummy_.salt);   // same work as a real login
        return nullptr;
    }
    if (!hasher_.verify(password, user->passwordHash(), user->salt())) return nullptr;
    if (!user->isActive()) return nullptr;
    return user;
}

void AuthService::validateNewPassword(const std::string& password) {
    if (password.size() < 8) throw InvalidInputException("Password must be at least 8 characters");
}
