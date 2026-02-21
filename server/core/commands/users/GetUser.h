#pragma once

#include <BedrockCommand.h>

class BedrockPlugin_Core;

class GetUser : public BedrockCommand {
public:
    GetUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);
    ~GetUser() override = default;

    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

private:
    void buildResponse(SQLite& db);
<<<<<<< HEAD
};
=======
};
>>>>>>> origin/main
