#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class EditPoll : public BedrockCommand {
public:
    EditPoll(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~EditPoll() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
