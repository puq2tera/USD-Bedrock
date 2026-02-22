#include "MessagesTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::MessagesTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE messages (
            messageID INTEGER PRIMARY KEY AUTOINCREMENT,
            chatID INTEGER NOT NULL,
            userID INTEGER NOT NULL,
            body TEXT NOT NULL,
            createdAt INTEGER NOT NULL,
            updatedAt INTEGER NOT NULL,
            FOREIGN KEY (chatID) REFERENCES chats(chatID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (length(trim(body)) BETWEEN 1 AND 1048576),
            CHECK (createdAt > 0),
            CHECK (updatedAt >= createdAt)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "messages", schema);
    TableUtils::verifyIndex(db, "messagesChatIDMessageID", "messages", "(chatID, messageID DESC)");
    TableUtils::verifyIndex(db, "messagesUserID", "messages", "(userID)");
}

} // namespace Tables::MessagesTable
