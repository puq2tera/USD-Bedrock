#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class CreatePoll : public BedrockCommand {
public:
    CreatePoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~CreatePoll() override = default;

    // peek = read-only phase (runs on any node). We just validate here.
    bool peek(SQLite& db) override;

    // process = read-write phase (runs on leader). We do the INSERT here.
    void process(SQLite& db) override;
};
