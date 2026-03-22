#pragma once

#include "../RequestBinding.h"

#include <libstuff/libstuff.h>

namespace UserAuth {

inline constexpr size_t MIN_PASSWORD_LENGTH = 8;
inline constexpr size_t MAX_PASSWORD_LENGTH = 128;

inline string requirePassword(const SData& request, const char* key = "password") {
    return RequestBinding::requireString(request, key, MIN_PASSWORD_LENGTH, MAX_PASSWORD_LENGTH);
}

inline string hashPasswordWithSalt(const string& password, const string& salt) {
    return SHashSHA256(salt + ":" + password);
}

inline string hashPassword(const string& password) {
    // Store as `salt$digest` so verification can recompute using per-user salt.
    const string seed = SToStr(STimeNow()) + ":" + SHashSHA1(password);
    const string salt = SHashSHA256(seed).substr(0, 32);
    return salt + "$" + hashPasswordWithSalt(password, salt);
}

inline bool verifyPassword(const string& password, const string& storedPasswordHash) {
    const size_t splitPos = storedPasswordHash.find('$');
    if (splitPos == string::npos || splitPos == 0 || splitPos + 1 >= storedPasswordHash.size()) {
        return false;
    }

    const string salt = storedPasswordHash.substr(0, splitPos);
    const string storedDigest = storedPasswordHash.substr(splitPos + 1);
    return hashPasswordWithSalt(password, salt) == storedDigest;
}

} // namespace UserAuth
