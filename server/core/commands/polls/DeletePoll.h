#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class DeletePoll : public BedrockCommand {
public:
    DeletePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~DeletePoll() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
