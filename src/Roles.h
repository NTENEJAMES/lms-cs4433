#pragma once
#include "User.h"

class Administrator : public User {
public:
    using User::User;
    std::string role() const override { return "ADMIN"; }
    void showMenu(std::ostream& out) const override;
};

class Lecturer : public User {
public:
    using User::User;
    std::string role() const override { return "LECTURER"; }
    void showMenu(std::ostream& out) const override;
};

class HeadOfDepartment : public User {
public:
    HeadOfDepartment(int userId, std::string username, std::string passwordHash,
                     std::string salt, std::string fullName, int departmentId,
                     bool isActive = true);
    int departmentId() const { return departmentId_; }
    std::string role() const override { return "HOD"; }
    void showMenu(std::ostream& out) const override;
private:
    int departmentId_;
};

class Student : public User {
public:
    using User::User;
    std::string role() const override { return "STUDENT"; }
    void showMenu(std::ostream& out) const override;
};
