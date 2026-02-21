#include "CreatePoll.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>
#include <limits>
#include <set>

namespace {

constexpr size_t MAX_OPTIONS = 20;

string boolToResponse(bool value) {
    return value ? "true" : "false";
}

struct CreatePollRequestModel {
    int64_t chatID;
    int64_t creatorUserID;
    string question;
<<<<<<< HEAD
    string type;
    bool allowChangeVote;
    bool isAnonymous;
    optional<int64_t> expiresAt;
=======
    int64_t createdBy;
>>>>>>> origin/main
    list<string> options;

    static CreatePollRequestModel bind(const SData& request) {
        const int64_t chatID = RequestBinding::requirePositiveInt64(request, "chatID");
        const int64_t creatorUserID = RequestBinding::requirePositiveInt64(request, "creatorUserID");
        const string question = PollCommandUtils::trimAndValidateText(
            RequestBinding::requireString(request, "question", 1, BedrockPlugin::MAX_SIZE_SMALL),
            "question",
            1,
            BedrockPlugin::MAX_SIZE_SMALL,
            "CREATE_POLL_INVALID_QUESTION",
            "CreatePoll"
        );

<<<<<<< HEAD
        string type;
        if (!PollCommandUtils::parsePollType(
                RequestBinding::requireString(request, "type", 1, 64),
                type)) {
            CommandError::badRequest(
                "Invalid parameter: type",
                "CREATE_POLL_INVALID_TYPE",
                {{"command", "CreatePoll"}, {"parameter", "type"}}
            );
        }

        const bool allowChangeVote = RequestBinding::optionalBool(request, "allowChangeVote").value_or(false);
        const bool isAnonymous = RequestBinding::optionalBool(request, "isAnonymous").value_or(false);
        const optional<int64_t> expiresAt = RequestBinding::optionalInt64(
            request,
            "expiresAt",
            1,
            numeric_limits<int64_t>::max()
        );

        optional<list<string>> parsedOptions = RequestBinding::optionalJSONArray(request, "options", 0, MAX_OPTIONS);
        list<string> normalizedOptions;

        if (type == "free_text") {
            if (parsedOptions && !parsedOptions->empty()) {
                CommandError::badRequest(
                    "Free-text polls cannot include options",
                    "CREATE_POLL_OPTIONS_NOT_ALLOWED",
                    {{"command", "CreatePoll"}}
                );
            }
        } else {
            if (!parsedOptions || parsedOptions->size() < 2 || parsedOptions->size() > MAX_OPTIONS) {
                CommandError::badRequest(
                    "Choice polls must include 2-20 options",
                    "CREATE_POLL_INVALID_OPTION_COUNT",
                    {{"command", "CreatePoll"}}
                );
            }

            set<string> seenLabels;
            size_t optionOrdinal = 0;
            for (const string& optionLabel : *parsedOptions) {
                const string normalizedLabel = PollCommandUtils::trimAndValidateText(
                    optionLabel,
                    "options",
                    1,
                    BedrockPlugin::MAX_SIZE_SMALL,
                    "CREATE_POLL_INVALID_OPTION_LABEL",
                    "CreatePoll"
                );

                if (!seenLabels.insert(normalizedLabel).second) {
                    CommandError::badRequest(
                        "Duplicate poll option",
                        "CREATE_POLL_DUPLICATE_OPTION",
                        {{"command", "CreatePoll"}, {"option", normalizedLabel}, {"ord", SToStr(optionOrdinal)}}
                    );
                }

                normalizedOptions.emplace_back(normalizedLabel);
                ++optionOrdinal;
=======
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
>>>>>>> origin/main
            }
        }

        return {
<<<<<<< HEAD
            chatID,
            creatorUserID,
            question,
            type,
            allowChangeVote,
            isAnonymous,
            expiresAt,
            normalizedOptions,
=======
            RequestBinding::requireString(request, "question", 1, BedrockPlugin::MAX_SIZE_SMALL),
            RequestBinding::requirePositiveInt64(request, "createdBy"),
            std::move(opts)
>>>>>>> origin/main
        };
    }
};

struct CreatePollResponseModel {
    int64_t pollID;
    int64_t chatID;
    int64_t creatorUserID;
    string question;
<<<<<<< HEAD
    string type;
    string status;
    bool allowChangeVote;
    bool isAnonymous;
    optional<int64_t> expiresAt;
    int64_t optionCount;
    int64_t createdAt;
    int64_t updatedAt;
=======
    int64_t createdBy;
    size_t optionCount;
    string createdAt;
>>>>>>> origin/main

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "creatorUserID", creatorUserID);
        ResponseBinding::setString(response, "question", question);
<<<<<<< HEAD
        ResponseBinding::setString(response, "type", type);
        ResponseBinding::setString(response, "status", status);
        ResponseBinding::setString(response, "allowChangeVote", boolToResponse(allowChangeVote));
        ResponseBinding::setString(response, "isAnonymous", boolToResponse(isAnonymous));
        ResponseBinding::setString(response, "expiresAt", expiresAt ? SToStr(*expiresAt) : "");
        ResponseBinding::setInt64(response, "optionCount", optionCount);
        ResponseBinding::setInt64(response, "createdAt", createdAt);
        ResponseBinding::setInt64(response, "updatedAt", updatedAt);
=======
        ResponseBinding::setInt64(response, "createdBy", createdBy);
        ResponseBinding::setSize(response, "optionCount", optionCount);
        ResponseBinding::setString(response, "createdAt", createdAt);
>>>>>>> origin/main
    }
};

} // namespace

