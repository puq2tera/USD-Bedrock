#include "CreatePoll.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct CreatePollRequestModel {
    string question;
    int64_t createdBy;
    list<string> options;

    static CreatePollRequestModel bind(const SData& request) {
        auto opts = RequestBinding::requireJSONArray(request, "options", 2, 20);

        set<string> seen;
        for (const string& text : opts) {
            const string trimmed = SStrip(text);
            if (trimmed.empty()) {
                CommandError::badRequest(
                    "Option text cannot be empty",
                    "CREATE_POLL_OPTION_EMPTY",
                    {{"command", "CreatePoll"}}
                );
            }
            if (!seen.insert(trimmed).second) {
                CommandError::badRequest(
                    "Duplicate option: " + trimmed,
                    "CREATE_POLL_OPTION_DUPLICATE",
                    {{"command", "CreatePoll"}, {"option", trimmed}}
                );
            }
        }

        return {
            RequestBinding::requireString(request, "question", 1, BedrockPlugin::MAX_SIZE_SMALL),
            RequestBinding::requirePositiveInt64(request, "createdBy"),
            std::move(opts)
        };
    }
};

struct CreatePollResponseModel {
    string pollID;
    string question;
    int64_t createdBy;
    size_t optionCount;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "pollID", pollID);
        ResponseBinding::setString(response, "question", question);
        ResponseBinding::setInt64(response, "createdBy", createdBy);
        ResponseBinding::setSize(response, "optionCount", optionCount);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

CreatePoll::CreatePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreatePoll::peek(SQLite& db) {
    (void)db;
    (void)CreatePollRequestModel::bind(request);
    return false; // false = "I need the write phase (process), don't stop here"
}

void CreatePoll::process(SQLite& db) {
    const CreatePollRequestModel input = CreatePollRequestModel::bind(request);
    const string createdAt = SToStr(STimeNow());

    SQResult userResult;
    const string userQuery = fmt::format(
        "SELECT userID FROM users WHERE userID = {};",
        input.createdBy
    );
    if (!db.read(userQuery, userResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify poll creator",
            "CREATE_POLL_CREATOR_LOOKUP_FAILED",
            {{"command", "CreatePoll"}, {"createdBy", SToStr(input.createdBy)}}
        );
    }
    if (userResult.empty()) {
        CommandError::notFound(
            "User not found",
            "CREATE_POLL_CREATOR_NOT_FOUND",
            {{"command", "CreatePoll"}, {"createdBy", SToStr(input.createdBy)}}
        );
    }

    // ---- 1. Insert the poll ----
    const string insertPoll = fmt::format(
        "INSERT INTO polls (question, createdAt, createdBy) VALUES ({}, {}, {});",
        SQ(input.question), createdAt, input.createdBy
    );

    if (!db.write(insertPoll)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert poll",
            "CREATE_POLL_INSERT_FAILED",
            {{"command", "CreatePoll"}, {"createdBy", SToStr(input.createdBy)}}
        );
    }

    // Get the new poll's ID
    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve pollID",
            "CREATE_POLL_LAST_INSERT_ID_FAILED",
            {{"command", "CreatePoll"}, {"createdBy", SToStr(input.createdBy)}}
        );
    }
    const string pollID = idResult[0][0];

    // ---- 2. Insert each option ----
    for (const string& optionText : input.options) {
        const string insertOption = fmt::format(
            "INSERT INTO poll_options (pollID, text) VALUES ({}, {});",
            pollID, SQ(optionText)
        );

        if (!db.write(insertOption)) {
            CommandError::upstreamFailure(
                db,
                "Failed to insert poll option",
                "CREATE_POLL_OPTION_INSERT_FAILED",
                {{"command", "CreatePoll"}, {"pollID", pollID}}
            );
        }
    }

    // ---- 3. Build the response ----
    const CreatePollResponseModel output = {
        pollID,
        input.question,
        input.createdBy,
        input.options.size(),
        createdAt,
    };
    output.writeTo(response);

    SINFO("Created poll " << pollID << ": " << input.question);
}
