#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class CreateMessage : public BedrockCommand {
public:
    CreateMessage(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~CreateMessage() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void validateRequest() const;
};