CreatePoll::CreatePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreatePoll::peek(SQLite& db) {
    (void)db;
    (void)CreatePollRequestModel::bind(request);
    return false;
}

void CreatePoll::process(SQLite& db) {
    const CreatePollRequestModel input = CreatePollRequestModel::bind(request);

<<<<<<< HEAD
    PollCommandUtils::requireChatExists(
        db,
        input.chatID,
        "CreatePoll",
        "CREATE_POLL_CHAT_LOOKUP_FAILED",
        "CREATE_POLL_CHAT_NOT_FOUND"
    );
    PollCommandUtils::requireChatMember(
        db,
        input.chatID,
        input.creatorUserID,
        "CreatePoll",
        "CREATE_POLL_CHAT_MEMBER_LOOKUP_FAILED",
        "CREATE_POLL_CREATOR_NOT_CHAT_MEMBER"
    );

    const int64_t now = PollCommandUtils::nowUnix();
    const string insertPollQuery = fmt::format(
        "INSERT INTO polls (chatID, creatorUserID, question, type, allowChangeVote, isAnonymous, status, "
        "expiresAt, createdAt, updatedAt, closedAt) "
        "VALUES ({}, {}, {}, {}, {}, {}, 'open', {}, {}, {}, NULL);",
        input.chatID,
        input.creatorUserID,
        SQ(input.question),
        SQ(input.type),
        input.allowChangeVote ? 1 : 0,
        input.isAnonymous ? 1 : 0,
        PollCommandUtils::sqlNullableInt(input.expiresAt),
        now,
        now
    );

    if (!db.write(insertPollQuery)) {
=======
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
>>>>>>> origin/main
        CommandError::upstreamFailure(
            db,
            "Failed to insert poll",
            "CREATE_POLL_INSERT_FAILED",
<<<<<<< HEAD
            {
                {"command", "CreatePoll"},
                {"chatID", SToStr(input.chatID)},
                {"creatorUserID", SToStr(input.creatorUserID)}
            }
=======
            {{"command", "CreatePoll"}, {"createdBy", SToStr(input.createdBy)}}
>>>>>>> origin/main
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
<<<<<<< HEAD
            "Failed to retrieve poll ID",
            "CREATE_POLL_LAST_INSERT_ID_FAILED",
            {{"command", "CreatePoll"}}
=======
            "Failed to retrieve pollID",
            "CREATE_POLL_LAST_INSERT_ID_FAILED",
            {{"command", "CreatePoll"}, {"createdBy", SToStr(input.createdBy)}}
>>>>>>> origin/main
        );
    }
    const int64_t pollID = SToInt64(idResult[0][0]);

    size_t ord = 0;
    for (const string& label : input.options) {
        const string insertOptionQuery = fmt::format(
            "INSERT INTO poll_options (pollID, label, ord, isActive) VALUES ({}, {}, {}, 1);",
            pollID,
            SQ(label),
            ord
        );

<<<<<<< HEAD
        if (!db.write(insertOptionQuery)) {
=======
        if (!db.write(insertOption)) {
>>>>>>> origin/main
            CommandError::upstreamFailure(
                db,
                "Failed to insert poll option",
                "CREATE_POLL_OPTION_INSERT_FAILED",
<<<<<<< HEAD
                {{"command", "CreatePoll"}, {"pollID", SToStr(pollID)}, {"ord", SToStr(ord)}}
=======
                {{"command", "CreatePoll"}, {"pollID", pollID}}
>>>>>>> origin/main
            );
        }

        ++ord;
    }

    PollCommandUtils::emitPollEvent(
        db,
        pollID,
        input.creatorUserID,
        "created",
        {
            {"chatID", SToStr(input.chatID)},
            {"type", input.type},
            {"optionCount", SToStr(input.options.size())}
        },
        "CreatePoll",
        "CREATE_POLL_EVENT_INSERT_FAILED"
    );

    const CreatePollResponseModel output = {
        pollID,
        input.chatID,
        input.creatorUserID,
        input.question,
<<<<<<< HEAD
        input.type,
        "open",
        input.allowChangeVote,
        input.isAnonymous,
        input.expiresAt,
        static_cast<int64_t>(input.options.size()),
        now,
        now,
=======
        input.createdBy,
        input.options.size(),
        createdAt,
>>>>>>> origin/main
    };
    output.writeTo(response);

    SINFO("Created poll " << pollID << " in chat " << input.chatID);
}
