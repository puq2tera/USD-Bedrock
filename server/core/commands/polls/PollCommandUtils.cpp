#include "PollCommandUtils.h"

#include <fmt/format.h>

namespace PollCommandUtils {

namespace {

optional<int64_t> parseNullableIntCell(const string& rawCell) {
    if (rawCell.empty()) {
        return nullopt;
    }
    return SToInt64(rawCell);
}

PollRecord rowToPollRecord(const SQResultRow& row) {
    if (row.size() < 12) {
        STHROW("Poll row missing required columns");
    }

    return {
        SToInt64(row[0]),
        SToInt64(row[1]),
        SToInt64(row[2]),
        row[3],
        row[4],
        row[5] == "1",
        row[6] == "1",
        row[7],
        parseNullableIntCell(row[8]),
        SToInt64(row[9]),
        SToInt64(row[10]),
        parseNullableIntCell(row[11]),
    };
}

void closePoll(SQLite& db,
               int64_t pollID,
               int64_t closedAt,
               const string& commandName,
               const string& closeErrorCode) {
    const string closeQuery = fmt::format(
        "UPDATE polls "
        "SET status = 'closed', updatedAt = {}, closedAt = COALESCE(closedAt, {}) "
        "WHERE pollID = {} AND status = 'open';",
        closedAt,
        closedAt,
        pollID
    );

    if (!db.write(closeQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to close poll",
            closeErrorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }
}

} // namespace

int64_t nowUnix() {
    return static_cast<int64_t>(STimeNow());
}

bool parsePollType(const string& rawValue, string& normalizedType) {
    if (SIEquals(rawValue, "single_choice")) {
        normalizedType = "single_choice";
        return true;
    }
    if (SIEquals(rawValue, "multiple_choice")) {
        normalizedType = "multiple_choice";
        return true;
    }
    if (SIEquals(rawValue, "free_text")) {
        normalizedType = "free_text";
        return true;
    }
    return false;
}

bool parsePollStatus(const string& rawValue, string& normalizedStatus) {
    if (SIEquals(rawValue, "open")) {
        normalizedStatus = "open";
        return true;
    }
    if (SIEquals(rawValue, "closed")) {
        normalizedStatus = "closed";
        return true;
    }
    return false;
}

PollRecord getPollOrThrow(SQLite& db,
                          int64_t pollID,
                          const string& commandName,
                          const string& lookupErrorCode,
                          const string& notFoundErrorCode) {
    SQResult pollResult;
    const string pollQuery = fmt::format(
        "SELECT pollID, chatID, creatorUserID, question, type, allowChangeVote, isAnonymous, "
        "status, IFNULL(expiresAt, ''), createdAt, updatedAt, IFNULL(closedAt, '') "
        "FROM polls WHERE pollID = {};",
        pollID
    );

    if (!db.read(pollQuery, pollResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to query poll",
            lookupErrorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }

    if (pollResult.empty()) {
        CommandError::notFound(
            "Poll not found",
            notFoundErrorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}}
        );
    }

