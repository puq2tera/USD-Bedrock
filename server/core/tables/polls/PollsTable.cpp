#include "PollsTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::PollsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE polls (
            pollID INTEGER PRIMARY KEY AUTOINCREMENT,
            chatID INTEGER NOT NULL,
            creatorUserID INTEGER NOT NULL,
            question TEXT NOT NULL,
            type TEXT NOT NULL,
            allowChangeVote INTEGER NOT NULL,
            isAnonymous INTEGER NOT NULL,
            status TEXT NOT NULL,
            expiresAt INTEGER,
            createdAt INTEGER NOT NULL,
            updatedAt INTEGER NOT NULL,
            closedAt INTEGER,
            FOREIGN KEY (chatID) REFERENCES chats(chatID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (creatorUserID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (length(trim(question)) BETWEEN 1 AND 1000),
            CHECK (type IN ('single_choice', 'multiple_choice', 'free_text', 'ranked_choice')),
            CHECK (allowChangeVote IN (0, 1)),
            CHECK (isAnonymous IN (0, 1)),
            CHECK (status IN ('open', 'closed')),
            CHECK (expiresAt IS NULL OR expiresAt > 0),
            CHECK (createdAt > 0),
            CHECK (updatedAt > 0),
            CHECK (closedAt IS NULL OR closedAt > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "polls", schema);
    TableUtils::verifyIndex(db, "pollsChatID", "polls", "(chatID)");
    TableUtils::verifyIndex(db, "pollsCreatorUserID", "polls", "(creatorUserID)");
    TableUtils::verifyIndex(db, "pollsStatus", "polls", "(status)");
}

} // namespace Tables::PollsTable
