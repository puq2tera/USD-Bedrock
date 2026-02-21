#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetPollParticipation : public BedrockCommand {
public:
    GetPollParticipation(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetPollParticipation() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
