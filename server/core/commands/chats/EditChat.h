#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class EditChat : public BedrockCommand {
public:
    EditChat(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~EditChat() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
