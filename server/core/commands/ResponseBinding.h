#pragma once

#include <BedrockCommand.h>
#include <libstuff/libstuff.h>

namespace ResponseBinding {

// Centralizing serialization here keeps response models declarative and guarantees that numeric
// fields are string-encoded the same way Bedrock expects across commands.
inline void setString(SData& response, const char* key, const string& value) {
    response[key] = value;
}

inline void setInt64(SData& response, const char* key, int64_t value) {
    response[key] = SToStr(value);
}

inline void setSize(SData& response, const char* key, size_t value) {
    response[key] = SToStr(value);
}

inline void setJSONArray(SData& response, const char* key, const list<string>& values) {
    response[key] = SComposeJSONArray(values);
}

} // namespace ResponseBinding
