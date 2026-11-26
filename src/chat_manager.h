#pragma once

#include "database_manager.h"

#include <QObject>
#include <QVariantList>
#include <QString>

class ChatManager : public QObject
{
    Q_OBJECT

  public:
    explicit ChatManager(Database &database, QObject *parent = nullptr);

    Q_INVOKABLE QVariantList loadMessages();
    Q_INVOKABLE bool saveMessage(const QString &sender, const QString &message);

    Q_INVOKABLE bool clearMessages();

  private:
    Database &m_database;
};
