#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class RemoveChatMember : public BedrockCommand {
public:
    RemoveChatMember(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~RemoveChatMember() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
