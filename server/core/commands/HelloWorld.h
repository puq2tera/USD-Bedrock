#pragma once
#include <libstuff/libstuff.h>
#include <BedrockCommand.h>

// Forward declaration
class BedrockPlugin_Core;

class HelloWorld : public BedrockCommand {
public:
    // Constructor
    HelloWorld(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin);

    // Destructor
    ~HelloWorld() override;

    // Command execution - override the base class methods
    bool peek(SQLite& db) override;
    void process(SQLite& db) override;

    // Serialize/deserialize command data (required by BedrockCommand)
    string serializeData() const override;
    void deserializeData(const string& data) override;

private:
    static const string _name;
    static const string _description;
};
