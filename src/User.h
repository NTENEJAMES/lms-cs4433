#pragma once
#include <ostream>
#include <string>
#include <vector>

// Abstract base class for all four kinds of user.
class User {
public:
    User(int userId, std::string username, std::string passwordHash,
         std::string salt, std::string fullName, bool isActive = true);
    virtual ~User() = default;

    int userId() const { return userId_; }
    const std::string& username() const { return username_; }
    const std::string& passwordHash() const { return passwordHash_; }
    const std::string& salt() const { return salt_; }
    const std::string& fullName() const { return fullName_; }
    bool isActive() const { return isActive_; }

    // Role name exactly as stored in users.role
    virtual std::string role() const = 0;
    // Writes the menu this role is allowed to use (role-based access, FR-01)
    virtual void showMenu(std::ostream& out) const = 0;

protected:
    static void printMenu(std::ostream& out, const std::string& title,
                          const std::vector<std::string>& items);

private:
    int userId_;
    std::string username_;
    std::string passwordHash_;
    std::string salt_;
    std::string fullName_;
    bool isActive_;
};
