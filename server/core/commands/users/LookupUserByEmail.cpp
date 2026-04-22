#include "LookupUserByEmail.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "UserValidation.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct LookupUserByEmailRequestModel {
    string email;

    static LookupUserByEmailRequestModel bind(const SData& request) {
        return {UserValidation::requireEmail(request, "email")};
    }
};

struct LookupUserByEmailResponseModel {
    string userID;
    string firstName;
    string lastName;
    string displayName;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "userID", userID);
        ResponseBinding::setString(response, "firstName", firstName);
        ResponseBinding::setString(response, "lastName", lastName);
        ResponseBinding::setString(response, "displayName", displayName);
    }
};

} // namespace

LookupUserByEmail::LookupUserByEmail(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool LookupUserByEmail::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void LookupUserByEmail::process(SQLite& db) {
    buildResponse(db);
}

void LookupUserByEmail::buildResponse(SQLite& db) {
    const LookupUserByEmailRequestModel input = LookupUserByEmailRequestModel::bind(request);

    SQResult result;
    const string query = fmt::format(
        "SELECT userID, firstName, lastName, displayName FROM users WHERE email = {} LIMIT 1;",
        SQ(input.email)
    );
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to lookup user by email",
            "LOOKUP_USER_BY_EMAIL_READ_FAILED",
            {{"command", "LookupUserByEmail"}}
        );
    }

    if (result.empty()) {
        CommandError::notFound(
            "User not found",
            "LOOKUP_USER_BY_EMAIL_NOT_FOUND",
            {{"command", "LookupUserByEmail"}}
        );
    }

    const LookupUserByEmailResponseModel output = {
        result[0][0],
        result[0][1],
        result[0][2],
        result[0][3],
    };
    output.writeTo(response);
}
