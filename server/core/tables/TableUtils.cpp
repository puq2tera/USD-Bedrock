#include "TableUtils.h"

#include <sqlitecluster/SQLite.h>

namespace Tables::TableUtils {

void verifyTableOrRecreate(SQLite& db, const string& tableName, const string& schema) {
    bool created = false;
    while (!db.verifyTable(tableName, schema, created)) {
        SASSERT(db.write("PRAGMA foreign_keys = OFF;"));
        SASSERT(db.write("DROP TABLE IF EXISTS " + tableName + ";"));
        SASSERT(db.write("PRAGMA foreign_keys = ON;"));
        created = false;
    }
}

void verifyIndex(SQLite& db,
                 const string& indexName,
                 const string& tableName,
                 const string& indexedColumns,
                 bool unique) {
    db.verifyIndex(indexName, tableName, indexedColumns, unique, true);
}

} // namespace Tables::TableUtils
