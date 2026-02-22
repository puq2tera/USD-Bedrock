#include "ChatMembersTable.h"

#include "../TableUtils.h"

#include <libstuff/libstuff.h>

namespace Tables::ChatMembersTable {

void verify(SQLite& db) {
    const string schema = R"(
        CREATE TABLE chat_members (
            chatID INTEGER NOT NULL,
            userID INTEGER NOT NULL,
            role TEXT NOT NULL,
            joinedAt INTEGER NOT NULL,
            PRIMARY KEY (chatID, userID),
            FOREIGN KEY (chatID) REFERENCES chats(chatID) ON DELETE CASCADE ON UPDATE CASCADE,
            FOREIGN KEY (userID) REFERENCES users(userID) ON DELETE CASCADE ON UPDATE CASCADE,
            CHECK (role IN ('owner', 'member')),
            CHECK (joinedAt > 0)
        )
    )";

    TableUtils::verifyTableOrRecreate(db, "chat_members", schema);
    TableUtils::verifyIndex(db, "chatMembersUserID", "chat_members", "(userID)");
}

} // namespace Tables::ChatMembersTable
