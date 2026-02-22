#include "ChatsTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::ChatsTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE chats (
            chatID INTEGER PRIMARY KEY AUTOINCREMENT,
            title TEXT NOT NULL,
            createdAt INTEGER NOT NULL,
            createdByUserID INTEGER NOT NULL,
            FOREIGN KEY (createdByUserID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (length(trim(title)) BETWEEN 1 AND 255),
            CHECK (createdAt > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "chats", schema);
    TableUtils::verifyIndex(db, "chatsCreatedByUserID", "chats", "(createdByUserID)");
}

} // namespace Tables::ChatsTable
