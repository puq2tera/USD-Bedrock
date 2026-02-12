#include "GetPoll.h"

#include "../Core.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

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
    // Validate pollID is provided
    const string& pollID = request["pollID"];
    if (pollID.empty()) {
        STHROW("400 Missing required parameter: pollID");
    }

    // ---- 1. Fetch the poll ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID, question, createdAt FROM polls WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    response["pollID"] = pollResult[0][0];
    response["question"] = pollResult[0][1];
    response["createdAt"] = pollResult[0][2];

    // ---- 2. Fetch the options for this poll ----
    SQResult optionsResult;
    const string optionsQuery = fmt::format(
        "SELECT optionID, text FROM poll_options WHERE pollID = {} ORDER BY optionID;",
        SQ(pollID)
    );

    if (!db.read(optionsQuery, optionsResult)) {
        STHROW("502 Failed to fetch poll options");
    }

    list<string> options;
    for (const auto& row : optionsResult) {
        if (row.size() < 2) {
            continue;
        }
        STable option;
        option["optionID"] = row[0];
        option["text"] = row[1];
        options.emplace_back(SComposeJSONObject(option));
    }

    response["options"] = SComposeJSONArray(options);
    response["optionCount"] = SToStr(options.size());
}
