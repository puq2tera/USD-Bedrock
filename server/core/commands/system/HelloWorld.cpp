#include "HelloWorld.h"

#include "../../Core.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

namespace {

struct HelloWorldRequestModel {
    string name;

    static HelloWorldRequestModel bind(const SData& request) {
        const optional<string> providedName = RequestBinding::optionalString(request, "name", 1, BedrockPlugin::MAX_SIZE_SMALL);
        return {providedName ? *providedName : "World"};
    }
};

struct HelloWorldResponseModel {
    string message;
    string from;
    string timestamp;
    string pluginName;
    string pluginVersion;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "message", message);
        ResponseBinding::setString(response, "from", from);
        ResponseBinding::setString(response, "timestamp", timestamp);
        ResponseBinding::setString(response, "plugin_name", pluginName);
        ResponseBinding::setString(response, "plugin_version", pluginVersion);
    }
};

} // namespace

// Static member definitions
const string HelloWorld::_name = "HelloWorld";
const string HelloWorld::_description = "A simple hello world command for the Core plugin";

HelloWorld::HelloWorld(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
    // Initialize the command
}

HelloWorld::~HelloWorld() = default;

bool HelloWorld::peek(SQLite& db) {
    // This command doesn't need to peek at the database
    (void)db; // Unused
    return false;
}

void HelloWorld::process(SQLite& db) {
    (void)db; // Unused

    const HelloWorldRequestModel input = HelloWorldRequestModel::bind(request);

    const HelloWorldResponseModel output = {
        "Hello, " + input.name + "!",
        "Bedrock Core Plugin",
        SToStr(STimeNow()),
        _plugin->getName(),
        static_cast<BedrockPlugin_Core*>(_plugin)->getVersion(),
    };
    output.writeTo(response);

    SINFO("HelloWorld command executed for: " << input.name);
}

string HelloWorld::serializeData() const {
    // HelloWorld doesn't need to serialize any data
    return "";
}

void HelloWorld::deserializeData(const string& data) {
    // HelloWorld doesn't need to deserialize any data
    (void)data; // Suppress unused parameter warning
}
