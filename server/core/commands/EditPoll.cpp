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

    // ---- 2. Require at least one field to update ----
    const string& question = request["question"];
    const string& optionsStr = request["options"];
    if (question.empty() && optionsStr.empty()) {
        STHROW("400 Must provide question or options to update");
    }

    // ---- 3. Update the question if provided ----
    if (!question.empty()) {
        const string trimmedQuestion = SStrip(question);
        if (trimmedQuestion.empty()) {
            STHROW("400 Question cannot be empty or blank");
        }

        const string updateQuery = fmt::format(
            "UPDATE polls SET question = {} WHERE pollID = {};",
            SQ(trimmedQuestion), SQ(pollID)
        );

        if (!db.write(updateQuery)) {
            STHROW("502 Failed to update poll question");
        }
    }

    // ---- 4. Replace options if provided ----
    if (!optionsStr.empty()) {
        const list<string> options = SParseJSONArray(optionsStr);

        if (options.size() < 2) {
            STHROW("400 A poll needs at least 2 options");
        }

        if (options.size() > 20) {
            STHROW("400 A poll cannot have more than 20 options");
        }

        // Check for empty/blank option text and duplicates
        set<string> seenOptions;
        for (const string& optionText : options) {
            const string trimmed = SStrip(optionText);
            if (trimmed.empty()) {
                STHROW("400 Option text cannot be empty");
            }
            if (!seenOptions.insert(trimmed).second) {
                STHROW("400 Duplicate option: " + trimmed);
            }
        }

        // Delete votes first (old optionIDs become invalid)
        const string deleteVotes = fmt::format(
            "DELETE FROM votes WHERE pollID = {};",
            SQ(pollID)
        );

        if (!db.write(deleteVotes)) {
            STHROW("502 Failed to clear votes");
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
                SQ(pollID), SQ(SStrip(optionText))
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

    if (SToInt64(request["pollID"]) <= 0) {
        STHROW("400 pollID must be a positive integer");
    }
}
