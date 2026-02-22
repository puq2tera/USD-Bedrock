#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class CreateChat : public BedrockCommand {
public:
    CreateChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~CreateChat() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
