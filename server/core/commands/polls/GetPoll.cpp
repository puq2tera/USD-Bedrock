#include "GetPoll.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct GetPollRequestModel {
    int64_t pollID;

    static GetPollRequestModel bind(const SData& request) {
        return {RequestBinding::requirePositiveInt64(request, "pollID")};
    }
};

struct GetPollResponseModel {
    string pollID;
    string question;
    string createdAt;
    list<string> options;
    size_t optionCount;
    int64_t totalVotes;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "pollID", pollID);
        ResponseBinding::setString(response, "question", question);
        ResponseBinding::setString(response, "createdAt", createdAt);
        ResponseBinding::setJSONArray(response, "options", options);
        ResponseBinding::setSize(response, "optionCount", optionCount);
        ResponseBinding::setInt64(response, "totalVotes", totalVotes);
    }
};

} // namespace

GetPoll::GetPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetPoll::peek(SQLite& db) {
    // This is read-only, so we do all the work in peek and return true
    // (true = "I'm done, no need for the write phase")
    buildResponse(db);
    return true;
}

void GetPoll::process(SQLite& db) {
    // Fallback in case peek doesn't complete (e.g., on a follower node)
    buildResponse(db);
}

void GetPoll::buildResponse(SQLite& db) {
    const GetPollRequestModel input = GetPollRequestModel::bind(request);

    // ---- 1. Fetch the poll ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID, question, createdAt FROM polls WHERE pollID = {};",
        input.pollID
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    GetPollResponseModel output = {
        pollResult[0][0],
        pollResult[0][1],
        pollResult[0][2],
        {},
        0,
        0,
    };

    // ---- 2. Fetch the options for this poll ----
    SQResult optionsResult;
    const string optionsQuery = fmt::format(
        "SELECT optionID, text FROM poll_options WHERE pollID = {} ORDER BY optionID;",
        input.pollID
    );

    if (!db.read(optionsQuery, optionsResult)) {
        STHROW("502 Failed to fetch poll options");
    }

    // ---- 3. Count votes per option ----
    SQResult votesResult;
    const string votesQuery = fmt::format(
        "SELECT optionID, COUNT(*) FROM votes WHERE pollID = {} GROUP BY optionID;",
        input.pollID
    );

    // Build a map of optionID → vote count
    map<string, string> voteCounts;
    if (db.read(votesQuery, votesResult)) {
        for (const auto& row : votesResult) {
            if (row.size() >= 2) {
                voteCounts[row[0]] = row[1];
            }
        }
    }

    // ---- 4. Build options array with vote counts ----
    int64_t totalVotes = 0;
    list<string> options;
    for (const auto& row : optionsResult) {
        if (row.size() < 2) {
            continue;
        }
        STable option;
        option["optionID"] = row[0];
        option["text"] = row[1];

        // Look up vote count for this option, default to "0"
        auto it = voteCounts.find(row[0]);
        const string count = (it != voteCounts.end()) ? it->second : "0";
        option["votes"] = count;
        totalVotes += SToInt64(count);

        options.emplace_back(SComposeJSONObject(option));
    }

    output.options = options;
    output.optionCount = output.options.size();
    output.totalVotes = totalVotes;
    output.writeTo(response);
}
