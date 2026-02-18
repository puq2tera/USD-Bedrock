#include "CreateMessage.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct CreateMessageRequestModel {
    string name;
    string message;

    static CreateMessageRequestModel bind(const SData& request) {
        return {
            RequestBinding::requireString(request, "name", 1, BedrockPlugin::MAX_SIZE_SMALL),
            RequestBinding::requireString(request, "message", 1, BedrockPlugin::MAX_SIZE_QUERY),
        };
    }
};

struct CreateMessageResponseModel {
    string result;
    string messageID;
    string name;
    string message;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "result", result);
        ResponseBinding::setString(response, "messageID", messageID);
        ResponseBinding::setString(response, "name", name);
        ResponseBinding::setString(response, "message", message);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

CreateMessage::CreateMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreateMessage::peek(SQLite& db) {
    (void)db;
    (void)CreateMessageRequestModel::bind(request);
    return false;
}

void CreateMessage::process(SQLite& db) {
    const CreateMessageRequestModel input = CreateMessageRequestModel::bind(request);
    const string createdAt = SToStr(STimeNow());

    const string query = fmt::format(
        "INSERT INTO messages (name, message, createdAt) VALUES ({}, {}, {});",
        SQ(input.name), SQ(input.message), createdAt
    );

    if (!db.write(query)) {
        STHROW("502 Failed to insert message");
    }

    SQResult result;
    const string selectQuery = "SELECT last_insert_rowid()";
    if (!db.read(selectQuery, result) || result.empty() || result[0].empty()) {
        STHROW("502 Failed to retrieve inserted messageID");
    }

    const CreateMessageResponseModel output = {
        "stored",
        result[0][0],
        input.name,
        input.message,
        createdAt,
    };
    output.writeTo(response);
}
