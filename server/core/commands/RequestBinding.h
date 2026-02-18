#pragma once

#include <BedrockPlugin.h>
#include <BedrockCommand.h>
#include <libstuff/libstuff.h>

#include <cerrno>
#include <cstdlib>
#include <limits>
#include <optional>
#include <string>

namespace RequestBinding {

// These helpers are intentionally fail-fast and throw 400-level errors so command code can
// operate on typed values without repeating parameter-validation branches.
[[noreturn]] inline void throwMissing(const char* key) {
    STHROW(string("400 Missing required parameter: ") + key);
}

[[noreturn]] inline void throwInvalid(const char* key, const string& detail = "") {
    if (detail.empty()) {
        STHROW(string("400 Invalid parameter: ") + key);
    }
    STHROW(string("400 Invalid parameter: ") + key + " (" + detail + ")");
}

inline bool isPresent(const SData& request, const char* key) {
    return request.isSet(key) && !request[key].empty();
}

inline const string& requireString(const SData& request,
                                   const char* key,
                                   size_t minSize = 1,
                                   size_t maxSize = static_cast<size_t>(BedrockPlugin::MAX_SIZE_QUERY)) {
    const string& rawValue = request[key];
    if (rawValue.empty()) {
        throwMissing(key);
    }

    if (rawValue.size() < minSize || rawValue.size() > maxSize) {
        throwInvalid(key);
    }

    return rawValue;
}

inline optional<string> optionalString(const SData& request,
                                       const char* key,
                                       size_t minSize = 1,
                                       size_t maxSize = static_cast<size_t>(BedrockPlugin::MAX_SIZE_QUERY)) {
    if (!isPresent(request, key)) {
        return nullopt;
    }

    const string& rawValue = request[key];
    if (rawValue.size() < minSize || rawValue.size() > maxSize) {
        throwInvalid(key);
    }

    return rawValue;
}

inline int64_t parseInt64Strict(const string& rawValue, const char* key) {
    // Keep lexical and numeric parsing separate so values like "1abc" or overflowed numbers are
    // rejected consistently instead of being partially parsed.
    if (!SREMatch("^-?[0-9]+$", rawValue)) {
        throwInvalid(key);
    }

    errno = 0;
    char* parseEnd = nullptr;
    const long long parsed = strtoll(rawValue.c_str(), &parseEnd, 10);
    if (errno == ERANGE || parseEnd == nullptr || *parseEnd != '\0') {
        throwInvalid(key);
    }

    return static_cast<int64_t>(parsed);
}

inline int64_t requireInt64(const SData& request,
                            const char* key,
                            int64_t minValue = numeric_limits<int64_t>::min(),
                            int64_t maxValue = numeric_limits<int64_t>::max()) {
    const string& rawValue = request[key];
    if (rawValue.empty()) {
        throwMissing(key);
    }

    const int64_t parsedValue = parseInt64Strict(rawValue, key);
    if (parsedValue < minValue || parsedValue > maxValue) {
        throwInvalid(key);
    }

    return parsedValue;
}

inline optional<int64_t> optionalInt64(const SData& request,
                                       const char* key,
                                       int64_t minValue = numeric_limits<int64_t>::min(),
                                       int64_t maxValue = numeric_limits<int64_t>::max()) {
    if (!isPresent(request, key)) {
        return nullopt;
    }

    const int64_t parsedValue = parseInt64Strict(request[key], key);
    if (parsedValue < minValue || parsedValue > maxValue) {
        throwInvalid(key);
    }

    return parsedValue;
}

inline int64_t requirePositiveInt64(const SData& request, const char* key) {
    const int64_t parsedValue = requireInt64(request, key, 1, numeric_limits<int64_t>::max());
    return parsedValue;
}

inline bool parseBoolStrict(const string& rawValue, const char* key) {
    if (SIEquals(rawValue, "true") || rawValue == "1") {
        return true;
    }
    if (SIEquals(rawValue, "false") || rawValue == "0") {
        return false;
    }
    throwInvalid(key);
}

inline bool requireBool(const SData& request, const char* key) {
    const string& rawValue = request[key];
    if (rawValue.empty()) {
        throwMissing(key);
    }
    return parseBoolStrict(rawValue, key);
}

inline optional<bool> optionalBool(const SData& request, const char* key) {
    if (!isPresent(request, key)) {
        return nullopt;
    }
    return parseBoolStrict(request[key], key);
}

inline list<string> requireJSONArray(const SData& request,
                                     const char* key,
                                     size_t minItems = 0,
                                     size_t maxItems = numeric_limits<size_t>::max()) {
    const string& rawValue = requireString(request, key, 2);
    list<string> items = SParseJSONArray(rawValue);

    // SParseJSONArray returns empty for both "[]" and malformed payloads; preserve that distinction.
    if (items.empty() && STrim(rawValue) != "[]") {
        throwInvalid(key, "expected JSON array");
    }

    if (items.size() < minItems || items.size() > maxItems) {
        throwInvalid(key, "invalid number of items");
    }

    return items;
}

inline optional<list<string>> optionalJSONArray(const SData& request,
                                                const char* key,
                                                size_t minItems = 0,
                                                size_t maxItems = numeric_limits<size_t>::max()) {
    if (!isPresent(request, key)) {
        return nullopt;
    }
    return requireJSONArray(request, key, minItems, maxItems);
}

} // namespace RequestBinding
