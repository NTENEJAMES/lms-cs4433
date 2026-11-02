#include "UserFactory.h"
#include "Exceptions.h"

std::unique_ptr<User> UserFactory::create(const std::string& role, int userId,
                                          const std::string& username,
                                          const std::string& passwordHash,
                                          const std::string& salt,
                                          const std::string& fullName,
                                          bool isActive, int departmentId) {
    if (role == "ADMIN")
        return std::make_unique<Administrator>(userId, username, passwordHash, salt, fullName, isActive);
    if (role == "LECTURER")
        return std::make_unique<Lecturer>(userId, username, passwordHash, salt, fullName, isActive);
    if (role == "HOD")
        return std::make_unique<HeadOfDepartment>(userId, username, passwordHash, salt, fullName,
                                                  departmentId, isActive);
    if (role == "STUDENT")
        return std::make_unique<Student>(userId, username, passwordHash, salt, fullName, isActive);
    throw InvalidInputException("Unknown role: " + role);
}
