#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class ListChatMembers : public BedrockCommand {
public:
    ListChatMembers(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~ListChatMembers() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
