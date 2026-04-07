#include "CreateUserSession.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>

namespace {

struct CreateUserSessionRequestModel {
    int64_t userID;
    string refreshTokenHash;
    int64_t expiresAt;
    optional<string> userAgent;

    static CreateUserSessionRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "userID"),
            RequestBinding::requireString(request, "refreshTokenHash", 64, 64),
            RequestBinding::requirePositiveInt64(request, "expiresAt"),
            RequestBinding::optionalString(request, "userAgent", 1, BedrockPlugin::MAX_SIZE_SMALL),
        };
    }
};

} // namespace

CreateUserSession::CreateUserSession(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreateUserSession::peek(SQLite& db) {
    (void)db;
    (void)CreateUserSessionRequestModel::bind(request);
    return false;
}

void CreateUserSession::process(SQLite& db) {
    const CreateUserSessionRequestModel input = CreateUserSessionRequestModel::bind(request);
    const int64_t createdAt = STimeNow();
    if (input.expiresAt <= createdAt) {
        CommandError::badRequest(
            "Session expiration must be in the future",
            "CREATE_USER_SESSION_INVALID_EXPIRATION",
            {{"command", "CreateUserSession"}}
        );
    }

    SQResult userResult;
    const string userQuery = fmt::format(
        "SELECT userID FROM users WHERE userID = {} LIMIT 1;",
        input.userID
    );
    if (!db.read(userQuery, userResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify user for session creation",
            "CREATE_USER_SESSION_USER_LOOKUP_FAILED",
            {{"command", "CreateUserSession"}, {"userID", SToStr(input.userID)}}
        );
    }
    if (userResult.empty()) {
        CommandError::notFound(
            "User not found",
            "CREATE_USER_SESSION_USER_NOT_FOUND",
            {{"command", "CreateUserSession"}, {"userID", SToStr(input.userID)}}
        );
    }

    const string userAgentSql = input.userAgent ? SQ(*input.userAgent) : "NULL";
    const string insertQuery = fmt::format(
        "INSERT INTO user_sessions (userID, refreshTokenHash, createdAt, expiresAt, revokedAt, replacedBySessionID, userAgent) "
        "VALUES ({}, {}, {}, {}, NULL, NULL, {});",
        input.userID,
        SQ(input.refreshTokenHash),
        createdAt,
        input.expiresAt,
        userAgentSql
    );
    if (!db.write(insertQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to create user session",
            "CREATE_USER_SESSION_INSERT_FAILED",
            {{"command", "CreateUserSession"}, {"userID", SToStr(input.userID)}}
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve sessionID",
            "CREATE_USER_SESSION_LAST_INSERT_ID_FAILED",
            {{"command", "CreateUserSession"}, {"userID", SToStr(input.userID)}}
        );
    }

    ResponseBinding::setString(response, "sessionID", idResult[0][0]);
    ResponseBinding::setInt64(response, "userID", input.userID);
    ResponseBinding::setInt64(response, "createdAt", createdAt);
    ResponseBinding::setInt64(response, "expiresAt", input.expiresAt);
    if (input.userAgent) {
        ResponseBinding::setString(response, "userAgent", *input.userAgent);
    }
}
