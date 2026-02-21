#include "VotesTable.h"

#include "TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::VotesTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE votes (
            voteID INTEGER PRIMARY KEY AUTOINCREMENT,
            pollID INTEGER NOT NULL,
            optionID INTEGER NOT NULL,
            userID INTEGER NOT NULL,
            createdAt INTEGER NOT NULL,
            FOREIGN KEY (pollID) REFERENCES polls(pollID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (optionID) REFERENCES poll_options(optionID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            UNIQUE (pollID, userID)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "votes", schema);
    TableUtils::verifyIndex(db, "votesOptionID", "votes", "(optionID)");
    TableUtils::verifyIndex(db, "votesUserID", "votes", "(userID)");
}

} // namespace Tables::VotesTable
