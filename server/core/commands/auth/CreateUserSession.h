#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class CreateUserSession : public BedrockCommand {
public:
    CreateUserSession(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~CreateUserSession() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
