#include "SqliteUserRepository.h"
#include "Exceptions.h"
#include "Statement.h"
#include "UserFactory.h"

namespace {
const char* kSelect =
    "SELECT u.user_id, u.username, u.password_hash, u.salt, u.full_name, u.role, u.is_active, "
    "COALESCE((SELECT d.department_id FROM departments d WHERE d.hod_user_id = u.user_id), 0) "
    "FROM users u ";
}

std::unique_ptr<User> SqliteUserRepository::findOne(const std::string& whereClause,
                                                    const std::string& text, int number) const {
    Statement st(db_, std::string(kSelect) + whereClause);
    if (!text.empty()) st.bindText(1, text); else st.bindInt(1, number);
    if (!st.step()) return nullptr;
    return UserFactory::create(st.columnText(5), st.columnInt(0), st.columnText(1), st.columnText(2),
                               st.columnText(3), st.columnText(4), st.columnInt(6) != 0, st.columnInt(7));
}

std::unique_ptr<User> SqliteUserRepository::findByUsername(const std::string& username) const {
    if (username.empty()) return nullptr;
    return findOne("WHERE u.username = ?", username, 0);
}

std::unique_ptr<User> SqliteUserRepository::findById(int userId) const {
    return findOne("WHERE u.user_id = ?", "", userId);
}

int SqliteUserRepository::save(const User& user) {
    if (user.userId() == 0) {
        Statement st(db_, "INSERT INTO users(username, password_hash, salt, full_name, role, is_active) "
                          "VALUES(?, ?, ?, ?, ?, ?)");
        st.bindText(1, user.username()).bindText(2, user.passwordHash()).bindText(3, user.salt())
          .bindText(4, user.fullName()).bindText(5, user.role()).bindInt(6, user.isActive() ? 1 : 0);
        st.step();
        return static_cast<int>(db_.lastInsertId());
    }
    Statement st(db_, "UPDATE users SET username = ?, password_hash = ?, salt = ?, full_name = ?, "
                      "role = ?, is_active = ? WHERE user_id = ?");
    st.bindText(1, user.username()).bindText(2, user.passwordHash()).bindText(3, user.salt())
      .bindText(4, user.fullName()).bindText(5, user.role()).bindInt(6, user.isActive() ? 1 : 0)
      .bindInt(7, user.userId());
    st.step();
    if (db_.changes() == 0) throw InvalidInputException("No such user to update");
    return user.userId();
}
