#include "RotateUserSession.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>

namespace {

struct RotateUserSessionRequestModel {
    int64_t sessionID;
    string refreshTokenHash;
    int64_t expiresAt;
    optional<string> userAgent;

    static RotateUserSessionRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "sessionID"),
            RequestBinding::requireString(request, "refreshTokenHash", 64, 64),
            RequestBinding::requirePositiveInt64(request, "expiresAt"),
            RequestBinding::optionalString(request, "userAgent", 1, BedrockPlugin::MAX_SIZE_SMALL),
        };
    }
};

} // namespace

RotateUserSession::RotateUserSession(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool RotateUserSession::peek(SQLite& db) {
    (void)db;
    (void)RotateUserSessionRequestModel::bind(request);
    return false;
}

void RotateUserSession::process(SQLite& db) {
    const RotateUserSessionRequestModel input = RotateUserSessionRequestModel::bind(request);
    const int64_t now = STimeNow();
    if (input.expiresAt <= now) {
        CommandError::badRequest(
            "Session expiration must be in the future",
            "ROTATE_USER_SESSION_INVALID_EXPIRATION",
            {{"command", "RotateUserSession"}}
        );
    }

    SQResult sessionResult;
    const string lookupQuery = fmt::format(
        "SELECT userID, createdAt, expiresAt, COALESCE(revokedAt, ''), COALESCE(replacedBySessionID, ''), COALESCE(userAgent, '') "
        "FROM user_sessions WHERE sessionID = {} LIMIT 1;",
        input.sessionID
    );
    if (!db.read(lookupQuery, sessionResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch user session for rotation",
            "ROTATE_USER_SESSION_READ_FAILED",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }
    if (sessionResult.empty() || sessionResult[0].size() < 6) {
        CommandError::notFound(
            "Session not found",
            "ROTATE_USER_SESSION_NOT_FOUND",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    const int64_t userID = RequestBinding::parseInt64Strict(sessionResult[0][0], "userID");
    const int64_t currentExpiresAt = RequestBinding::parseInt64Strict(sessionResult[0][2], "expiresAt");
    const bool revoked = !sessionResult[0][3].empty() || !sessionResult[0][4].empty();
    if (revoked || currentExpiresAt <= now) {
        CommandError::conflict(
            "Session is no longer valid",
            "ROTATE_USER_SESSION_INVALID",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    const string nextUserAgent = input.userAgent ? *input.userAgent : sessionResult[0][5];
    const string nextUserAgentSql = nextUserAgent.empty() ? "NULL" : SQ(nextUserAgent);
    const string insertQuery = fmt::format(
        "INSERT INTO user_sessions (userID, refreshTokenHash, createdAt, expiresAt, revokedAt, replacedBySessionID, userAgent) "
        "VALUES ({}, {}, {}, {}, NULL, NULL, {});",
        userID,
        SQ(input.refreshTokenHash),
        now,
        input.expiresAt,
        nextUserAgentSql
    );
    if (!db.write(insertQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to create replacement session",
            "ROTATE_USER_SESSION_INSERT_FAILED",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    SQResult newSessionResult;
    if (!db.read("SELECT last_insert_rowid()", newSessionResult) || newSessionResult.empty() || newSessionResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve replacement sessionID",
            "ROTATE_USER_SESSION_LAST_INSERT_ID_FAILED",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }
    const string& newSessionID = newSessionResult[0][0];

    const string updateQuery = fmt::format(
        "UPDATE user_sessions SET revokedAt = {}, replacedBySessionID = {} "
        "WHERE sessionID = {} AND revokedAt IS NULL AND replacedBySessionID IS NULL AND expiresAt > {};",
        now,
        newSessionID,
        input.sessionID,
        now
    );
    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to revoke rotated session",
            "ROTATE_USER_SESSION_UPDATE_FAILED",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    SQResult changesResult;
    if (!db.read("SELECT changes()", changesResult) || changesResult.empty() || changesResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify rotated session update",
            "ROTATE_USER_SESSION_CHANGES_FAILED",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }
    if (changesResult[0][0] != "1") {
        db.write("DELETE FROM user_sessions WHERE sessionID = " + newSessionID + ";");
        CommandError::conflict(
            "Session is no longer valid",
            "ROTATE_USER_SESSION_INVALID",
            {{"command", "RotateUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    ResponseBinding::setString(response, "sessionID", newSessionID);
    ResponseBinding::setInt64(response, "userID", userID);
    ResponseBinding::setInt64(response, "createdAt", now);
    ResponseBinding::setInt64(response, "expiresAt", input.expiresAt);
    ResponseBinding::setInt64(response, "replacedSessionID", input.sessionID);
    if (!nextUserAgent.empty()) {
        ResponseBinding::setString(response, "userAgent", nextUserAgent);
    }
}
