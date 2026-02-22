#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class SubmitPollTextResponse : public BedrockCommand {
public:
    SubmitPollTextResponse(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~SubmitPollTextResponse() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
