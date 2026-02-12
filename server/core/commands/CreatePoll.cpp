#include "CreatePoll.h"

#include "../Core.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

CreatePoll::CreatePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreatePoll::peek(SQLite& db) {
    (void)db;
    // Validate early so we fail fast before reaching the write phase
    validateRequest();
    return false; // false = "I need the write phase (process), don't stop here"
}

void CreatePoll::process(SQLite& db) {
    validateRequest();

    const string& question = request["question"];
    const string createdAt = SToStr(STimeNow());

    // ---- 1. Insert the poll ----
    const string insertPoll = fmt::format(
        "INSERT INTO polls (question, createdAt) VALUES ({}, {});",
        SQ(question), createdAt
    );

    if (!db.write(insertPoll)) {
        STHROW("502 Failed to insert poll");
    }

    // Get the new poll's ID
    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        STHROW("502 Failed to retrieve pollID");
    }
    const string pollID = idResult[0][0];

    // ---- 2. Insert each option ----
    // Options arrive as a JSON array: ["Red", "Blue", "Green"]
    const list<string> options = SParseJSONArray(request["options"]);

    if (options.size() < 2) {
        STHROW("400 A poll needs at least 2 options");
    }

    for (const string& optionText : options) {
        const string insertOption = fmt::format(
            "INSERT INTO poll_options (pollID, text) VALUES ({}, {});",
            pollID, SQ(optionText)
        );

        if (!db.write(insertOption)) {
            STHROW("502 Failed to insert poll option");
        }
    }

    // ---- 3. Build the response ----
    response["pollID"] = pollID;
    response["question"] = question;
    response["optionCount"] = SToStr(options.size());
    response["createdAt"] = createdAt;

    SINFO("Created poll " << pollID << ": " << question);
}

void CreatePoll::validateRequest() const {
    // question is required, between 1 and 1000 chars
    BedrockPlugin::verifyAttributeSize(request, "question", 1, BedrockPlugin::MAX_SIZE_SMALL);
    // options is required (JSON array string)
    BedrockPlugin::verifyAttributeSize(request, "options", 1, BedrockPlugin::MAX_SIZE_QUERY);
}
