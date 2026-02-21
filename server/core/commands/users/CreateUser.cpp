#include "CreateUser.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../ResponseBinding.h"
#include "UserValidation.h"

#include <libstuff/libstuff.h>
#include <fmt/format.h>

namespace {

struct CreateUserRequestModel {
    string email;
    string firstName;
    string lastName;
<<<<<<< HEAD
    string displayName;

    static CreateUserRequestModel bind(const SData& request) {
        const string firstName = UserValidation::requireName(request, "firstName");
        const string lastName = UserValidation::requireName(request, "lastName");
        const optional<string> displayName = UserValidation::optionalDisplayName(request, "displayName");

        return {
            UserValidation::requireEmail(request, "email"),
            firstName,
            lastName,
            displayName.value_or(UserValidation::defaultDisplayName(firstName, lastName))
=======

    static CreateUserRequestModel bind(const SData& request) {
        return {
            UserValidation::requireEmail(request, "email"),
            UserValidation::requireName(request, "firstName"),
            UserValidation::requireName(request, "lastName")
>>>>>>> origin/main
        };
    }
};

struct CreateUserResponseModel {
    string userID;
    string email;
    string firstName;
    string lastName;
<<<<<<< HEAD
    string displayName;
=======
>>>>>>> origin/main
    string createdAt;

    void writeTo(SData& response) const {
        ResponseBinding::setString(response, "userID", userID);
        ResponseBinding::setString(response, "email", email);
        ResponseBinding::setString(response, "firstName", firstName);
        ResponseBinding::setString(response, "lastName", lastName);
<<<<<<< HEAD
        ResponseBinding::setString(response, "displayName", displayName);
=======
>>>>>>> origin/main
        ResponseBinding::setString(response, "createdAt", createdAt);
    }
};

} // namespace

CreateUser::CreateUser(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool CreateUser::peek(SQLite& db) {
    (void)db;
    (void)CreateUserRequestModel::bind(request);
    return false;
}

void CreateUser::process(SQLite& db) {
    const CreateUserRequestModel input = CreateUserRequestModel::bind(request);
    const string createdAt = SToStr(STimeNow());

    SQResult existingEmailResult;
    const string existingEmailQuery = fmt::format(
        "SELECT userID FROM users WHERE email = {} LIMIT 1;",
        SQ(input.email)
    );
    if (!db.read(existingEmailQuery, existingEmailResult)) {
        CommandError::upstreamFailure(
            db,
            "Failed to verify user email uniqueness",
            "CREATE_USER_EMAIL_LOOKUP_FAILED",
            {{"command", "CreateUser"}, {"email", input.email}}
        );
    }
    if (!existingEmailResult.empty()) {
        CommandError::conflict(
            "Email already in use",
            "CREATE_USER_EMAIL_CONFLICT",
            {{"command", "CreateUser"}, {"email", input.email}}
        );
    }

    const string insertUserQuery = fmt::format(
<<<<<<< HEAD
        "INSERT INTO users (email, firstName, lastName, displayName, createdAt) VALUES ({}, {}, {}, {}, {});",
        SQ(input.email), SQ(input.firstName), SQ(input.lastName), SQ(input.displayName), createdAt
=======
        "INSERT INTO users (email, firstName, lastName, createdAt) VALUES ({}, {}, {}, {});",
        SQ(input.email), SQ(input.firstName), SQ(input.lastName), createdAt
>>>>>>> origin/main
    );
    if (!db.write(insertUserQuery)) {
        CommandError::upstreamFailure(
            db,
            "Failed to insert user",
            "CREATE_USER_INSERT_FAILED",
            {{"command", "CreateUser"}, {"email", input.email}}
        );
    }

    SQResult idResult;
    if (!db.read("SELECT last_insert_rowid()", idResult) || idResult.empty() || idResult[0].empty()) {
        CommandError::upstreamFailure(
            db,
            "Failed to retrieve userID",
            "CREATE_USER_LAST_INSERT_ID_FAILED",
            {{"command", "CreateUser"}, {"email", input.email}}
        );
    }

    const CreateUserResponseModel output = {
        idResult[0][0],
        input.email,
        input.firstName,
        input.lastName,
<<<<<<< HEAD
        input.displayName,
=======
>>>>>>> origin/main
        createdAt,
    };
    output.writeTo(response);

    SINFO("Created user " << output.userID);
}
