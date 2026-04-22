#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class LookupUsers : public BedrockCommand {
public:
    LookupUsers(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~LookupUsers() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
