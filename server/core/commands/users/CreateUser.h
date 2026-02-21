#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class CreateUser : public BedrockCommand {
public:
    CreateUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~CreateUser() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};