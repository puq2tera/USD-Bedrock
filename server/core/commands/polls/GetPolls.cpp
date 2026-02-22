#include "GetPolls.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct GetPollsRequestModel {
    int64_t limit;

    static GetPollsRequestModel bind(const SData& request) {
        const auto optLimit = RequestBinding::optionalInt64(request, "limit", 1, 100);
        return {optLimit.value_or(50)};
    }
};

struct GetPollsResponseModel {
    size_t resultCount;
    list<string> polls;

    void writeTo(SData& response) const {
        ResponseBinding::setSize(response, "resultCount", resultCount);
        ResponseBinding::setJSONArray(response, "polls", polls);
    }
};

} // namespace

GetPolls::GetPolls(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetPolls::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void GetPolls::process(SQLite& db) {
    buildResponse(db);
}

void GetPolls::buildResponse(SQLite& db) {
    const GetPollsRequestModel input = GetPollsRequestModel::bind(request);

    const string pollsQuery = fmt::format(
        "SELECT p.pollID, p.question, p.createdAt, "
        "  (SELECT COUNT(*) FROM poll_options WHERE pollID = p.pollID) AS optionCount, "
        "  (SELECT COUNT(*) FROM votes WHERE pollID = p.pollID) AS totalVotes "
        "FROM polls p "
        "ORDER BY p.createdAt DESC "
        "LIMIT {};",
        input.limit
    );

    SQResult result;
    if (!db.read(pollsQuery, result)) {
        STHROW("502 Failed to fetch polls");
    }

    list<string> polls;
    for (const auto& row : result) {
        if (row.size() < 5) {
            continue;
        }
        STable poll;
        poll["pollID"] = row[0];
        poll["question"] = row[1];
        poll["createdAt"] = row[2];
        poll["optionCount"] = row[3];
        poll["totalVotes"] = row[4];
        polls.emplace_back(SComposeJSONObject(poll));
    }

    const GetPollsResponseModel output = {polls.size(), polls};
    output.writeTo(response);
}
