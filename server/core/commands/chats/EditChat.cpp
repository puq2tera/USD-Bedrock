#include "EditChat.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "ChatAccess.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct EditChatRequestModel {
    int64_t chatID;
    int64_t userID;
    string title;

    static EditChatRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const string title = STrim(RequestBinding::requireString(request, "title", 1, BedrockPlugin::MAX_SIZE_SMALL));
        if (title.empty()) {
            CommandError::badRequest(
                "Invalid parameter: title",
                "EDIT_CHAT_INVALID_TITLE",
                {{"command", "EditChat"}, {"parameter", "title"}}
            );
        }

        return {chatID, userID, title};
    }
};

struct EditChatResponseModel {
    int64_t chatID;
    string title;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setString(response, "title", title);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

EditChat::EditChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditChat::peek(SQLite& db) {
    (void)db;
    (void)EditChatRequestModel::bind(request);
    return false;
}

void EditChat::process(SQLite& db) {
    const EditChatRequestModel input = EditChatRequestModel::bind(request);

    ChatAccess::ensureChatExists(
        db,
        input.chatID,
        "EditChat",
        "EDIT_CHAT_LOOKUP_FAILED",
        "EDIT_CHAT_NOT_FOUND"
    );

    ChatAccess::requireOwner(
        db,
        input.chatID,
        input.userID,
        "EditChat",
        "EDIT_CHAT_MEMBER_LOOKUP_FAILED",
        "EDIT_CHAT_FORBIDDEN"
    );

    const string updateQuery = fmt::format(
        "UPDATE chats SET title = {} WHERE chatID = {};",
        SQ(input.title),
        input.chatID
    );
    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to update chat",
            "EDIT_CHAT_UPDATE_FAILED",
            {{"command", "EditChat"}, {"chatID", SToStr(input.chatID)}}
        );
    }

    const EditChatResponseModel output = {
        input.chatID,
        input.title,
        "updated",
    };
    output.writeTo(response);

    SINFO("Updated chat " << input.chatID);
}
