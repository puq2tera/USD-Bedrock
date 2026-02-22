#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class EditChatMemberRole : public BedrockCommand {
public:
    EditChatMemberRole(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~EditChatMemberRole() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;
};
