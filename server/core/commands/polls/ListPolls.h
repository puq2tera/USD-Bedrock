#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class ListPolls : public BedrockCommand {
public:
    ListPolls(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~ListPolls() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
