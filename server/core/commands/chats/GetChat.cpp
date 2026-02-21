#include "GetChat.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct GetChatRequestModel {
    int64_t chatID;
    int64_t userID; // Caller requesting chat details.

    static GetChatRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "chatID"),
            RequestBinding::requirePositiveInt64(request, "userID"),
        };
    }
};

struct GetChatResponseModel {
    int64_t chatID;
    int64_t createdByUserID;
    string title;
    string createdAt;
    string requesterRole; // Requester's membership role within this chat.
    size_t memberCount;
    size_t ownerCount;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "createdByUserID", createdByUserID);
        ResponseBinding::setString(response, "title", title);
        ResponseBinding::setString(response, "createdAt", createdAt);
        ResponseBinding::setString(response, "requesterRole", requesterRole);
        ResponseBinding::setSize(response, "memberCount", memberCount);
        ResponseBinding::setSize(response, "ownerCount", ownerCount);
    }
};

size_t readCount(SQLite& db, const string& query, const char* command, const char* errorCode, int64_t chatID) {
    SQResult result;
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to read chat counts",
            errorCode,
            {{"command", command}, {"chatID", SToStr(chatID)}}
        );
    }

    if (result.empty() || result[0].empty()) {
        return 0;
    }

    return static_cast<size_t>(SToInt64(result[0][0]));
}

} // namespace

GetChat::GetChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetChat::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void GetChat::process(SQLite& db) {
    buildResponse(db);
}

void GetChat::buildResponse(SQLite& db) {
    const GetChatRequestModel input = GetChatRequestModel::bind(request);

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "GetChat",
        "GET_CHAT_USER_LOOKUP_FAILED",
        "GET_CHAT_USER_NOT_FOUND"
    );

    SQResult chatResult;
    const string chatQuery = fmt::format(
        "SELECT chatID, title, createdAt, createdByUserID FROM chats WHERE chatID = {};",
        input.chatID
    );
    if (!db.read(chatQuery, chatResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch chat",
            "GET_CHAT_LOOKUP_FAILED",
            {{"command", "GetChat"}, {"chatID", SToStr(input.chatID)}}
        );
    }
    if (chatResult.empty() || chatResult[0].size() < 4) {
        CommandError::notFound(
            "Chat not found",
            "GET_CHAT_NOT_FOUND",
            {{"command", "GetChat"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    const string requesterRole = ChatAccess::requireMembershipRole(
        db,
        input.chatID,
        input.userID,
        "GetChat",
        "GET_CHAT_MEMBER_LOOKUP_FAILED",
        "GET_CHAT_FORBIDDEN"
    );

    const size_t memberCount = readCount(
        db,
        fmt::format("SELECT COUNT(1) FROM chat_members WHERE chatID = {};", input.chatID),
        "GetChat",
        "GET_CHAT_MEMBER_COUNT_FAILED",
        input.chatID
    );
    const size_t ownerCount = readCount(
        db,
        fmt::format("SELECT COUNT(1) FROM chat_members WHERE chatID = {} AND role = 'owner';", input.chatID),
        "GetChat",
        "GET_CHAT_OWNER_COUNT_FAILED",
        input.chatID
    );

    const GetChatResponseModel output = {
        SToInt64(chatResult[0][0]),
        SToInt64(chatResult[0][3]),
        chatResult[0][1],
        chatResult[0][2],
        requesterRole,
        memberCount,
        ownerCount,
    };
    output.writeTo(response);
}
