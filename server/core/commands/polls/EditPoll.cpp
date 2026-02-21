#include "EditPoll.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>
#include <set>
#include <vector>

namespace {

constexpr size_t MAX_OPTIONS = 20;

struct EditPollRequestModel {
    int64_t pollID;
    int64_t actorUserID;
    optional<string> question;
    optional<bool> allowChangeVote;
    optional<bool> isAnonymous;
    optional<string> status;
    optional<optional<int64_t>> expiresAt;
    optional<list<string>> options;

    static EditPollRequestModel bind(const SData& request) {
        const int64_t pollID = RequestBinding::requirePositiveInt64(request, "pollID");
        const int64_t actorUserID = RequestBinding::requirePositiveInt64(request, "actorUserID");

        const optional<string> question = RequestBinding::optionalString(
            request,
            "question",
            1,
            BedrockPlugin::MAX_SIZE_SMALL
        );
        const optional<bool> allowChangeVote = RequestBinding::optionalBool(request, "allowChangeVote");
        const optional<bool> isAnonymous = RequestBinding::optionalBool(request, "isAnonymous");

        optional<string> status;
        if (RequestBinding::isPresent(request, "status")) {
            string normalizedStatus;
            if (!PollCommandUtils::parsePollStatus(request["status"], normalizedStatus)) {
                CommandError::badRequest(
                    "Invalid parameter: status",
                    "EDIT_POLL_INVALID_STATUS",
                    {{"command", "EditPoll"}, {"parameter", "status"}}
                );
            }
            status = normalizedStatus;
        }

        optional<optional<int64_t>> expiresAt;
        if (RequestBinding::isPresent(request, "expiresAt")) {
            const string rawExpiresAt = STrim(request["expiresAt"]);
            if (rawExpiresAt.empty() || SIEquals(rawExpiresAt, "null")) {
                expiresAt = optional<int64_t> {};
            } else {
                const int64_t parsed = RequestBinding::parseInt64Strict(rawExpiresAt, "expiresAt");
                if (parsed <= 0) {
                    CommandError::badRequest(
                        "Invalid parameter: expiresAt",
                        "EDIT_POLL_INVALID_EXPIRES_AT",
                        {{"command", "EditPoll"}, {"parameter", "expiresAt"}}
                    );
                }
                expiresAt = parsed;
            }
        }

        optional<list<string>> options;
        if (RequestBinding::isPresent(request, "options")) {
            options = RequestBinding::requireJSONArray(request, "options", 0, MAX_OPTIONS);
        }

        if (!question && !allowChangeVote && !isAnonymous && !status && !expiresAt && !options) {
            CommandError::badRequest(
                "Missing required parameter: at least one editable field",
                "EDIT_POLL_MISSING_FIELDS",
                {{"command", "EditPoll"}}
            );
        }

        return {
            pollID,
            actorUserID,
            question,
            allowChangeVote,
            isAnonymous,
            status,
            expiresAt,
            options,
        };
    }
};

struct EditPollResponseModel {
    int64_t pollID;
    string status;
    int64_t updatedAt;
    int64_t optionCount;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setString(response, "status", status);
        ResponseBinding::setInt64(response, "updatedAt", updatedAt);
        ResponseBinding::setInt64(response, "optionCount", optionCount);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

EditPoll::EditPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditPoll::peek(SQLite& db) {
    (void)db;
    (void)EditPollRequestModel::bind(request);
    return false;
}

void EditPoll::process(SQLite& db) {
    const EditPollRequestModel input = EditPollRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "EditPoll",
        "EDIT_POLL_LOOKUP_FAILED",
        "EDIT_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "EditPoll",
        "EDIT_POLL_CLOSE_EXPIRED_FAILED",
        "EDIT_POLL_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.actorUserID,
        "EditPoll",
        "EDIT_POLL_CHAT_MEMBER_LOOKUP_FAILED",
        "EDIT_POLL_ACTOR_NOT_CHAT_MEMBER"
    );

    if (poll.creatorUserID != input.actorUserID) {
        CommandError::conflict(
            "Only the poll creator can edit this poll",
            "EDIT_POLL_FORBIDDEN",
            {
                {"command", "EditPoll"},
                {"pollID", SToStr(input.pollID)},
                {"actorUserID", SToStr(input.actorUserID)},
                {"creatorUserID", SToStr(poll.creatorUserID)}
            }
        );
    }

    optional<list<string>> normalizedOptions;
    if (input.options) {
        if (poll.type == "free_text") {
            CommandError::badRequest(
                "Free-text polls cannot include options",
                "EDIT_POLL_OPTIONS_NOT_ALLOWED",
                {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}}
            );
        }

        if (input.options->size() < 2 || input.options->size() > MAX_OPTIONS) {
            CommandError::badRequest(
                "Choice polls must include 2-20 options",
                "EDIT_POLL_INVALID_OPTION_COUNT",
                {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}}
            );
        }

