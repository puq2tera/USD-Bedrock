#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetChatMessages : public BedrockCommand {
public:
    GetChatMessages(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetChatMessages() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
