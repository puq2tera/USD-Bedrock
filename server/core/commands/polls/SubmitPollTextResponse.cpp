#include "SubmitPollTextResponse.h"

#include "PollCommandUtils.h"
#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

string boolToResponse(bool value) {
    return value ? "true" : "false";
}

struct SubmitPollTextResponseRequestModel {
    int64_t pollID;
    int64_t userID;
    string textValue;

    static SubmitPollTextResponseRequestModel bind(const SData& request) {
        return {
            RequestBinding::requirePositiveInt64(request, "pollID"),
            RequestBinding::requirePositiveInt64(request, "userID"),
            PollCommandUtils::trimAndValidateText(
                RequestBinding::requireString(request, "textValue", 1, BedrockPlugin::MAX_SIZE_QUERY),
                "textValue",
                1,
                BedrockPlugin::MAX_SIZE_QUERY,
                "SUBMIT_POLL_TEXT_RESPONSE_INVALID_TEXT",
                "SubmitPollTextResponse"
            )
        };
    }
};

struct SubmitPollTextResponseResponseModel {
    int64_t responseID;
    int64_t pollID;
    int64_t userID;
    string textValue;
    int64_t createdAt;
    bool replaced;

    void writeTo(SData& response) const {
        ResponseBinding::setInt64(response, "responseID", responseID);
        ResponseBinding::setInt64(response, "pollID", pollID);
        ResponseBinding::setInt64(response, "userID", userID);
        ResponseBinding::setString(response, "textValue", textValue);
        ResponseBinding::setInt64(response, "createdAt", createdAt);
        ResponseBinding::setString(response, "replaced", boolToResponse(replaced));
    }
};

} // namespace

SubmitPollTextResponse::SubmitPollTextResponse(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool SubmitPollTextResponse::peek(SQLite& db) {
    (void)db;
    (void)SubmitPollTextResponseRequestModel::bind(request);
    return false;
}

void SubmitPollTextResponse::process(SQLite& db) {
    const SubmitPollTextResponseRequestModel input = SubmitPollTextResponseRequestModel::bind(request);

    PollCommandUtils::PollRecord poll = PollCommandUtils::getPollOrThrow(
        db,
        input.pollID,
        "SubmitPollTextResponse",
        "SUBMIT_POLL_TEXT_RESPONSE_POLL_LOOKUP_FAILED",
        "SUBMIT_POLL_TEXT_RESPONSE_POLL_NOT_FOUND"
    );

    PollCommandUtils::closePollIfExpired(
        db,
        poll,
        "SubmitPollTextResponse",
        "SUBMIT_POLL_TEXT_RESPONSE_CLOSE_EXPIRED_FAILED",
        "SUBMIT_POLL_TEXT_RESPONSE_EVENT_CLOSE_INSERT_FAILED"
    );

    PollCommandUtils::requireChatMember(
        db,
        poll.chatID,
        input.userID,
        "SubmitPollTextResponse",
        "SUBMIT_POLL_TEXT_RESPONSE_CHAT_MEMBER_LOOKUP_FAILED",
        "SUBMIT_POLL_TEXT_RESPONSE_USER_NOT_CHAT_MEMBER"
    );

    if (poll.type != "free_text") {
        CommandError::badRequest(
            "Poll does not accept free-text responses",
            "SUBMIT_POLL_TEXT_RESPONSE_WRONG_TYPE",
            {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    if (poll.status != "open") {
        CommandError::conflict(
            "Poll is closed",
            "SUBMIT_POLL_TEXT_RESPONSE_POLL_CLOSED",
            {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}}
        );
    }

    SQResult existingResult;
    const string existingQuery = fmt::format(
        "SELECT responseID FROM poll_text_responses WHERE pollID = {} AND userID = {} LIMIT 1;",
        input.pollID,
        input.userID
    );
    if (!db.read(existingQuery, existingResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to check existing poll response",
            "SUBMIT_POLL_TEXT_RESPONSE_EXISTING_LOOKUP_FAILED",
            {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
        );
    }

    const bool hasExisting = !existingResult.empty();
    if (hasExisting && !poll.allowChangeVote) {
        CommandError::conflict(
            "Poll does not allow response changes",
            "SUBMIT_POLL_TEXT_RESPONSE_CHANGE_NOT_ALLOWED",
            {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
        );
    }

    if (hasExisting) {
        const string deleteQuery = fmt::format(
            "DELETE FROM poll_text_responses WHERE pollID = {} AND userID = {};",
            input.pollID,
            input.userID
        );
        if (!db.write(deleteQuery)) {
            CommandError::upstreamFailure(
                db,
                "Failed to replace existing text response",
                "SUBMIT_POLL_TEXT_RESPONSE_REPLACE_DELETE_FAILED",
                {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
            );
        }
    }

    const int64_t now = PollCommandUtils::nowUnix();
    const string insertQuery = fmt::format(
        "INSERT INTO poll_text_responses (pollID, userID, textValue, createdAt) VALUES ({}, {}, {}, {});",
        input.pollID,
        input.userID,
        SQ(input.textValue),
        now
    );
    if (!db.write(insertQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert poll text response",
            "SUBMIT_POLL_TEXT_RESPONSE_INSERT_FAILED",
            {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve response ID",
            "SUBMIT_POLL_TEXT_RESPONSE_LAST_INSERT_ID_FAILED",
            {{"command", "SubmitPollTextResponse"}, {"pollID", SToStr(input.pollID)}, {"userID", SToStr(input.userID)}}
        );
    }
    const int64_t responseID = SToInt64(idResult[0][0]);

    PollCommandUtils::emitPollEvent(
        db,
        input.pollID,
        input.userID,
        "text_submitted",
        {
            {"userID", SToStr(input.userID)},
            {"replaced", hasExisting ? "true" : "false"}
        },
        "SubmitPollTextResponse",
        "SUBMIT_POLL_TEXT_RESPONSE_EVENT_INSERT_FAILED"
    );

    const SubmitPollTextResponseResponseModel output = {
        responseID,
        input.pollID,
        input.userID,
        input.textValue,
        now,
        hasExisting,
    };
    output.writeTo(response);
}
