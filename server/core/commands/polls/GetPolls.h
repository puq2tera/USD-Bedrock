#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetPolls : public BedrockCommand {
public:
    GetPolls(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetPolls() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
