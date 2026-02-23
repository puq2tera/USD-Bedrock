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

string boolToResponse(bool value) {
    return value ? "true" : "false";
}

struct CreatePollRequestModel {
    int64_t chatID;
    int64_t creatorUserID;
    string question;
    string type;
    bool allowChangeVote;
    bool isAnonymous;
    optional<int64_t> expiresAt; // Optional auto-close timestamp (unix epoch seconds).
    list<string> options; // Choice labels in display order; empty only for free_text polls.

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

        // API sends options as a JSON string. Parse it here so later logic works with typed labels.
        optional<list<string>> parsedOptions = RequestBinding::optionalJSONArray(
            request,
            "options",
            0,
            PollCommandUtils::REQUEST_MAX_OPTIONS
        );
        list<string> normalizedOptions;

        // Free-text polls do not have selectable options.
        if (type == "free_text") {
            if (parsedOptions && !parsedOptions->empty()) {
                CommandError::badRequest(
                    "Free-text polls cannot include options",
                    "CREATE_POLL_OPTIONS_NOT_ALLOWED",
                    {{"command", "CreatePoll"}}
                );
            }
        } else {
            if (!parsedOptions || parsedOptions->empty()) {
                CommandError::badRequest(
                    "Choice polls must include options",
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
                    // Duplicate labels make results ambiguous, so reject before writing.
                    CommandError::badRequest(
                        "Duplicate poll option",
                        "CREATE_POLL_DUPLICATE_OPTION",
                        {{"command", "CreatePoll"}, {"option", normalizedLabel}, {"ord", SToStr(optionOrdinal)}}
                    );
                }

                normalizedOptions.emplace_back(normalizedLabel);
                ++optionOrdinal;
            }
        }

        return {
            chatID,
            creatorUserID,
            question,
            type,
            allowChangeVote,
            isAnonymous,
            expiresAt,
            normalizedOptions,
        };
    }
};

struct CreatePollResponseModel {
    int64_t pollID;
    int64_t chatID;
    int64_t creatorUserID;
    string question;
    string type;
    string status;
    bool allowChangeVote;
    bool isAnonymous;
    optional<int64_t> expiresAt;
    int64_t optionCount; // Number of persisted poll_options rows.
    int64_t createdAt;
    int64_t updatedAt;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "chatID", chatID);
        ResponseBinding::setInt64(response, "creatorUserID", creatorUserID);
        ResponseBinding::setString(response, "question", question);
        ResponseBinding::setString(response, "type", type);
        ResponseBinding::setString(response, "status", status);
        ResponseBinding::setString(response, "allowChangeVote", boolToResponse(allowChangeVote));
        ResponseBinding::setString(response, "isAnonymous", boolToResponse(isAnonymous));
        ResponseBinding::setString(response, "expiresAt", expiresAt ? SToStr(*expiresAt) : "");
        ResponseBinding::setInt64(response, "optionCount", optionCount);
        ResponseBinding::setInt64(response, "createdAt", createdAt);
        ResponseBinding::setInt64(response, "updatedAt", updatedAt);
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

    if (input.type != "free_text" &&
        (input.options.size() < PollCommandUtils::MIN_OPTIONS ||
         input.options.size() > PollCommandUtils::MAX_OPTIONS)) {
        CommandError::badRequest(
            fmt::format(
                "Choice polls must include {}-{} options",
                PollCommandUtils::MIN_OPTIONS,
                PollCommandUtils::MAX_OPTIONS
            ),
            "CREATE_POLL_INVALID_OPTION_COUNT",
            {{"command", "CreatePoll"}}
        );
    }

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
        CommandError::upstreamFailure(
            db,
            "Failed to insert poll",
            "CREATE_POLL_INSERT_FAILED",
            {
                {"command", "CreatePoll"},
                {"chatID", SToStr(input.chatID)},
                {"creatorUserID", SToStr(input.creatorUserID)}
            }
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve poll ID",
            "CREATE_POLL_LAST_INSERT_ID_FAILED",
            {{"command", "CreatePoll"}}
        );
    }
    // last_insert_rowid() is scoped to this DB connection, so this is the poll we just inserted.
    const int64_t pollID = SToInt64(idResult[0][0]);

    size_t ord = 0;
    for (const string& label : input.options) {
        const string insertOptionQuery = fmt::format(
            "INSERT INTO poll_options (pollID, label, ord, isActive) VALUES ({}, {}, {}, 1);",
            pollID,
            SQ(label),
            ord
        );

        if (!db.write(insertOptionQuery)) {
            CommandError::upstreamFailure(
                db,
                "Failed to insert poll option",
                "CREATE_POLL_OPTION_INSERT_FAILED",
                {{"command", "CreatePoll"}, {"pollID", SToStr(pollID)}, {"ord", SToStr(ord)}}
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
        input.type,
        "open",
        input.allowChangeVote,
        input.isAnonymous,
        input.expiresAt,
        static_cast<int64_t>(input.options.size()),
        now,
        now,
    };
    output.writeTo(response);

    SINFO("Created poll " << pollID << " in chat " << input.chatID);
}
