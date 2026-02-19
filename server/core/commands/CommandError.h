#pragma once

#include <libstuff/libstuff.h>
#include <sqlitecluster/SQLite.h>

namespace CommandError {

inline string _methodLine(int statusCode, const string& message) {
    return SToStr(statusCode) + " " + message;
}

inline string _body(const string& message, const string& errorCode) {
    STable payload;
    payload["error"] = message;
    payload["errorCode"] = errorCode;
    return SComposeJSONObject(payload);
}

[[noreturn]] inline void throwError(int statusCode,
                                    const string& message,
                                    const string& errorCode,
                                    const STable& details = {}) {
    STable headers = details;
    if (!errorCode.empty()) {
        headers["errorCode"] = errorCode;
    }
    STHROW(_methodLine(statusCode, message), headers, _body(message, errorCode));
}

[[noreturn]] inline void throwCriticalError(int statusCode,
                                            const string& message,
                                            const string& errorCode,
                                            const STable& details = {}) {
    STable headers = details;
    if (!errorCode.empty()) {
        headers["errorCode"] = errorCode;
    }
    STHROW_STACK(_methodLine(statusCode, message), headers, _body(message, errorCode));
}

[[noreturn]] inline void badRequest(const string& message,
                                    const string& errorCode,
                                    const STable& details = {}) {
    throwError(400, message, errorCode, details);
}

[[noreturn]] inline void notFound(const string& message,
                                  const string& errorCode,
                                  const STable& details = {}) {
    throwError(404, message, errorCode, details);
}

[[noreturn]] inline void conflict(const string& message,
                                  const string& errorCode,
                                  const STable& details = {}) {
    throwError(409, message, errorCode, details);
}

[[noreturn]] inline void upstreamFailure(SQLite& db,
                                         const string& message,
                                         const string& errorCode,
                                         const STable& details = {}) {
    STable mergedDetails = details;
    mergedDetails["sqliteError"] = db.getLastError();
    throwCriticalError(502, message, errorCode, mergedDetails);
}

} // namespace CommandError
