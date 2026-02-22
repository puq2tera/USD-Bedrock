#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class CreateChatMessage : public BedrockCommand {
public:
    CreateChatMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~CreateChatMessage() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
