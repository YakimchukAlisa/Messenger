#ifndef DATABASE_H
#define DATABASE_H

#include <sqlite3.h>

#include <algorithm>
#include <ctime>
#include <iostream>
#include <mutex>
#include <string>
#include <vector>

class Database {
private:

    sqlite3* db = nullptr;

    std::mutex dbMutex;

    static Database* instance;

    Database() {
        open();
        createTables();
    }

    void open() {

        int rc = sqlite3_open("messenger.db", &db);

        if (rc != SQLITE_OK) {

            std::cerr
                << "Can't open database: "
                << sqlite3_errmsg(db)
                << std::endl;
        }
        else {

            std::cout
                << "Database opened successfully"
                << std::endl;
        }
    }

    void createTables() {

        const char* sqlMessages = R"(
            CREATE TABLE IF NOT EXISTS private_messages (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                from_user TEXT NOT NULL,
                to_user TEXT NOT NULL,
                message TEXT NOT NULL,
                timestamp INTEGER NOT NULL,
                delivered INTEGER DEFAULT 0
            );
        )";

        const char* sqlLogs = R"(
            CREATE TABLE IF NOT EXISTS logs (
                id INTEGER PRIMARY KEY AUTOINCREMENT,
                event_type TEXT NOT NULL,
                details TEXT,
                timestamp INTEGER NOT NULL
            );
        )";

        char* errMsg = nullptr;

        sqlite3_exec(
            db,
            sqlMessages,
            nullptr,
            nullptr,
            &errMsg
        );

        if (errMsg) {

            std::cerr
                << "SQL error: "
                << errMsg
                << std::endl;

            sqlite3_free(errMsg);
        }

        sqlite3_exec(
            db,
            sqlLogs,
            nullptr,
            nullptr,
            &errMsg
        );

        if (errMsg) {

            std::cerr
                << "SQL error: "
                << errMsg
                << std::endl;

            sqlite3_free(errMsg);
        }
    }

public:

    static Database& getInstance() {

        if (!instance) {
            instance = new Database();
        }

        return *instance;
    }

    void saveMessage(
        const std::string& from,
        const std::string& to,
        const std::string& message,
        time_t timestamp
    ) {

        std::lock_guard<std::mutex> lock(dbMutex);

        const char* sql =
            "INSERT INTO private_messages "
            "(from_user, to_user, message, timestamp, delivered) "
            "VALUES (?, ?, ?, ?, 0);";

        sqlite3_stmt* stmt = nullptr;

        int rc = sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        );

        if (rc != SQLITE_OK) {

            std::cerr
                << "Prepare failed: "
                << sqlite3_errmsg(db)
                << std::endl;

            return;
        }

        sqlite3_bind_text(
            stmt,
            1,
            from.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            2,
            to.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            3,
            message.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_int64(
            stmt,
            4,
            timestamp
        );

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {

            std::cerr
                << "Insert failed: "
                << sqlite3_errmsg(db)
                << std::endl;
        }

        sqlite3_finalize(stmt);
    }

    std::vector<std::string> getHistory(
        const std::string& user1,
        const std::string& user2,
        int limit = 50
    ) {

        std::lock_guard<std::mutex> lock(dbMutex);

        std::vector<std::string> history;

        const char* sql = R"(
            SELECT from_user, message, timestamp
            FROM private_messages
            WHERE
                (from_user = ? AND to_user = ?)
                OR
                (from_user = ? AND to_user = ?)
            ORDER BY timestamp DESC
            LIMIT ?;
        )";

        sqlite3_stmt* stmt = nullptr;

        int rc = sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        );

        if (rc != SQLITE_OK) {

            std::cerr
                << "Prepare failed: "
                << sqlite3_errmsg(db)
                << std::endl;

            return history;
        }

        sqlite3_bind_text(
            stmt,
            1,
            user1.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            2,
            user2.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            3,
            user2.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            4,
            user1.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_int(
            stmt,
            5,
            limit
        );

        while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

            const char* fromText =
                reinterpret_cast<const char*>(
                    sqlite3_column_text(stmt, 0)
                    );

            const char* msgText =
                reinterpret_cast<const char*>(
                    sqlite3_column_text(stmt, 1)
                    );

            time_t ts =
                sqlite3_column_int64(stmt, 2);

            std::string from =
                fromText ? fromText : "";

            std::string msg =
                msgText ? msgText : "";

            char timeStr[64];

            tm timeInfo{};

            localtime_r(&ts, &timeInfo);

            strftime(
                timeStr,
                sizeof(timeStr),
                "%H:%M:%S",
                &timeInfo
            );

            std::string line =
                "[" +
                std::string(timeStr) +
                "] " +
                from +
                ": " +
                msg;

            history.push_back(line);

            std::cout
                << "[DB HISTORY] "
                << line
                << std::endl;
        }

        sqlite3_finalize(stmt);

        std::reverse(
            history.begin(),
            history.end()
        );

        return history;
    }

    void saveLog(
        const std::string& eventType,
        const std::string& details,
        time_t timestamp
    ) {

        std::lock_guard<std::mutex> lock(dbMutex);

        const char* sql =
            "INSERT INTO logs "
            "(event_type, details, timestamp) "
            "VALUES (?, ?, ?);";

        sqlite3_stmt* stmt = nullptr;

        int rc = sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &stmt,
            nullptr
        );

        if (rc != SQLITE_OK) {

            std::cerr
                << "Prepare failed: "
                << sqlite3_errmsg(db)
                << std::endl;

            return;
        }

        sqlite3_bind_text(
            stmt,
            1,
            eventType.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_text(
            stmt,
            2,
            details.c_str(),
            -1,
            SQLITE_TRANSIENT
        );

        sqlite3_bind_int64(
            stmt,
            3,
            timestamp
        );

        rc = sqlite3_step(stmt);

        if (rc != SQLITE_DONE) {

            std::cerr
                << "Insert failed: "
                << sqlite3_errmsg(db)
                << std::endl;
        }

        sqlite3_finalize(stmt);
    }

    ~Database() {

        if (db) {

            sqlite3_close(db);
            db = nullptr;
        }
    }
};

inline Database* Database::instance = nullptr;

#endif