
#include "filewatcher.h"

#include <QFileInfo>
#include <QDebug>
#include <utility>

FileWatcher::FileWatcher(const QStringList &paths, std::function<void()> onChanged, QObject *parent)
    : QObject(parent), m_paths(paths), m_onChanged(std::move(onChanged))
{
    // Приводим пути к абсолютным.
    for (QString &path : m_paths) {
        path = QFileInfo(path).absoluteFilePath();
    }

    // Объединяем несколько быстрых изменений в одно событие.
    m_debounceTimer.setSingleShot(true);
    m_debounceTimer.setInterval(250);

    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
        qDebug() << "File changed:" << path;
        m_debounceTimer.start();
    });

    connect(&m_debounceTimer, &QTimer::timeout, this, [this]() {
        // Восстанавливаем наблюдение после перезаписи файла.
        addWatchPaths();

        if (m_onChanged) {
            m_onChanged();
        }
    });

    addWatchPaths();
}

void
FileWatcher::addWatchPaths()
{
    const QStringList watchedFiles = m_watcher.files();

    for (const QString &path : m_paths) {
        if (QFileInfo::exists(path) && !watchedFiles.contains(path)) {

            if (m_watcher.addPath(path)) {
                qDebug() << "Watching:" << path;
            }
        }
    }
}
