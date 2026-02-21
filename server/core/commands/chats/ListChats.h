#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class ListChats : public BedrockCommand {
public:
    ListChats(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~ListChats() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
