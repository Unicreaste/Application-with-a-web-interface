    #include "database_manager.h"
    #include <iostream>
    Database::Database(const std::string &path)
    {
        int rc = sqlite3_open(path.c_str(), &db);
        if (rc != SQLITE_OK) {
            std::cerr << "Database open error: " << (db ? sqlite3_errmsg(db) : "Unknown error") << '\n';

            if (db) {
                sqlite3_close(db);
                db = nullptr;
            }

            return;
        }

        sqlite3_busy_timeout(db, 5000);
    }
    Database::~Database()
    {
        if (db) {
            sqlite3_close(db);
        }
    }
    bool
    Database::isOpen() const
    {
        return db != nullptr;
    }
    sqlite3 *
    Database::handle() const
    {
        return db;
    }
