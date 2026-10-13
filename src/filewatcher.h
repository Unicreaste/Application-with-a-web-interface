
#pragma once

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QStringList>
#include <functional>

class FileWatcher : public QObject
{
  public:
    explicit FileWatcher(const QStringList &paths, std::function<void()> onChanged, QObject *parent = nullptr);

  private:
    QFileSystemWatcher m_watcher;
    QTimer m_debounceTimer;
    QStringList m_paths;
    std::function<void()> m_onChanged;

    void addWatchPaths();
};
