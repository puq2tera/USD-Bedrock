#pragma once

#include "../../config/AppConfig.h"
#include "../CommandError.h"

#include <libstuff/libstuff.h>
#include <sqlitecluster/SQLite.h>

#include <cstddef>
#include <optional>

namespace PollCommandUtils {

inline constexpr size_t MIN_OPTIONS = AppConfig::Poll::MIN_OPTIONS;
inline constexpr size_t MAX_OPTIONS = AppConfig::Poll::MAX_OPTIONS;
// Hard cap while parsing incoming JSON arrays.
// Command-specific rules (for example min/max options by poll type) are validated later.
inline constexpr size_t REQUEST_MAX_OPTIONS = AppConfig::Poll::REQUEST_MAX_OPTIONS;

struct PollRecord {
    int64_t pollID;
    int64_t chatID;
    int64_t creatorUserID;
    string question;
    string type;
    bool allowChangeVote;
    bool isAnonymous;
    string status;
    optional<int64_t> expiresAt;
    int64_t createdAt;
    int64_t updatedAt;
    optional<int64_t> closedAt;
};

int64_t nowUnix();
bool parsePollType(const string& rawValue, string& normalizedType);
bool parsePollStatus(const string& rawValue, string& normalizedStatus);

PollRecord getPollOrThrow(SQLite& db,
                          int64_t pollID,
                          const string& commandName,
                          const string& lookupErrorCode,
                          const string& notFoundErrorCode);

void requireChatExists(SQLite& db,
                       int64_t chatID,
                       const string& commandName,
                       const string& lookupErrorCode,
                       const string& notFoundErrorCode);

void requireChatMember(SQLite& db,
                       int64_t chatID,
                       int64_t userID,
                       const string& commandName,
                       const string& lookupErrorCode,
                       const string& notMemberErrorCode);

void emitPollEvent(SQLite& db,
                   int64_t pollID,
                   optional<int64_t> actorUserID,
                   const string& eventType,
                   const STable& payload,
                   const string& commandName,
                   const string& errorCode);

bool closePollIfExpired(SQLite& db,
                        PollRecord& poll,
                        const string& commandName,
                        const string& closeErrorCode,
                        const string& eventErrorCode);

void autoCloseExpiredPollsInChat(SQLite& db,
                                 int64_t chatID,
                                 const string& commandName,
                                 const string& lookupErrorCode,
                                 const string& closeErrorCode,
                                 const string& eventErrorCode);

optional<int64_t> getPollSummaryMessageID(SQLite& db,
                                          int64_t pollID,
                                          const string& commandName,
                                          const string& errorCode);
optional<int64_t> ensurePollSummaryMessage(SQLite& db,
                                           const PollRecord& poll,
                                           const string& commandName,
                                           const string& readErrorCode,
                                           const string& writeErrorCode);

string sqlNullableInt(const optional<int64_t>& value);
string trimAndValidateText(const string& rawValue,
                           const char* fieldName,
                           size_t minLength,
                           size_t maxLength,
                           const string& errorCode,
                           const string& commandName);

} // namespace PollCommandUtils
