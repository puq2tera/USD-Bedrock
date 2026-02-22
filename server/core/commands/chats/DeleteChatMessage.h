#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class DeleteChatMessage : public BedrockCommand {
public:
    DeleteChatMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~DeleteChatMessage() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
