#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class SubmitPollVotes : public BedrockCommand {
public:
    SubmitPollVotes(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~SubmitPollVotes() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
