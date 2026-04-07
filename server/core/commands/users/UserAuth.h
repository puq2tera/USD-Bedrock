#pragma once

#include "../RequestBinding.h"

#include <cerrno>
#include <fstream>
#include <libstuff/libstuff.h>
#include <vector>

namespace UserAuth {

inline constexpr size_t MIN_PASSWORD_LENGTH = 8;
inline constexpr size_t MAX_PASSWORD_LENGTH = 128;
inline constexpr int PBKDF2_ITERATIONS = 120000;
inline constexpr size_t SALT_BYTES = 16;
inline constexpr size_t DERIVED_KEY_BYTES = 32;

inline string requirePassword(const SData& request, const char* key = "password") {
    return RequestBinding::requireString(request, key, MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH);
}

inline string toHex(const string& raw) {
    static constexpr char HEX[] = "0123456789abcdef";
    string encoded;
    encoded.reserve(raw.size() * 2);
    for (unsigned char c : raw) {
        encoded.push_back(HEX[c >> 4]);
        encoded.push_back(HEX[c & 0x0f]);
    }
    return encoded;
}

inline int hexValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }
    if (c >= 'a' && c <= 'f') {
        return 10 + (c - 'a');
    }
    if (c >= 'A' && c <= 'F') {
        return 10 + (c - 'A');
    }
    return -1;
}

inline optional<string> fromHex(const string& encoded) {
    if (encoded.size() % 2 != 0) {
        return nullopt;
    }

    string decoded;
    decoded.reserve(encoded.size() / 2);
    for (size_t i = 0; i < encoded.size(); i += 2) {
        const int high = hexValue(encoded[i]);
        const int low = hexValue(encoded[i + 1]);
        if (high < 0 || low < 0) {
            return nullopt;
        }
        decoded.push_back(static_cast<char>((high << 4) | low));
    }
    return decoded;
}

inline string secureRandomBytes(size_t size) {
    // Password salts must come from a cryptographically secure source so identical passwords do
    // not produce reusable offline-attack material across accounts.
    std::ifstream random("/dev/urandom", std::ios::in | std::ios::binary);
    if (!random) {
        STHROW("Unable to open /dev/urandom for password salt generation");
    }

    string bytes(size, '\0');
    random.read(bytes.data(), static_cast<streamsize>(size));
    if (random.gcount() != static_cast<streamsize>(size)) {
        STHROW("Unable to read /dev/urandom for password salt generation");
    }

    return bytes;
}

inline string hmacSha256Raw(const string& key, const string& message) {
    const string digest = SHMACSHA256(key, message);
    if (digest.size() != DERIVED_KEY_BYTES) {
        STHROW("Unable to compute HMAC-SHA256 digest");
    }
    return digest;
}

inline string pbkdf2Sha256(const string& password, const string& salt, int iterations) {
    if (iterations <= 0) {
        STHROW("PBKDF2 iterations must be positive");
    }

    string blockInput = salt;
    blockInput.push_back('\0');
    blockInput.push_back('\0');
    blockInput.push_back('\0');
    blockInput.push_back('\x01');

    string accumulator = hmacSha256Raw(password, blockInput);
    string previous = accumulator;

    for (int i = 1; i < iterations; ++i) {
        previous = hmacSha256Raw(password, previous);
        for (size_t j = 0; j < accumulator.size(); ++j) {
            accumulator[j] = static_cast<char>(static_cast<unsigned char>(accumulator[j]) ^
                                               static_cast<unsigned char>(previous[j]));
        }
    }

    return accumulator;
}

inline bool constantTimeEquals(const string& left, const string& right) {
    if (left.size() != right.size()) {
        return false;
    }

    unsigned char diff = 0;
    for (size_t i = 0; i < left.size(); ++i) {
        diff |= static_cast<unsigned char>(left[i]) ^ static_cast<unsigned char>(right[i]);
    }
    return diff == 0;
}

inline vector<string> split(const string& value, char delimiter) {
    vector<string> parts;
    size_t start = 0;
    while (start <= value.size()) {
        const size_t next = value.find(delimiter, start);
        parts.push_back(value.substr(start, next == string::npos ? string::npos : next - start));
        if (next == string::npos) {
            break;
        }
        start = next + 1;
    }
    return parts;
}

inline optional<int64_t> parseIterations(const string& rawValue) {
    if (!SREMatch("^[0-9]+$", rawValue)) {
        return nullopt;
    }

    errno = 0;
    char* parseEnd = nullptr;
    const long long parsed = strtoll(rawValue.c_str(), &parseEnd, 10);
    if (errno == ERANGE || parseEnd == nullptr || *parseEnd != '\0' || parsed <= 0 || parsed > 10000000) {
        return nullopt;
    }

    return static_cast<int64_t>(parsed);
}

inline string hashPassword(const string& password) {
    const string salt = secureRandomBytes(SALT_BYTES);
    const string digest = pbkdf2Sha256(password, salt, PBKDF2_ITERATIONS);
    return "pbkdf2_sha256$" + SToStr(PBKDF2_ITERATIONS) + "$" + toHex(salt) + "$" + toHex(digest);
}

inline bool verifyPassword(const string& password, const string& storedPasswordHash) {
    const vector<string> parts = split(storedPasswordHash, '$');
    if (parts.size() != 4 || parts[0] != "pbkdf2_sha256") {
        return false;
    }

    const optional<int64_t> iterations = parseIterations(parts[1]);
    if (!iterations) {
        return false;
    }

    const optional<string> salt = fromHex(parts[2]);
    const optional<string> storedDigest = fromHex(parts[3]);
    if (!salt || !storedDigest || salt->size() != SALT_BYTES || storedDigest->size() != DERIVED_KEY_BYTES) {
        return false;
    }

    const string calculatedDigest = pbkdf2Sha256(password, *salt, static_cast<int>(*iterations));
    return constantTimeEquals(calculatedDigest, *storedDigest);
}

} // namespace UserAuth
