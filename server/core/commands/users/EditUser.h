#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class EditUser : public BedrockCommand {
public:
    EditUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~EditUser() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};