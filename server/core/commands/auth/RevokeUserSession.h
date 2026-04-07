#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class RevokeUserSession : public BedrockCommand {
public:
    RevokeUserSession(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~RevokeUserSession() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
