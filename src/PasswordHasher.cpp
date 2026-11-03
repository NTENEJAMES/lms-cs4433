#include "PasswordHasher.h"
#include <argon2.h>
#include <fstream>
#include <vector>
#include "Exceptions.h"

#ifdef _WIN32
#include <windows.h>
#include <bcrypt.h>
#endif

namespace {
const std::size_t kSaltBytes = 16;
const std::size_t kHashBytes = 32;

// Cryptographically secure random bytes from the operating system
std::string randomBytes(std::size_t count) {
    std::string out(count, '\0');
#ifdef _WIN32
    if (BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(&out[0]), static_cast<ULONG>(count),
                        BCRYPT_USE_SYSTEM_PREFERRED_RNG) != 0)
        throw SecurityException("Could not get secure random bytes");
#else
    std::ifstream in("/dev/urandom", std::ios::binary);
    if (!in.read(&out[0], static_cast<std::streamsize>(count)))
        throw SecurityException("Could not get secure random bytes");
#endif
    return out;
}

std::string toHex(const std::string& bytes) {
    static const char* digits = "0123456789abcdef";
    std::string out;
    for (unsigned char c : bytes) { out += digits[c >> 4]; out += digits[c & 15]; }
    return out;
}

int hexValue(char c) {
    if (c >= '0' && c <= '9') return c - '0';
    if (c >= 'a' && c <= 'f') return c - 'a' + 10;
    return -1;
}

bool fromHex(const std::string& hex, std::string& out) {
    if (hex.size() % 2 != 0) return false;
    out.clear();
    for (std::size_t i = 0; i < hex.size(); i += 2) {
        int hi = hexValue(hex[i]), lo = hexValue(hex[i + 1]);
        if (hi < 0 || lo < 0) return false;
        out += static_cast<char>(hi * 16 + lo);
    }
    return true;
}

// Compares without stopping at the first difference (prevents timing attacks)
bool constantTimeEquals(const std::string& a, const std::string& b) {
    if (a.size() != b.size()) return false;
    unsigned char diff = 0;
    for (std::size_t i = 0; i < a.size(); ++i)
        diff |= static_cast<unsigned char>(a[i]) ^ static_cast<unsigned char>(b[i]);
    return diff == 0;
}
}  // namespace

PasswordHasher::PasswordHasher(std::uint32_t timeCost, std::uint32_t memoryKiB, std::uint32_t parallelism)
    : timeCost_(timeCost), memoryKiB_(memoryKiB), parallelism_(parallelism) {}

std::string PasswordHasher::compute(const std::string& password, const std::string& salt) const {
    std::vector<unsigned char> out(kHashBytes);
    const int rc = argon2id_hash_raw(timeCost_, memoryKiB_, parallelism_,
                                     password.data(), password.size(),
                                     salt.data(), salt.size(), out.data(), out.size());
    if (rc != ARGON2_OK) throw SecurityException(std::string("Password hashing failed: ") + argon2_error_message(rc));
    return std::string(out.begin(), out.end());
}

PasswordHasher::Result PasswordHasher::hash(const std::string& password) const {
    if (password.empty()) throw InvalidInputException("Password cannot be empty");
    const std::string salt = randomBytes(kSaltBytes);
    return Result{toHex(compute(password, salt)), toHex(salt)};
}

bool PasswordHasher::verify(const std::string& password, const std::string& hashHex,
                            const std::string& saltHex) const {
    std::string expected, salt;
    if (password.empty() || !fromHex(hashHex, expected) || !fromHex(saltHex, salt) || salt.size() < 8)
        return false;
    return constantTimeEquals(compute(password, salt), expected);
}
