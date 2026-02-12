#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetPoll : public BedrockCommand {
public:
    GetPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetPoll() override = default;

    // Read-only command — all work happens in peek()
    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
