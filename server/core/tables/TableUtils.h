#pragma once

#include <libstuff/libstuff.h>

class SQLite;

namespace Tables::TableUtils {

void verifyTableOrRecreate(SQLite& db, const string& tableName, const string& schema);
void verifyIndex(SQLite& db,
                 const string& indexName,
                 const string& tableName,
                 const string& indexedColumns,
                 bool unique = false);

} // namespace Tables::TableUtils
