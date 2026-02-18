#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetMessages : public BedrockCommand {
public:
    GetMessages(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetMessages() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};

