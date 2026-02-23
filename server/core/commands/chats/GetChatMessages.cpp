#include "GetChatMessages.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct GetChatMessagesRequestModel {
    int64_t chatID;
    int64_t userID; // Caller requesting message history; must be a chat member.
    size_t limit; // Page size for message history.
    optional<int64_t> beforeMessageID; // Cursor: return only messages with messageID < this value.

    static GetChatMessagesRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const optional<int64_t> parsedLimit = RequestBinding::optionalInt64(request, "limit", 1, 100);
        const optional<int64_t> beforeMessageID = RequestBinding::optionalInt64(
            request,
            "beforeMessageID",
            1,
            numeric_limits<int64_t>::max()
        );

        return {
            chatID,
            userID,
            static_cast<size_t>(parsedLimit.value_or(20)),
            beforeMessageID,
        };
    }
};

struct GetChatMessagesResponseModel {
    list<string> messages;
    optional<int64_t> nextBeforeMessageID; // Cursor for the next page (oldest ID in this page).

    void writeTo(SData& response) const {
        ResponseBinding::setSize(response, "resultCount", messages.size());
        ResponseBinding::setJSONArray(response, "messages", messages);
        ResponseBinding::setString(response, "format", "json");
        if (nextBeforeMessageID) {
            ResponseBinding::setInt64(response, "nextBeforeMessageID", *nextBeforeMessageID);
        }
    }
};

} // namespace

GetChatMessages::GetChatMessages(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetChatMessages::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void GetChatMessages::process(SQLite& db) {
    buildResponse(db);
}

void GetChatMessages::buildResponse(SQLite& db) {
    const GetChatMessagesRequestModel input = GetChatMessagesRequestModel::bind(request);

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "GetChatMessages",
        "GET_CHAT_MESSAGES_USER_LOOKUP_FAILED",
        "GET_CHAT_MESSAGES_USER_NOT_FOUND"
    );

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "GetChatMessages",
        "GET_CHAT_MESSAGES_CHAT_LOOKUP_FAILED",
        "GET_CHAT_MESSAGES_CHAT_NOT_FOUND"
    );

    (void)ChatAccess::requireMembershipRole(
        db,
        input.chatID,
        input.userID,
        "GetChatMessages",
        "GET_CHAT_MESSAGES_MEMBER_LOOKUP_FAILED",
        "GET_CHAT_MESSAGES_FORBIDDEN"
    );

    string query = fmt::format(
        "SELECT messageID, chatID, userID, body, createdAt, updatedAt FROM messages WHERE chatID = {}",
        input.chatID
    );

    if (input.beforeMessageID) {
        query += fmt::format(" AND messageID < {}", *input.beforeMessageID);
    }

    query += fmt::format(" ORDER BY messageID DESC LIMIT {};", input.limit);

    SQResult result;
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch chat messages",
            "GET_CHAT_MESSAGES_READ_FAILED",
            {{"command", "GetChatMessages"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    list<string> rows;
    optional<int64_t> nextBeforeMessageID;
    for (const vector<string>& row : result) {
        if (row.size() < 6) {
            continue;
        }

        STable item;
        item["messageID"] = row[0];
        item["chatID"] = row[1];
        item["userID"] = row[2];
        item["body"] = row[3];
        item["createdAt"] = row[4];
        item["updatedAt"] = row[5];
        rows.emplace_back(SComposeJSONObject(item));
        nextBeforeMessageID = SToInt64(row[0]);
    }

    const GetChatMessagesResponseModel output = {rows, nextBeforeMessageID};
    output.writeTo(response);
}
