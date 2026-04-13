#include "GetUserSessionByRefreshTokenHash.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>

namespace {

struct GetUserSessionByRefreshTokenHashRequestModel {
    string refreshTokenHash;

    static GetUserSessionByRefreshTokenHashRequestModel bind(const SData& request) {
        return {RequestBinding::requireString(request, "refreshTokenHash", 64, 64)};
    }
};

} // namespace

GetUserSessionByRefreshTokenHash::GetUserSessionByRefreshTokenHash(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetUserSessionByRefreshTokenHash::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void GetUserSessionByRefreshTokenHash::process(SQLite& db) {
    buildResponse(db);
}

void GetUserSessionByRefreshTokenHash::buildResponse(SQLite& db) {
    const GetUserSessionByRefreshTokenHashRequestModel input = GetUserSessionByRefreshTokenHashRequestModel::bind(request);

    SQResult result;
    const string query = fmt::format(
        "SELECT sessionID, userID, refreshTokenHash, createdAt, expiresAt, "
        "COALESCE(revokedAt, ''), COALESCE(replacedBySessionID, ''), COALESCE(userAgent, '') "
        "FROM user_sessions WHERE refreshTokenHash = {} LIMIT 1;",
        SQ(input.refreshTokenHash)
    );
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch user session",
            "GET_USER_SESSION_BY_HASH_READ_FAILED",
            {{"command", "GetUserSessionByRefreshTokenHash"}}
        );
    }
    if (result.empty() || result[0].size() < 8) {
        CommandError::notFound(
            "Session not found",
            "GET_USER_SESSION_BY_HASH_NOT_FOUND",
            {{"command", "GetUserSessionByRefreshTokenHash"}}
        );
    }

    ResponseBinding::setString(response, "sessionID", result[0][0]);
    ResponseBinding::setString(response, "userID", result[0][1]);
    ResponseBinding::setString(response, "refreshTokenHash", result[0][2]);
    ResponseBinding::setString(response, "createdAt", result[0][3]);
    ResponseBinding::setString(response, "expiresAt", result[0][4]);
    if (!result[0][5].empty()) {
        ResponseBinding::setString(response, "revokedAt", result[0][5]);
    }
    if (!result[0][6].empty()) {
        ResponseBinding::setString(response, "replacedBySessionID", result[0][6]);
    }
    if (!result[0][7].empty()) {
        ResponseBinding::setString(response, "userAgent", result[0][7]);
    }
}
