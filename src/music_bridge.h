#pragma once

#include <QObject>
#include <QString>

class MusicBridge : public QObject
{
    Q_OBJECT

  public:
    explicit MusicBridge(const QString &musicDirectory, QObject *parent = nullptr);

    Q_INVOKABLE QString readFileBase64(const QString &relativePath) const;

  private:
    QString m_musicDirectory;
};
