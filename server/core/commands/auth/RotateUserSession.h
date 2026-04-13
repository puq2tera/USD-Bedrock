#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class RotateUserSession : public BedrockCommand {
public:
    RotateUserSession(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~RotateUserSession() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
