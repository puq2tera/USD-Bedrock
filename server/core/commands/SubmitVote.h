#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class SubmitVote : public BedrockCommand {
public:
    SubmitVote(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~SubmitVote() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void validateRequest() const;
};
