#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class AddChatMember : public BedrockCommand {
public:
    AddChatMember(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~AddChatMember() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
