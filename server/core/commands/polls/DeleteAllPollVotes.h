#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class DeleteAllPollVotes : public BedrockCommand {
public:
    DeleteAllPollVotes(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~DeleteAllPollVotes() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
