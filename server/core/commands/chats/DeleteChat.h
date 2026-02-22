#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class DeleteChat : public BedrockCommand {
public:
    DeleteChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~DeleteChat() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
