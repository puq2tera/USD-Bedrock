#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class LookupUserByEmail : public BedrockCommand {
public:
    LookupUserByEmail(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~LookupUserByEmail() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
