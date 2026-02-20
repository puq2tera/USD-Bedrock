#include "MessagesTable.h"

#include "TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::MessagesTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE messages (
            messageID INTEGER PRIMARY KEY AUTOINCREMENT,
            userID INTEGER NOT NULL,
            name TEXT NOT NULL,
            message TEXT NOT NULL,
            createdAt INTEGER NOT NULL,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "messages", schema);
    TableUtils::verifyIndex(db, "messagesCreatedAt", "messages", "(createdAt DESC)");
    TableUtils::verifyIndex(db, "messagesUserID", "messages", "(userID)");
}

} // namespace Tables::MessagesTable
