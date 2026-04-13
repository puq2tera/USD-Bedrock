#include "RevokeUserSession.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>

namespace {

struct RevokeUserSessionRequestModel {
    int64_t sessionID;

    static RevokeUserSessionRequestModel bind(const SData& request) {
        return {RequestBinding::requirePositiveInt64(request, "sessionID")};
    }
};

} // namespace

RevokeUserSession::RevokeUserSession(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool RevokeUserSession::peek(SQLite& db) {
    (void)db;
    (void)RevokeUserSessionRequestModel::bind(request);
    return false;
}

void RevokeUserSession::process(SQLite& db) {
    const RevokeUserSessionRequestModel input = RevokeUserSessionRequestModel::bind(request);
    const int64_t now = STimeNow();

    SQResult sessionResult;
    const string lookupQuery = fmt::format(
        "SELECT userID, expiresAt, COALESCE(revokedAt, ''), COALESCE(replacedBySessionID, '') "
        "FROM user_sessions WHERE sessionID = {} LIMIT 1;",
        input.sessionID
    );
    if (!db.read(lookupQuery, sessionResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch user session for revocation",
            "REVOKE_USER_SESSION_READ_FAILED",
            {{"command", "RevokeUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }
    if (sessionResult.empty() || sessionResult[0].size() < 4) {
        CommandError::notFound(
            "Session not found",
            "REVOKE_USER_SESSION_NOT_FOUND",
            {{"command", "RevokeUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    const int64_t userID = RequestBinding::parseInt64Strict(sessionResult[0][0], "userID");
    const int64_t expiresAt = RequestBinding::parseInt64Strict(sessionResult[0][1], "expiresAt");
    if (!sessionResult[0][2].empty() || !sessionResult[0][3].empty() || expiresAt <= now) {
        CommandError::conflict(
            "Session is no longer valid",
            "REVOKE_USER_SESSION_INVALID",
            {{"command", "RevokeUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    const string updateQuery = fmt::format(
        "UPDATE user_sessions SET revokedAt = {} WHERE sessionID = {} AND revokedAt IS NULL AND replacedBySessionID IS NULL AND expiresAt > {};",
        now,
        input.sessionID,
        now
    );
    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to revoke user session",
            "REVOKE_USER_SESSION_UPDATE_FAILED",
            {{"command", "RevokeUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    SQResult changesResult;
    if (!db.read("SELECT changes()", changesResult) || changesResult.empty() || changesResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify session revocation",
            "REVOKE_USER_SESSION_CHANGES_FAILED",
            {{"command", "RevokeUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }
    if (changesResult[0][0] != "1") {
        CommandError::conflict(
            "Session is no longer valid",
            "REVOKE_USER_SESSION_INVALID",
            {{"command", "RevokeUserSession"}, {"sessionID", SToStr(input.sessionID)}}
        );
    }

    ResponseBinding::setInt64(response, "sessionID", input.sessionID);
    ResponseBinding::setInt64(response, "userID", userID);
    ResponseBinding::setInt64(response, "revokedAt", now);
    ResponseBinding::setString(response, "result", "revoked");
}
