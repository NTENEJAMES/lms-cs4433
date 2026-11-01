#include "User.h"
#include "Exceptions.h"

User::User(int userId, std::string username, std::string passwordHash,
           std::string salt, std::string fullName, bool isActive)
    : userId_(userId), username_(std::move(username)),
      passwordHash_(std::move(passwordHash)), salt_(std::move(salt)),
      fullName_(std::move(fullName)), isActive_(isActive) {
    if (username_.empty()) throw InvalidInputException("Username cannot be empty");
    if (fullName_.empty()) throw InvalidInputException("Full name cannot be empty");
}

void User::printMenu(std::ostream& out, const std::string& title,
                     const std::vector<std::string>& items) {
    out << "=== " << title << " ===\n";
    for (std::size_t i = 0; i < items.size(); ++i)
        out << (i + 1) << ". " << items[i] << "\n";
    out << "0. Log out\n";
}
