
#include "task_positions.h"

#include <QDebug>
#include <QByteArray>
#include <QVariantList>

#include <sqlite3.h>

// ==========================================
// КОНСТРУКТОР
// ==========================================

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

        CREATE TABLE IF NOT EXISTS deleted_tasks (
            task_id TEXT PRIMARY KEY
        );
    )";

    char *error = nullptr;

    int rc = sqlite3_exec(m_database.handle(), sql, nullptr, nullptr, &error);

    if (rc != SQLITE_OK) {
        qWarning() << "Create tables error:" << (error ? error : "Unknown error");

        sqlite3_free(error);
    }
}

// ==========================================
// ЗАГРУЗКА ПОЗИЦИЙ И УДАЛЁННЫХ БЛОКОВ
// ==========================================

QVariantMap
TaskPositions::loadPositions()
{
    QVariantMap positions;

    if (!m_database.isOpen()) {
        return positions;
    }

    // Загружаем координаты блоков
    const char *sql = "SELECT task_id, pos_x, pos_y FROM task_positions";

    sqlite3_stmt *stmt = nullptr;

    int rc = sqlite3_prepare_v2(m_database.handle(), sql, -1, &stmt, nullptr);

    if (rc != SQLITE_OK) {
        qWarning() << "Load positions error:" << sqlite3_errmsg(m_database.handle());

        return positions;
    }

    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {

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

    if (rc != SQLITE_DONE) {
        qWarning() << "Read positions error:" << sqlite3_errmsg(m_database.handle());
    }

    sqlite3_finalize(stmt);

    // --------------------------------------
    // Загружаем список удалённых блоков
    // --------------------------------------

    const char *deletedSql = "SELECT task_id FROM deleted_tasks";

    sqlite3_stmt *deletedStmt = nullptr;

    rc = sqlite3_prepare_v2(m_database.handle(), deletedSql, -1, &deletedStmt, nullptr);

    QVariantList deletedIds;

    if (rc != SQLITE_OK) {

        qWarning() << "Load deleted tasks error:" << sqlite3_errmsg(m_database.handle());

    } else {

        while ((rc = sqlite3_step(deletedStmt)) == SQLITE_ROW) {

            const auto *rawId = sqlite3_column_text(deletedStmt, 0);

            if (!rawId) {
                continue;
            }

            deletedIds.append(QString::fromUtf8(reinterpret_cast<const char *>(rawId)));
        }

        if (rc != SQLITE_DONE) {
            qWarning() << "Read deleted tasks error:" << sqlite3_errmsg(m_database.handle());
        }

        sqlite3_finalize(deletedStmt);
    }

    // Передаём список удалённых ID в JavaScript
    positions.insert("__deletedTasks", deletedIds);

    return positions;
}

// ==========================================
// СОХРАНЕНИЕ ПОЗИЦИИ БЛОКА
// ==========================================

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

// ==========================================
// УДАЛЕНИЕ БЛОКА
// ==========================================

void
TaskPositions::deletePosition(const QString &taskId)
{
    if (!m_database.isOpen() || taskId.isEmpty()) {
        return;
    }

    sqlite3 *db = m_database.handle();

    const QByteArray id = taskId.toUtf8();

    // Начинаем транзакцию
    char *error = nullptr;

    int rc = sqlite3_exec(db, "BEGIN IMMEDIATE TRANSACTION;", nullptr, nullptr, &error);

    if (rc != SQLITE_OK) {

        qWarning() << "Begin delete transaction error:" << (error ? error : "Unknown error");

        sqlite3_free(error);

        return;
    }

    // --------------------------------------
    // Удаляем сохранённые координаты
    // --------------------------------------

    const char *deleteSql = "DELETE FROM task_positions WHERE task_id = ?";

    sqlite3_stmt *deleteStmt = nullptr;

    rc = sqlite3_prepare_v2(db, deleteSql, -1, &deleteStmt, nullptr);

    if (rc != SQLITE_OK) {

        qWarning() << "Prepare delete error:" << sqlite3_errmsg(db);

        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

        return;
    }

    sqlite3_bind_text(deleteStmt, 1, id.constData(), id.size(), SQLITE_TRANSIENT);

    rc = sqlite3_step(deleteStmt);

    sqlite3_finalize(deleteStmt);

    if (rc != SQLITE_DONE) {

        qWarning() << "Delete position error:" << sqlite3_errmsg(db);

        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

        return;
    }

    // --------------------------------------
    // Запоминаем удалённый блок
    // --------------------------------------

    const char *markSql = R"(
        INSERT OR IGNORE INTO deleted_tasks (task_id)
        VALUES (?);
    )";

    sqlite3_stmt *markStmt = nullptr;

    rc = sqlite3_prepare_v2(db, markSql, -1, &markStmt, nullptr);

    if (rc != SQLITE_OK) {

        qWarning() << "Prepare deleted task error:" << sqlite3_errmsg(db);

        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

        return;
    }

    sqlite3_bind_text(markStmt, 1, id.constData(), id.size(), SQLITE_TRANSIENT);

    rc = sqlite3_step(markStmt);

    sqlite3_finalize(markStmt);

    if (rc != SQLITE_DONE) {

        qWarning() << "Mark deleted task error:" << sqlite3_errmsg(db);

        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

        return;
    }

    // --------------------------------------
    // Завершаем транзакцию
    // --------------------------------------

    rc = sqlite3_exec(db, "COMMIT;", nullptr, nullptr, &error);

    if (rc != SQLITE_OK) {

        qWarning() << "Commit delete error:" << (error ? error : "Unknown error");

        sqlite3_free(error);

        sqlite3_exec(db, "ROLLBACK;", nullptr, nullptr, nullptr);

        return;
    }

    qDebug() << "Block deleted:" << taskId;
}
