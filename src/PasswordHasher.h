#pragma once
#include <cstdint>
#include <string>

// Hashes passwords with Argon2id (the reference library from the Password Hashing
// Competition). Every password gets its own random 16-byte salt.
// Hash and salt are returned as hexadecimal text, ready for users.password_hash / users.salt.
class PasswordHasher {
public:
    struct Result {
        std::string hash;
        std::string salt;
    };

    // Defaults follow the OWASP minimum for Argon2id: 19 MiB memory, 2 passes, 1 lane.
    // Tests use cheaper settings so they run fast.
    explicit PasswordHasher(std::uint32_t timeCost = 2, std::uint32_t memoryKiB = 19456,
                            std::uint32_t parallelism = 1);

    Result hash(const std::string& password) const;          // new random salt each time
    bool verify(const std::string& password, const std::string& hashHex,
                const std::string& saltHex) const;

private:
    std::string compute(const std::string& password, const std::string& salt) const;
    std::uint32_t timeCost_;
    std::uint32_t memoryKiB_;
    std::uint32_t parallelism_;
};
