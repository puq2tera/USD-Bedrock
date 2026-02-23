#include "ListChats.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct ListChatsRequestModel {
    int64_t userID; // Caller whose memberships define the result set.
    size_t limit; // Page size for chat listing.
    optional<int64_t> beforeChatID; // Cursor: return chats with chatID < this value.

    static ListChatsRequestModel bind(const SData& request) {
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const optional<int64_t> parsedLimit = RequestBinding::optionalInt64(request, "limit", 1, 100);
        const optional<int64_t> beforeChatID = RequestBinding::optionalInt64(
            request,
            "beforeChatID",
            1,
            numeric_limits<int64_t>::max()
        );

        return {
            userID,
            static_cast<size_t>(parsedLimit.value_or(20)),
            beforeChatID,
        };
    }
};

struct ListChatsResponseModel {
    list<string> chats;

    void writeTo(SData& response) const {
        ResponseBinding::setSize(response, "resultCount", chats.size());
        ResponseBinding::setJSONArray(response, "chats", chats);
        ResponseBinding::setString(response, "format", "json");
    }
};

} // namespace

ListChats::ListChats(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool ListChats::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void ListChats::process(SQLite& db) {
    buildResponse(db);
}

void ListChats::buildResponse(SQLite& db) {
    const ListChatsRequestModel input = ListChatsRequestModel::bind(request);

    ChatAccess::ensureUserExists(
        db,
        input.userID,
        "ListChats",
        "LIST_CHATS_USER_LOOKUP_FAILED",
        "LIST_CHATS_USER_NOT_FOUND"
    );

    // Join through chat_members so each row includes the caller's role in that chat.
    string query = fmt::format(
        "SELECT c.chatID, c.title, c.createdAt, c.createdByUserID, cm.role "
        "FROM chats c "
        "INNER JOIN chat_members cm ON cm.chatID = c.chatID "
        "WHERE cm.userID = {}",
        input.userID
    );

    if (input.beforeChatID) {
        query += fmt::format(" AND c.chatID < {}", *input.beforeChatID);
    }

    query += fmt::format(" ORDER BY c.chatID DESC LIMIT {};", input.limit);

    SQResult result;
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to list chats",
            "LIST_CHATS_READ_FAILED",
            {{"command", "ListChats"}, {"userID", SToStr(input.userID)}}
        );
    }

    list<string> rows;
    for (const vector<string>& row : result) {
        if (row.size() < 5) {
            continue;
        }

        STable item;
        item["chatID"] = row[0];
        item["title"] = row[1];
        item["createdAt"] = row[2];
        item["createdByUserID"] = row[3];
        item["requesterRole"] = row[4];
        rows.emplace_back(SComposeJSONObject(item));
    }

    const ListChatsResponseModel output = {rows};
    output.writeTo(response);
}
