#pragma once

#include "../RequestBinding.h"

#include <libstuff/libstuff.h>

namespace UserValidation {

inline constexpr size_t MAX_EMAIL_LENGTH_WITH_ANGLE_BRACKETS = 256;
inline constexpr size_t MAX_EMAIL_LENGTH = 254;
inline constexpr size_t MIN_EMAIL_LENGTH = 6;
inline constexpr size_t MIN_USERNAME_LENGTH = 1;
inline constexpr size_t MAX_USERNAME_LENGTH = 64;
inline constexpr size_t MIN_DISPLAY_NAME_LENGTH = 1;
inline constexpr size_t MAX_DISPLAY_NAME_LENGTH = 511;

inline const string& emailRegexPattern() {
    static const string pattern = R"EMAIL(^(?:[a-z0-9!#$%&'*+/=?^_`{|}~-]+(?:\.[a-z0-9!#$%&'*+/=?^_`{|}~-]+)*|"(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21\x23-\x5b\x5d-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])*")@(?:(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?|\[(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?|[a-z0-9-]*[a-z0-9]:(?:[\x01-\x08\x0b\x0c\x0e-\x1f\x21-\x5a\x53-\x7f]|\\[\x01-\x09\x0b\x0c\x0e-\x7f])+)\])$)EMAIL";
    return pattern;
}

inline string normalizeAndValidateEmail(const string& rawEmail, const char* key = "email") {
    string email = STrim(rawEmail);
    if (email.empty() || email.size() > MAX_EMAIL_LENGTH_WITH_ANGLE_BRACKETS) {
        RequestBinding::throwInvalid(key);
    }

    if (SStartsWith(SToLower(email), "mailto:")) {
        email = email.substr(7);
    }

    email = SStrip(email, "<>", false);
    email = STrim(email);
    if (email.size() < MIN_EMAIL_LENGTH || email.size() > MAX_EMAIL_LENGTH) {
        RequestBinding::throwInvalid(key);
    }

    // Match the formatEmail reference behavior while allowing case-insensitive input
    // before normalization to lowercase.
    if (!SREMatch(emailRegexPattern(), email, false)) {
        RequestBinding::throwInvalid(key);
    }

    const size_t atPos = email.find('@');
    if (atPos == string::npos || email.find('@', atPos + 1) != string::npos) {
        RequestBinding::throwInvalid(key);
    }
    if (atPos < MIN_USERNAME_LENGTH || atPos > MAX_USERNAME_LENGTH) {
        RequestBinding::throwInvalid(key);
    }

    return SToLower(email);
}

inline string requireEmail(const SData& request, const char* key = "email") {
    const string& rawEmail = RequestBinding::requireString(
        request, key, 1, MAX_EMAIL_LENGTH_WITH_ANGLE_BRACKETS
    );
    return normalizeAndValidateEmail(rawEmail, key);
}

inline optional<string> optionalEmail(const SData& request, const char* key = "email") {
    if (!RequestBinding::isPresent(request, key)) {
        return nullopt;
    }

    return normalizeAndValidateEmail(request[key], key);
}

inline string requireName(const SData& request, const char* key, size_t maxSize = BedrockPlugin::MAX_SIZE_SMALL) {
    string value = STrim(RequestBinding::requireString(request, key, 1, maxSize));
    if (value.empty()) {
        RequestBinding::throwInvalid(key);
    }
    return value;
}

inline optional<string> optionalName(const SData& request, const char* key, size_t maxSize = BedrockPlugin::MAX_SIZE_SMALL) {
    if (!RequestBinding::isPresent(request, key)) {
        return nullopt;
    }

    const optional<string> value = RequestBinding::optionalString(request, key, 1, maxSize);
    if (!value) {
        return nullopt;
    }

    const string trimmed = STrim(*value);
    if (trimmed.empty()) {
        RequestBinding::throwInvalid(key);
    }
    return trimmed;
}

inline string requireDisplayName(const SData& request, const char* key = "displayName") {
    return requireName(request, key, MAX_DISPLAY_NAME_LENGTH);
}

inline optional<string> optionalDisplayName(const SData& request, const char* key = "displayName") {
    return optionalName(request, key, MAX_DISPLAY_NAME_LENGTH);
}

inline string defaultDisplayName(const string& firstName, const string& lastName) {
    const string displayName = STrim(firstName + " " + lastName);
    if (displayName.size() < MIN_DISPLAY_NAME_LENGTH || displayName.size() > MAX_DISPLAY_NAME_LENGTH) {
        RequestBinding::throwInvalid("displayName");
    }
    return displayName;
}

} // namespace UserValidation
