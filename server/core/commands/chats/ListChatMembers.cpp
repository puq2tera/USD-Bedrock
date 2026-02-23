#include "ListChatMembers.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct ListChatMembersRequestModel {
    int64_t chatID;
    int64_t userID; // Caller requesting membership list; must be an owner.

    static ListChatMembersRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "chatID"),
            RequestBinding::requirePositiveInt64(request, "userID"),
        };
    }
};

struct ListChatMembersResponseModel {
    list<string> members;

    void writeTo(SData& response) const {
        ResponseBinding::setSize(response, "resultCount", members.size());
        ResponseBinding::setJSONArray(response, "members", members);
        ResponseBinding::setString(response, "format", "json");
    }
};

} // namespace

ListChatMembers::ListChatMembers(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool ListChatMembers::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void ListChatMembers::process(SQLite& db) {
    buildResponse(db);
}

void ListChatMembers::buildResponse(SQLite& db) {
    const ListChatMembersRequestModel input = ListChatMembersRequestModel::bind(request);

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "ListChatMembers",
        "LIST_CHAT_MEMBERS_CHAT_LOOKUP_FAILED",
        "LIST_CHAT_MEMBERS_CHAT_NOT_FOUND"
    );

    ChatAccess::requireOwner(
        db,
        input.chatID,
        input.userID,
        "ListChatMembers",
        "LIST_CHAT_MEMBERS_ROLE_LOOKUP_FAILED",
        "LIST_CHAT_MEMBERS_FORBIDDEN"
    );

    SQResult result;
    const string query = fmt::format(
        "SELECT userID, role, joinedAt FROM chat_members WHERE chatID = {} ORDER BY joinedAt ASC, userID ASC;",
        input.chatID
    );
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to list chat members",
            "LIST_CHAT_MEMBERS_READ_FAILED",
            {{"command", "ListChatMembers"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    list<string> rows;
    for (const vector<string>& row : result) {
        if (row.size() < 3) {
            continue;
        }

        STable item;
        item["chatID"] = SToStr(input.chatID);
        item["userID"] = row[0];
        item["role"] = row[1];
        item["joinedAt"] = row[2];
        rows.emplace_back(SComposeJSONObject(item));
    }

    const ListChatMembersResponseModel output = {rows};
    output.writeTo(response);
}
