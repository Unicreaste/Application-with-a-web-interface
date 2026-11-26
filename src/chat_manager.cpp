#include "chat_manager.h"

#include <QDebug>
#include <QVariantMap>

#include <sqlite3.h>

ChatManager::ChatManager(Database &database, QObject *parent)
    : QObject(parent),
      m_database(database)
{
    sqlite3 *db = m_database.handle();

    if (!db) {
        qWarning() << "ChatManager: база данных недоступна";
        return;
    }

    const char *sql = R"(
        CREATE TABLE IF NOT EXISTS chat_messages (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            sender TEXT NOT NULL,
            message TEXT NOT NULL,
            sent_at TEXT NOT NULL DEFAULT CURRENT_TIMESTAMP
        );
    )";

    char *errorMessage = nullptr;

    const int result = sqlite3_exec(
        db,
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {
        qWarning() << "Не удалось создать таблицу chat_messages:"
                   << (errorMessage ? errorMessage : "Unknown error");

        sqlite3_free(errorMessage);
    } else {
        qDebug() << "ChatManager: таблица chat_messages готова";
    }
}

QVariantList ChatManager::loadMessages()
{
    QVariantList messages;

    sqlite3 *db = m_database.handle();

    if (!db) {
        qWarning() << "loadMessages: база данных недоступна";
        return messages;
    }

    const char *sql = R"(
        SELECT id, sender, message, sent_at
        FROM chat_messages
        ORDER BY id ASC;
    )";

    sqlite3_stmt *statement = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &statement,
            nullptr
        ) != SQLITE_OK) {
        qWarning() << "Ошибка загрузки сообщений:"
                   << sqlite3_errmsg(db);
        return messages;
    }

    while (sqlite3_step(statement) == SQLITE_ROW) {
        QVariantMap item;

        item.insert(
            QStringLiteral("id"),
            sqlite3_column_int64(statement, 0)
        );

        item.insert(
            QStringLiteral("sender"),
            QString::fromUtf8(
                reinterpret_cast<const char *>(
                    sqlite3_column_text(statement, 1)
                )
            )
        );

        item.insert(
            QStringLiteral("message"),
            QString::fromUtf8(
                reinterpret_cast<const char *>(
                    sqlite3_column_text(statement, 2)
                )
            )
        );

        item.insert(
            QStringLiteral("sent_at"),
            QString::fromUtf8(
                reinterpret_cast<const char *>(
                    sqlite3_column_text(statement, 3)
                )
            )
        );

        messages.append(item);
    }

    sqlite3_finalize(statement);

    return messages;
}

bool ChatManager::saveMessage(
    const QString &sender,
    const QString &message
)
{
    sqlite3 *db = m_database.handle();

    if (!db) {
        qWarning() << "saveMessage: база данных недоступна";
        return false;
    }

    if (sender.trimmed().isEmpty() || message.trimmed().isEmpty()) {
        return false;
    }

    const char *sql = R"(
        INSERT INTO chat_messages (sender, message)
        VALUES (?, ?);
    )";

    sqlite3_stmt *statement = nullptr;

    if (sqlite3_prepare_v2(
            db,
            sql,
            -1,
            &statement,
            nullptr
        ) != SQLITE_OK) {
        qWarning() << "Ошибка подготовки сохранения:"
                   << sqlite3_errmsg(db);
        return false;
    }

    const QByteArray senderUtf8 = sender.toUtf8();
    const QByteArray messageUtf8 = message.toUtf8();

    sqlite3_bind_text(
        statement,
        1,
        senderUtf8.constData(),
        senderUtf8.size(),
        SQLITE_TRANSIENT
    );

    sqlite3_bind_text(
        statement,
        2,
        messageUtf8.constData(),
        messageUtf8.size(),
        SQLITE_TRANSIENT
    );

    const int result = sqlite3_step(statement);

    sqlite3_finalize(statement);

    if (result != SQLITE_DONE) {
        qWarning() << "Ошибка сохранения сообщения:"
                   << sqlite3_errmsg(db);
        return false;
    }

    qDebug() << "Сообщение сохранено";
    return true;
}

bool ChatManager::clearMessages()
{
    sqlite3 *db = m_database.handle();

    if (!db) {
        qWarning() << "clearMessages: база данных недоступна";
        return false;
    }

    const char *sql = "DELETE FROM chat_messages;";

    char *errorMessage = nullptr;

    const int result = sqlite3_exec(
        db,
        sql,
        nullptr,
        nullptr,
        &errorMessage
    );

    if (result != SQLITE_OK) {
        qWarning() << "Ошибка очистки чата:"
                   << (errorMessage ? errorMessage : "Unknown error");

        sqlite3_free(errorMessage);
        return false;
    }

    qDebug() << "История чата очищена";
    return true;
}