    return rowToPollRecord(pollResult[0]);
}

void requireChatExists(SQLite& db,
                       int64_t chatID,
                       const string& commandName,
                       const string& lookupErrorCode,
                       const string& notFoundErrorCode) {
    SQResult chatResult;
    const string chatQuery = fmt::format(
        "SELECT chatID FROM chats WHERE chatID = {};",
        chatID
    );

    if (!db.read(chatQuery, chatResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat",
            lookupErrorCode,
            {{"command", commandName}, {"chatID", SToStr(chatID)}}
        );
    }

    if (chatResult.empty()) {
        CommandError::notFound(
            "Chat not found",
            notFoundErrorCode,
            {{"command", commandName}, {"chatID", SToStr(chatID)}}
        );
    }
}

void requireChatMember(SQLite& db,
                       int64_t chatID,
                       int64_t userID,
                       const string& commandName,
                       const string& lookupErrorCode,
                       const string& notMemberErrorCode) {
    SQResult membershipResult;
    const string membershipQuery = fmt::format(
        "SELECT 1 FROM chat_members WHERE chatID = {} AND userID = {} LIMIT 1;",
        chatID,
        userID
    );

    if (!db.read(membershipQuery, membershipResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify chat membership",
            lookupErrorCode,
            {
                {"command", commandName},
                {"chatID", SToStr(chatID)},
                {"userID", SToStr(userID)}
            }
        );
    }

    if (membershipResult.empty()) {
        CommandError::conflict(
            "User is not a member of this chat",
            notMemberErrorCode,
            {
                {"command", commandName},
                {"chatID", SToStr(chatID)},
                {"userID", SToStr(userID)}
            }
        );
    }
}

void emitPollEvent(SQLite& db,
                   int64_t pollID,
                   optional<int64_t> actorUserID,
                   const string& eventType,
                   const STable& payload,
                   const string& commandName,
                   const string& errorCode) {
    const string createdAt = SToStr(nowUnix());
    const string actorSQL = actorUserID ? SToStr(*actorUserID) : "NULL";
    const string payloadJson = SComposeJSONObject(payload);
    const string query = fmt::format(
        "INSERT INTO poll_events (pollID, actorUserID, eventType, payloadJSON, createdAt) "
        "VALUES ({}, {}, {}, {}, {});",
        pollID,
        actorSQL,
        SQ(eventType),
        SQ(payloadJson),
        createdAt
    );

    if (!db.write(query)) {
        CommandError::upstreamFailure(
            db,
            "Failed to persist poll event",
            errorCode,
            {{"command", commandName}, {"pollID", SToStr(pollID)}, {"eventType", eventType}}
        );
    }
}

bool closePollIfExpired(SQLite& db,
                        PollRecord& poll,
                        const string& commandName,
                        const string& closeErrorCode,
                        const string& eventErrorCode) {
    if (poll.status != "open" || !poll.expiresAt || *poll.expiresAt > nowUnix()) {
        return false;
    }

    const int64_t closedAt = nowUnix();
    closePoll(db, poll.pollID, closedAt, commandName, closeErrorCode);
    emitPollEvent(
        db,
        poll.pollID,
        nullopt,
        "closed",
        {{"reason", "expired"}},
        commandName,
        eventErrorCode
    );

    poll = getPollOrThrow(
        db,
        poll.pollID,
        commandName,
        closeErrorCode,
        "POLL_NOT_FOUND_AFTER_CLOSE"
    );
    return true;
}

void autoCloseExpiredPollsInChat(SQLite& db,
                                 int64_t chatID,
                                 const string& commandName,
                                 const string& lookupErrorCode,
                                 const string& closeErrorCode,
                                 const string& eventErrorCode) {
    const int64_t now = nowUnix();
    SQResult pollResult;
    const string query = fmt::format(
        "SELECT pollID "
        "FROM polls "
        "WHERE chatID = {} AND status = 'open' AND expiresAt IS NOT NULL AND expiresAt <= {};",
        chatID,
        now
    );

    if (!db.read(query, pollResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to evaluate poll expiry",
            lookupErrorCode,
            {{"command", commandName}, {"chatID", SToStr(chatID)}}
        );
    }

    for (const SQResultRow& row : pollResult) {
        if (row.empty()) {
            continue;
        }
        const int64_t pollID = SToInt64(row[0]);
        closePoll(db, pollID, now, commandName, closeErrorCode);
        emitPollEvent(
            db,
            pollID,
            nullopt,
            "closed",
            {{"reason", "expired"}},
            commandName,
            eventErrorCode
        );
    }
}

string sqlNullableInt(const optional<int64_t>& value) {
    if (!value) {
        return "NULL";
    }
    return SToStr(*value);
}

string trimAndValidateText(const string& rawValue,
                           const char* fieldName,
                           size_t minLength,
                           size_t maxLength,
                           const string& errorCode,
                           const string& commandName) {
    const string trimmed = STrim(rawValue);
    if (trimmed.size() < minLength || trimmed.size() > maxLength) {
        CommandError::badRequest(
            string("Invalid parameter: ") + fieldName,
            errorCode,
            {{"command", commandName}, {"parameter", fieldName}}
        );
    }
    return trimmed;
}

} // namespace PollCommandUtils
