#include "LoginUser.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../ResponseBinding.h"
#include "UserAuth.h"
#include "UserValidation.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

namespace {

struct LoginUserRequestModel {
    string email;
    string password;

    static LoginUserRequestModel bind(const SData& request) {
        return {
            UserValidation::requireEmail(request, "email"),
            UserAuth::requirePassword(request, "password"),
        };
    }
};

struct LoginUserResponseModel {
    string userID;
    string email;
    string firstName;
    string lastName;
    string displayName;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "userID", userID);
        ResponseBinding::setString(response, "email", email);
        ResponseBinding::setString(response, "firstName", firstName);
        ResponseBinding::setString(response, "lastName", lastName);
        ResponseBinding::setString(response, "displayName", displayName);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

LoginUser::LoginUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool LoginUser::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void LoginUser::process(SQLite& db) {
    buildResponse(db);
}

void LoginUser::buildResponse(SQLite& db) {
    const LoginUserRequestModel input = LoginUserRequestModel::bind(request);

    SQResult result;
    const string query = fmt::format(
        "SELECT userID, email, passwordHash, firstName, lastName, displayName, createdAt "
        "FROM users WHERE email = {} LIMIT 1;",
        SQ(input.email)
    );
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch user login credentials",
            "LOGIN_USER_READ_FAILED",
            {{"command", "LoginUser"}, {"email", input.email}}
        );
    }

    if (result.empty() || result[0].size() < 7) {
        CommandError::throwError(
            401,
            "Invalid email or password",
            "LOGIN_USER_INVALID_CREDENTIALS",
            {{"command", "LoginUser"}, {"email", input.email}}
        );
    }

    const string& passwordHash = result[0][2];
    if (!UserAuth::verifyPassword(input.password, passwordHash)) {
        CommandError::throwError(
            401,
            "Invalid email or password",
            "LOGIN_USER_INVALID_CREDENTIALS",
            {{"command", "LoginUser"}, {"email", input.email}}
        );
    }

    const LoginUserResponseModel output = {
        result[0][0],
        result[0][1],
        result[0][3],
        result[0][4],
        result[0][5],
        result[0][6],
    };
    output.writeTo(response);
}
