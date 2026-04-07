#include "UserSessionsTable.h"

#include "../TableUtils.h"

namespace Tables::UserSessionsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE user_sessions (
            sessionID INTEGER PRIMARY KEY AUTOINCREMENT,
            userID INTEGER NOT NULL,
            refreshTokenHash TEXT NOT NULL UNIQUE,
            createdAt INTEGER NOT NULL,
            expiresAt INTEGER NOT NULL,
            revokedAt INTEGER,
            replacedBySessionID INTEGER,
            userAgent TEXT,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE,
            FOREIGN KEY (replacedBySessionID) REFERENCES user_sessions(sessionID) ON DELETE SET NULL,
            CHECK (userID > 0),
            CHECK (length(trim(refreshTokenHash)) = 64),
            CHECK (createdAt > 0),
            CHECK (expiresAt > createdAt),
            CHECK (revokedAt IS NULL OR revokedAt >= createdAt),
            CHECK (replacedBySessionID IS NULL OR replacedBySessionID > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "user_sessions", schema);
    TableUtils::verifyIndex(db, "user_sessions_userID_idx", "user_sessions", "(userID)");
    TableUtils::verifyIndex(db, "user_sessions_expiresAt_idx", "user_sessions", "(expiresAt)");
}

} // namespace Tables::UserSessionsTable
