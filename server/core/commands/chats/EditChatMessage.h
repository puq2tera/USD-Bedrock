#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class EditChatMessage : public BedrockCommand {
public:
    EditChatMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~EditChatMessage() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
