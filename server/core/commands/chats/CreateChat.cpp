#include "CreateChat.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct CreateChatRequestModel {
    int64_t creatorUserID;
    string title;

    static CreateChatRequestModel bind(const SData& request) {
        const int64_t creatorUserID = RequestBinding::requirePositiveInt64(request, "creatorUserID");
        const string title = STrim(RequestBinding::requireString(request, "title", 1, BedrockPlugin::MAX_SIZE_SMALL));
        if (title.empty()) {
            CommandError::badRequest(
                "Invalid parameter: title",
                "CREATE_CHAT_INVALID_TITLE",
                {{"command", "CreateChat"}, {"parameter", "title"}}
            );
        }

        return {creatorUserID, title};
    }
};

struct CreateChatResponseModel {
    int64_t chatID;
    int64_t createdByUserID;
    string title;
    string createdAt;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "createdByUserID", createdByUserID);
        ResponseBinding::setString(response, "title", title);
        ResponseBinding::setString(response, "createdAt", createdAt);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

CreateChat::CreateChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreateChat::peek(SQLite& db) {
    (void)db;
    (void)CreateChatRequestModel::bind(request);
    return false;
}

void CreateChat::process(SQLite& db) {
    const CreateChatRequestModel input = CreateChatRequestModel::bind(request);
    const string now = SToStr(STimeNow());

    ChatAccess::ensureUserExists(
        db,
        input.creatorUserID,
        "CreateChat",
        "CREATE_CHAT_CREATOR_LOOKUP_FAILED",
        "CREATE_CHAT_CREATOR_NOT_FOUND"
    );

    const string insertChatQuery = fmt::format(
        "INSERT INTO chats (title, createdAt, createdByUserID) VALUES ({}, {}, {});",
        SQ(input.title),
        now,
        input.creatorUserID
    );
    if (!db.write(insertChatQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert chat",
            "CREATE_CHAT_INSERT_FAILED",
            {{"command", "CreateChat"}, {"creatorUserID", SToStr(input.creatorUserID)}}
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve chatID",
            "CREATE_CHAT_LAST_INSERT_ID_FAILED",
            {{"command", "CreateChat"}, {"creatorUserID", SToStr(input.creatorUserID)}}
        );
    }

    const int64_t chatID = SToInt64(idResult[0][0]);

    const string insertMembershipQuery = fmt::format(
        "INSERT INTO chat_members (chatID, userID, role, joinedAt) VALUES ({}, {}, 'owner', {});",
        chatID,
        input.creatorUserID,
        now
    );
    if (!db.write(insertMembershipQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to add chat creator membership",
            "CREATE_CHAT_OWNER_MEMBERSHIP_INSERT_FAILED",
            {{"command", "CreateChat"}, {"chatID", SToStr(chatID)}, {"creatorUserID", SToStr(input.creatorUserID)}}
        );
    }

    const CreateChatResponseModel output = {
        chatID,
        input.creatorUserID,
        input.title,
        now,
        "created",
    };
    output.writeTo(response);

    SINFO("Created chat " << output.chatID);
}
