#include "GetUser.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct GetUserRequestModel {
    int64_t userID;

    static GetUserRequestModel bind(const SData& request) {
        return {RequestBinding::requirePositiveInt64(request, "userID")};
    }
};

struct GetUserResponseModel {
    string userID;
    string email;
    string firstName;
    string lastName;
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "userID", userID);
        ResponseBinding::setString(response, "email", email);
        ResponseBinding::setString(response, "firstName", firstName);
        ResponseBinding::setString(response, "lastName", lastName);
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

GetUser::GetUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool GetUser::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void GetUser::process(SQLite& db) {
    buildResponse(db);
}

void GetUser::buildResponse(SQLite& db) {
    const GetUserRequestModel input = GetUserRequestModel::bind(request);

    SQResult result;
    const string query = fmt::format(
        "SELECT userID, email, firstName, lastName, createdAt FROM users WHERE userID = {};",
        input.userID
    );
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch user",
            "GET_USER_READ_FAILED",
            {{"command", "GetUser"}, {"userID", SToStr(input.userID)}}
        );
    }
    if (result.empty()) {
        CommandError::notFound(
            "User not found",
            "GET_USER_NOT_FOUND",
            {{"command", "GetUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const GetUserResponseModel output = {
        result[0][0],
        result[0][1],
        result[0][2],
        result[0][3],
        result[0][4],
    };
    output.writeTo(response);
}