        list<string> validated;
        set<string> seen;
        size_t ord = 0;
        for (const string& label : *input.options) {
            const string normalized = PollCommandUtils::trimAndValidateText(
                label,
                "options",
                1,
                BedrockPlugin::MAX_SIZE_SMALL,
                "EDIT_POLL_INVALID_OPTION_LABEL",
                "EditPoll"
            );

            if (!seen.insert(normalized).second) {
                CommandError::badRequest(
                    "Duplicate poll option",
                    "EDIT_POLL_DUPLICATE_OPTION",
                    {{"command", "EditPoll"}, {"ord", SToStr(ord)}, {"option", normalized}}
                );
            }

            validated.emplace_back(normalized);
            ++ord;
        }

        normalizedOptions = validated;
    }

    const int64_t now = PollCommandUtils::nowUnix();

    vector<string> assignments;
    if (input.question) {
        assignments.emplace_back(fmt::format("question = {}", SQ(*input.question)));
    }
    if (input.allowChangeVote.has_value()) {
        assignments.emplace_back(fmt::format("allowChangeVote = {}", *input.allowChangeVote ? 1 : 0));
    }
    if (input.isAnonymous.has_value()) {
        assignments.emplace_back(fmt::format("isAnonymous = {}", *input.isAnonymous ? 1 : 0));
    }
    if (input.status) {
        assignments.emplace_back(fmt::format("status = {}", SQ(*input.status)));
        if (*input.status == "closed") {
            assignments.emplace_back(fmt::format("closedAt = {}", now));
        }
        if (*input.status == "open") {
            assignments.emplace_back("closedAt = NULL");
        }
    }
    if (input.expiresAt.has_value()) {
        assignments.emplace_back(fmt::format("expiresAt = {}", PollCommandUtils::sqlNullableInt(*input.expiresAt)));
    }

    assignments.emplace_back(fmt::format("updatedAt = {}", now));

    const string updateQuery = fmt::format(
        "UPDATE polls SET {} WHERE pollID = {};",
        SComposeList(assignments, ", "),
        input.pollID
    );

    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to update poll",
            "EDIT_POLL_UPDATE_FAILED",
            {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (normalizedOptions) {
        const string deleteVotesQuery = fmt::format(
            "DELETE FROM votes WHERE pollID = {};",
            input.pollID
        );
        if (!db.write(deleteVotesQuery)) {
            CommandError::upstreamFailure(
                db,
                "Failed to delete existing votes",
                "EDIT_POLL_VOTES_DELETE_FAILED",
                {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}}
            );
        }

        const string deleteOptionsQuery = fmt::format(
            "DELETE FROM poll_options WHERE pollID = {};",
            input.pollID
        );
        if (!db.write(deleteOptionsQuery)) {
            CommandError::upstreamFailure(
                db,
                "Failed to replace poll options",
                "EDIT_POLL_OPTIONS_DELETE_FAILED",
                {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}}
            );
        }

        size_t ord = 0;
        for (const string& optionLabel : *normalizedOptions) {
            const string insertOptionQuery = fmt::format(
                "INSERT INTO poll_options (pollID, label, ord, isActive) VALUES ({}, {}, {}, 1);",
                input.pollID,
                SQ(optionLabel),
                ord
            );
            if (!db.write(insertOptionQuery)) {
                CommandError::upstreamFailure(
                    db,
                    "Failed to insert replacement poll option",
                    "EDIT_POLL_OPTION_INSERT_FAILED",
                    {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}, {"ord", SToStr(ord)}}
                );
            }
            ++ord;
        }
    }

    PollCommandUtils::emitPollEvent(
        db,
        input.pollID,
        input.actorUserID,
        "edited",
        {{"updatedAt", SToStr(now)}},
        "EditPoll",
        "EDIT_POLL_EVENT_EDITED_INSERT_FAILED"
    );

    if (input.status) {
        if (poll.status != *input.status && *input.status == "closed") {
            PollCommandUtils::emitPollEvent(
                db,
                input.pollID,
                input.actorUserID,
                "closed",
                {{"reason", "manual"}},
                "EditPoll",
                "EDIT_POLL_EVENT_CLOSED_INSERT_FAILED"
            );
        }
        if (poll.status != *input.status && *input.status == "open") {
            PollCommandUtils::emitPollEvent(
                db,
                input.pollID,
                input.actorUserID,
                "reopened",
                STable {},
                "EditPoll",
                "EDIT_POLL_EVENT_REOPENED_INSERT_FAILED"
            );
        }
    }

    poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "EditPoll",
        "EDIT_POLL_LOOKUP_AFTER_UPDATE_FAILED",
        "EDIT_POLL_NOT_FOUND_AFTER_UPDATE"
    );

    SQResult optionCountResult;
    const string optionCountQuery = fmt::format(
        "SELECT COUNT(*) FROM poll_options WHERE pollID = {};",
        input.pollID
    );
    if (!db.read(optionCountQuery, optionCountResult) || optionCountResult.empty() || optionCountResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to count poll options",
            "EDIT_POLL_OPTION_COUNT_FAILED",
            {{"command", "EditPoll"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    const EditPollResponseModel output = {
        input.pollID,
        poll.status,
        poll.updatedAt,
        SToInt64(optionCountResult[0][0]),
        "updated",
    };
    output.writeTo(response);
}
