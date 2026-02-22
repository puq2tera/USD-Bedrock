#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class DeletePollVotes : public BedrockCommand {
public:
    DeletePollVotes(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~DeletePollVotes() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
