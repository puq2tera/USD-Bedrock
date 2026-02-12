#include "EditPoll.h"

#include "../Core.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

EditPoll::EditPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditPoll::peek(SQLite& db) {
    (void)db;
    validateRequest();
    return false; // Need the write phase
}

void EditPoll::process(SQLite& db) {
    validateRequest();

    const string& pollID = request["pollID"];

    // ---- 1. Verify the poll exists ----
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID FROM polls WHERE pollID = {};",
        SQ(pollID)
    );

    if (!db.read(pollQuery, pollResult) || pollResult.empty()) {
        STHROW("404 Poll not found");
    }

    // ---- 2. Update the question if provided ----
    const string& question = request["question"];
    if (!question.empty()) {
        const string updateQuery = fmt::format(
            "UPDATE polls SET question = {} WHERE pollID = {};",
            SQ(question), SQ(pollID)
        );

        if (!db.write(updateQuery)) {
            STHROW("502 Failed to update poll question");
        }
    }

    // ---- 3. Replace options if provided ----
    const string& optionsStr = request["options"];
    if (!optionsStr.empty()) {
        const list<string> options = SParseJSONArray(optionsStr);

        if (options.size() < 2) {
            STHROW("400 A poll needs at least 2 options");
        }

        // Delete existing options
        const string deleteQuery = fmt::format(
            "DELETE FROM poll_options WHERE pollID = {};",
            SQ(pollID)
        );

        if (!db.write(deleteQuery)) {
            STHROW("502 Failed to delete old options");
        }

        // Insert new options
        for (const string& optionText : options) {
            const string insertOption = fmt::format(
                "INSERT INTO poll_options (pollID, text) VALUES ({}, {});",
                SQ(pollID), SQ(optionText)
            );

            if (!db.write(insertOption)) {
                STHROW("502 Failed to insert poll option");
            }
        }

        response["optionCount"] = SToStr(options.size());
    }

    // ---- 4. Build the response ----
    response["pollID"] = pollID;
    response["result"] = "updated";

    SINFO("Updated poll " << pollID);
}

void EditPoll::validateRequest() const {
    BedrockPlugin::verifyAttributeSize(request, "pollID", 1, BedrockPlugin::MAX_SIZE_SMALL);
    // At least one of question or options must be provided, but we don't
    // enforce that here — if neither is sent, the command is a no-op.
}
