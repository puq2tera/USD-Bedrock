#include "GetMessages.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct GetMessagesRequestModel {
    size_t limit;

    static GetMessagesRequestModel bind(const SData& request) {
        const optional<int64_t> parsedLimit = RequestBinding::optionalInt64(request, "limit", 1, 100);
        const size_t limit = parsedLimit ? static_cast<size_t>(*parsedLimit) : 20;
        return {limit};
    }
};

struct GetMessagesResponseModel {
    list<string> messages;

    void writeTo(SData& response) const {
        ResponseBinding::setSize(response, "resultCount", messages.size());
        ResponseBinding::setJSONArray(response, "messages", messages);
        ResponseBinding::setString(response, "format", "json");
    }
};

} // namespace

GetMessages::GetMessages(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetMessages::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void GetMessages::process(SQLite& db) {
    buildResponse(db);
}

void GetMessages::buildResponse(SQLite& db) {
    const GetMessagesRequestModel input = GetMessagesRequestModel::bind(request);

    const string query = fmt::format(
        "SELECT messageID, name, message, createdAt "
        "FROM messages "
        "ORDER BY messageID DESC "
        "LIMIT {}",
        input.limit
    );

    SQResult result;
    if (!db.read(query, result)) {
        STHROW("502 Failed to fetch messages");
    }

    list<string> rows;
    for (const auto& row : result) {
        if (row.size() < 4) {
            continue;
        }
        STable item;
        item["messageID"] = row[0];
        item["name"] = row[1];
        item["message"] = row[2];
        item["createdAt"] = row[3];
        rows.emplace_back(SComposeJSONObject(item));
    }

    const GetMessagesResponseModel output = {rows};
    output.writeTo(response);
}
