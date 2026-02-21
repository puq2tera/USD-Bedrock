#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class DeleteUser : public BedrockCommand {
public:
    DeleteUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~DeleteUser() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};