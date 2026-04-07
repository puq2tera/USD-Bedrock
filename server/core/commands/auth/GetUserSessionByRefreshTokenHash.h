#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetUserSessionByRefreshTokenHash : public BedrockCommand {
public:
    GetUserSessionByRefreshTokenHash(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetUserSessionByRefreshTokenHash() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
