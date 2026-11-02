#pragma once
#include <memory>
#include <string>
#include "Roles.h"

// Factory pattern: creates the right User subclass from the role text.
class UserFactory {
public:
    // departmentId is only used for HOD (0 = not yet assigned to a department)
    static std::unique_ptr<User> create(const std::string& role, int userId,
                                        const std::string& username,
                                        const std::string& passwordHash,
                                        const std::string& salt,
                                        const std::string& fullName,
                                        bool isActive = true, int departmentId = 0);
};
