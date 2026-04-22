#include "LookupUsers.h"

#include "../../Core.h"
#include "../CommandError.h"
#include "../RequestBinding.h"
#include "../ResponseBinding.h"

#include <fmt/format.h>
#include <libstuff/libstuff.h>

#include <map>
#include <set>

namespace {

struct LookupUsersRequestModel {
    list<int64_t> userIDs;

    static LookupUsersRequestModel bind(const SData& request) {
        const list<string> rawUserIDs = RequestBinding::requireJSONArray(request, "userIDs", 1, 500);

        list<int64_t> userIDs;
        set<int64_t> seenUserIDs;
        for (const string& rawUserID : rawUserIDs) {
            const int64_t userID = RequestBinding::parseInt64Strict(rawUserID, "userIDs");
            if (userID <= 0) {
                RequestBinding::throwInvalid("userIDs");
            }

            if (seenUserIDs.insert(userID).second) {
                userIDs.push_back(userID);
            }
        }

        return {userIDs};
    }
};

struct LookupUsersResponseModel {
    list<string> users;

    void writeTo(SData& response) const {
        ResponseBinding::setSize(response, "resultCount", users.size());
        ResponseBinding::setJSONArray(response, "users", users);
    }
};

} // namespace

LookupUsers::LookupUsers(SQLiteCommand&& baseCommand, BedrockPlugin_Core* plugin)
    : BedrockCommand(std::move(baseCommand), plugin) {
}

bool LookupUsers::peek(SQLite& db) {
    buildResponse(db);
    return true;
}

void LookupUsers::process(SQLite& db) {
    buildResponse(db);
}

void LookupUsers::buildResponse(SQLite& db) {
    const LookupUsersRequestModel input = LookupUsersRequestModel::bind(request);

    string userIDPredicate;
    bool isFirst = true;
    for (const int64_t userID : input.userIDs) {
        if (!isFirst) {
            userIDPredicate += ",";
        }
        isFirst = false;
        userIDPredicate += SToStr(userID);
    }

    SQResult result;
    const string query = fmt::format(
        "SELECT userID, firstName, lastName, displayName "
        "FROM users WHERE userID IN ({}) ORDER BY userID ASC;",
        userIDPredicate
    );
    if (!db.read(query, result)) {
        CommandError::upstreamFailure(
            db,
            "Failed to lookup users",
            "LOOKUP_USERS_READ_FAILED",
            {{"command", "LookupUsers"}}
        );
    }

    map<int64_t, STable> usersByID;
    for (const SQResultRow& row : result) {
        if (row.size() < 4) {
            continue;
        }

        STable user;
        user["userID"] = row[0];
        user["firstName"] = row[1];
        user["lastName"] = row[2];
        user["displayName"] = row[3];
        usersByID[SToInt64(row[0])] = user;
    }

    list<string> users;
    for (const int64_t userID : input.userIDs) {
        auto it = usersByID.find(userID);
        if (it == usersByID.end()) {
            continue;
        }

        users.push_back(SComposeJSONObject(it->second));
    }

    const LookupUsersResponseModel output = {users};
    output.writeTo(response);
}
