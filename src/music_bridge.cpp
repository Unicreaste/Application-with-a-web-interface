#include "music_bridge.h"

#include <QFile>
#include <QFileInfo>
#include <QDir>

MusicBridge::MusicBridge(const QString &musicDirectory, QObject *parent)
    : QObject(parent), m_musicDirectory(QFileInfo(musicDirectory).canonicalFilePath())
{
}

QString
MusicBridge::readFileBase64(const QString &relativePath) const
{
    if (m_musicDirectory.isEmpty() || QDir::isAbsolutePath(relativePath)) {
        return {};
    }

    QDir directory(m_musicDirectory);

    const QString path = QFileInfo(directory.filePath(relativePath)).canonicalFilePath();

    if (path.isEmpty() || !path.startsWith(m_musicDirectory + QDir::separator())) {
        return {};
    }

    QFile file(path);

    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    return QString::fromLatin1(file.readAll().toBase64());
}
