#include "VotesTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::VotesTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE votes (
            voteID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            optionID INTEGER NOT NULL,
            userID INTEGER NOT NULL,
            rank INTEGER,
            createdAt INTEGER NOT NULL,
            updatedAt INTEGER NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (optionID) REFERENCES poll_options(optionID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (rank IS NULL OR rank > 0),
            CHECK (createdAt > 0),
            CHECK (updatedAt > 0),
            UNIQUE (pollID, userID, optionID)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "votes", schema);
    TableUtils::verifyIndex(db, "votesPollUser", "votes", "(pollID, userID)");
    TableUtils::verifyIndex(db, "votesOptionID", "votes", "(optionID)");
    TableUtils::verifyIndex(db, "votesUserID", "votes", "(userID)");
}

} // namespace Tables::VotesTable
