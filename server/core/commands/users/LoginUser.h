#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class LoginUser : public BedrockCommand {
public:
    LoginUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~LoginUser() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
};
