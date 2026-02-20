#include "EditUser.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"
#include "UserValidation.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>
#include <vector>

namespace {

struct EditUserRequestModel {
    int64_t userID;
    optional<string> email;
    optional<string> firstName;
    optional<string> lastName;

    static EditUserRequestModel bind(const SData& request) {
        const int64_t userID = RequestBinding::requirePositiveInt64(request, "userID");
        const optional<string> email = UserValidation::optionalEmail(request, "email");
        const optional<string> firstName = UserValidation::optionalName(request, "firstName");
        const optional<string> lastName = UserValidation::optionalName(request, "lastName");

        if (!email && !firstName && !lastName) {
            CommandError::badRequest(
                "Missing required parameter: email, firstName, or lastName",
                "EDIT_USER_MISSING_FIELDS",
                {{"command", "EditUser"}}
            );
        }

        return {userID, email, firstName, lastName};
    }
};

struct EditUserResponseModel {
    string userID;
    string email;
    string firstName;
    string lastName;
    string createdAt;
    string result;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "userID", userID);
        ResponseBinding::setString(response, "email", email);
        ResponseBinding::setString(response, "firstName", firstName);
        ResponseBinding::setString(response, "lastName", lastName);
        ResponseBinding::setString(response, "createdAt", createdAt);
        ResponseBinding::setString(response, "result", result);
    }
};

} // namespace

EditUser::EditUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool EditUser::peek(SQLite& db) {
    (void)db;
    (void)EditUserRequestModel::bind(request);
    return false;
}

void EditUser::process(SQLite& db) {
    const EditUserRequestModel input = EditUserRequestModel::bind(request);

    SQResult existingUserResult;
    const string existingUserQuery = fmt::format(
        "SELECT userID FROM users WHERE userID = {};",
        input.userID
    );
    if (!db.read(existingUserQuery, existingUserResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify user",
            "EDIT_USER_LOOKUP_FAILED",
            {{"command", "EditUser"}, {"userID", SToStr(input.userID)}}
        );
    }
    if (existingUserResult.empty()) {
        CommandError::notFound(
            "User not found",
            "EDIT_USER_NOT_FOUND",
            {{"command", "EditUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    if (input.email) {
        SQResult existingEmailResult;
        const string existingEmailQuery = fmt::format(
            "SELECT userID FROM users WHERE email = {} AND userID <> {} LIMIT 1;",
            SQ(*input.email), input.userID
        );
        if (!db.read(existingEmailQuery, existingEmailResult)) {
            CommandError::upstreamFailure(
                db,
                "Failed to verify user email uniqueness",
                "EDIT_USER_EMAIL_LOOKUP_FAILED",
                {{"command", "EditUser"}, {"userID", SToStr(input.userID)}, {"email", *input.email}}
            );
        }
        if (!existingEmailResult.empty()) {
            CommandError::conflict(
                "Email already in use",
                "EDIT_USER_EMAIL_CONFLICT",
                {{"command", "EditUser"}, {"userID", SToStr(input.userID)}, {"email", *input.email}}
            );
        }
    }

    vector<string> updateClauses;
    if (input.email) {
        updateClauses.emplace_back(fmt::format("email = {}", SQ(*input.email)));
    }
    if (input.firstName) {
        updateClauses.emplace_back(fmt::format("firstName = {}", SQ(*input.firstName)));
    }
    if (input.lastName) {
        updateClauses.emplace_back(fmt::format("lastName = {}", SQ(*input.lastName)));
    }

    string setClause;
    for (size_t i = 0; i < updateClauses.size(); i++) {
        if (i > 0) {
            setClause += ", ";
        }
        setClause += updateClauses[i];
    }

    const string updateQuery = fmt::format(
        "UPDATE users SET {} WHERE userID = {};",
        setClause, input.userID
    );
    if (!db.write(updateQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to update user",
            "EDIT_USER_UPDATE_FAILED",
            {{"command", "EditUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    SQResult updatedUserResult;
    const string updatedUserQuery = fmt::format(
        "SELECT userID, email, firstName, lastName, createdAt FROM users WHERE userID = {};",
        input.userID
    );
    if (!db.read(updatedUserQuery, updatedUserResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to fetch updated user",
            "EDIT_USER_FETCH_UPDATED_FAILED",
            {{"command", "EditUser"}, {"userID", SToStr(input.userID)}}
        );
    }
    if (updatedUserResult.empty()) {
        CommandError::notFound(
            "User not found",
            "EDIT_USER_NOT_FOUND_AFTER_UPDATE",
            {{"command", "EditUser"}, {"userID", SToStr(input.userID)}}
        );
    }

    const EditUserResponseModel output = {
        updatedUserResult[0][0],
        updatedUserResult[0][1],
        updatedUserResult[0][2],
        updatedUserResult[0][3],
        updatedUserResult[0][4],
        "updated",
    };
    output.writeTo(response);

    SINFO("Updated user " << output.userID);
}
