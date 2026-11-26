#include "task_positions.h"

#include <QDebug>
#include <QByteArray>
#include <sqlite3.h>

TaskPositions::TaskPositions(Database &database) : m_database(database)
{
    if (!m_database.isOpen()) {
        return;
    }

    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS task_positions (
            task_id TEXT PRIMARY KEY,
            pos_x REAL NOT NULL,
            pos_y REAL NOT NULL
        );
    )";

    char *error = nullptr;

    int rc = sqlite3_exec(m_database.handle(), sql, nullptr, nullptr, &error);

    if (rc != SQLITE_OK) {
        qWarning() << "Create task_positions error:" << (error ? error : "Unknown error");

        sqlite3_free(error);
    }
}

QVariantMap
TaskPositions::loadPositions()
{
    QVariantMap positions;

    if (!m_database.isOpen()) {
        return positions;
    }

    const char *sql = "SELECT task_id, pos_x, pos_y FROM task_positions";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_database.handle(), sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        qWarning() << "Load positions error:" << sqlite3_errmsg(m_database.handle());
        return positions;
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        const auto *rawId = sqlite3_column_text(stmt, 0);

        if (!rawId) {
            continue;
        }

        const QString id = QString::fromUtf8(reinterpret_cast<const char *>(rawId));

        QVariantMap position;
        position["left"] = sqlite3_column_double(stmt, 1);
        position["top"] = sqlite3_column_double(stmt, 2);

        positions.insert(id, position);
    }

    sqlite3_finalize(stmt);

    return positions;
}

void
TaskPositions::savePosition(const QString &taskId, double x, double y)
{
    if (!m_database.isOpen() || taskId.isEmpty()) {
        return;
    }

    const char *sql = R"(
        INSERT INTO task_positions (task_id, pos_x, pos_y)
        VALUES (?, ?, ?)
        ON CONFLICT(task_id)
        DO UPDATE SET
            pos_x = excluded.pos_x,
            pos_y = excluded.pos_y;
    )";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_database.handle(), sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        qWarning() << "Prepare position save error:" << sqlite3_errmsg(m_database.handle());
        return;
    }

    const QByteArray id = taskId.toUtf8();

    sqlite3_bind_text(stmt, 1, id.constData(), id.size(), SQLITE_TRANSIENT);
    sqlite3_bind_double(stmt, 2, x);
    sqlite3_bind_double(stmt, 3, y);

    rc = sqlite3_step(stmt);

    if (rc != SQLITE_DONE) {
        qWarning() << "Save position error:" << sqlite3_errmsg(m_database.handle());
    }

    sqlite3_finalize(stmt);
}
