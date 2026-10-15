#pragma once

#include "database_manager.h"

#include <QObject>
#include <QVariantMap>
#include <QString>

class TaskPositions : public QObject
{
    Q_OBJECT

  public:
    explicit TaskPositions(Database &database);

    Q_INVOKABLE QVariantMap loadPositions();

    Q_INVOKABLE void savePosition(const QString &taskId, double x, double y);

  private:
    Database &m_database;
};
