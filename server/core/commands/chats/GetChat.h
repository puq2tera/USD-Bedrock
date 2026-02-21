#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetChat : public BedrockCommand {
public:
    GetChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetChat() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
