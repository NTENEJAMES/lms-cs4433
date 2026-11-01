#include "Roles.h"

void Administrator::showMenu(std::ostream& out) const {
    printMenu(out, "Administrator menu",
              {"Manage users", "Manage departments and courses",
               "Assign lecturers to courses", "Enrol students"});
}

void Lecturer::showMenu(std::ostream& out) const {
    printMenu(out, "Lecturer menu",
              {"Create assignment", "Grant extension", "Mark submission"});
}

HeadOfDepartment::HeadOfDepartment(int userId, std::string username,
                                   std::string passwordHash, std::string salt,
                                   std::string fullName, int departmentId, bool isActive)
    : User(userId, std::move(username), std::move(passwordHash), std::move(salt),
           std::move(fullName), isActive),
      departmentId_(departmentId) {}

void HeadOfDepartment::showMenu(std::ostream& out) const {
    printMenu(out, "Head of Department menu",
              {"Moderate marks", "Release marks", "View audit log"});
}

void Student::showMenu(std::ostream& out) const {
    printMenu(out, "Student menu",
              {"View courses and assignments", "Submit assignment",
               "View released marks and feedback"});
}
