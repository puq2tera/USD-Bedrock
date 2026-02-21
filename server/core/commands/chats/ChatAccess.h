#pragma once

#include "../CommandError.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>
#include <sqlitecluster/SQLite.h>

#include <optional>

namespace ChatAccess {

inline bool isValidRole(const string& role) {
    return role == "owner" || role == "member";
}

inline string requireValidRole(const string& role,
                               const char* command,
                               const char* invalidErrorCode,
                               const char* parameter = "role") {
    if (!isValidRole(role)) {
        CommandError::badRequest(
            string("Invalid parameter: ") + parameter,
            invalidErrorCode,
            {{"command", command}, {"parameter", parameter}, {"value", role}}
        );
    }

    return role;
}

inline void ensureUserExists(SQLite& db,
                             int64_t userID,
                             const char* command,
                             const char* lookupFailedErrorCode,
                             const char* notFoundErrorCode) {
    SQResult result;
    const string query = fmt::format("SELECT userID FROM users WHERE userID = {};", userID);
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify user",
            lookupFailedErrorCode,
            {{"command", command}, {"userID", SToStr(userID)}}
        );
    }

    if (result.empty()) {
        CommandError::notFound(
            "User not found",
            notFoundErrorCode,
            {{"command", command}, {"userID", SToStr(userID)}}
        );
    }
}

inline void ensureChatExists(SQLite& db,
                             int64_t chatID,
                             const char* command,
                             const char* lookupFailedErrorCode,
                             const char* notFoundErrorCode) {
    SQResult result;
    const string query = fmt::format("SELECT chatID FROM chats WHERE chatID = {};", chatID);
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat",
            lookupFailedErrorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}}
        );
    }

    if (result.empty()) {
        CommandError::notFound(
            "Chat not found",
            notFoundErrorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}}
        );
    }
}

inline std::optional<string> optionalMembershipRole(SQLite& db,
                                                    int64_t chatID,
                                                    int64_t userID,
                                                    const char* command,
                                                    const char* lookupFailedErrorCode) {
    SQResult result;
    const string query = fmt::format(
        "SELECT role FROM chat_members WHERE chatID = {} AND userID = {} LIMIT 1;",
        chatID,
        userID
    );

    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat member",
            lookupFailedErrorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}, {"userID", SToStr(userID)}}
        );
    }

    if (result.empty() || result[0].empty()) {
        return std::nullopt;
    }

    return result[0][0];
}

inline string requireMembershipRole(SQLite& db,
                                    int64_t chatID,
                                    int64_t userID,
                                    const char* command,
                                    const char* lookupFailedErrorCode,
                                    const char* forbiddenErrorCode,
                                    const string& forbiddenMessage = "User is not a member of this chat") {
    const std::optional<string> role = optionalMembershipRole(
        db,
        chatID,
        userID,
        command,
        lookupFailedErrorCode
    );

    if (!role) {
        // Return 403 instead of 404 for non-members so callers cannot infer chat membership/existence by probing.
        CommandError::throwError(
            403,
            forbiddenMessage,
            forbiddenErrorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}, {"userID", SToStr(userID)}}
        );
    }

    return *role;
}

inline void requireOwner(SQLite& db,
                         int64_t chatID,
                         int64_t userID,
                         const char* command,
                         const char* lookupFailedErrorCode,
                         const char* forbiddenErrorCode,
                         const string& forbiddenMessage = "Only chat owners can perform this action") {
    const string role = requireMembershipRole(
        db,
        chatID,
        userID,
        command,
        lookupFailedErrorCode,
        forbiddenErrorCode,
        "User is not a member of this chat"
    );

    if (role != "owner") {
        CommandError::throwError(
            403,
            forbiddenMessage,
            forbiddenErrorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}, {"userID", SToStr(userID)}, {"role", role}}
        );
    }
}

inline size_t ownerCount(SQLite& db,
                         int64_t chatID,
                         const char* command,
                         const char* readFailedErrorCode) {
    SQResult result;
    const string query = fmt::format(
        "SELECT COUNT(1) FROM chat_members WHERE chatID = {} AND role = 'owner';",
        chatID
    );

    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to count chat owners",
            readFailedErrorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}}
        );
    }

    if (result.empty() || result[0].empty()) {
        return 0;
    }

    return static_cast<size_t>(SToInt64(result[0][0]));
}

} // namespace ChatAccess
