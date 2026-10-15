
#pragma once

#include <QWidget>
#include <QString>
#include <QPoint>
#include <QRect>
#include <QList>

#include "resizehandle.h"
#include "database_manager.h"
#include "theme_manager.h"
#include <memory>

class QWebEngineView;
class TitleBar;
class FileWatcher;
class PageLoader;
class QResizeEvent;
class QMouseEvent;

class QWebChannel;
class TaskPositions;
class ChatManager;

class MainWindow : public QWidget
{
  public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

  protected:
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

  private:
    enum ResizeEdge
    {
        NoEdge = 0,
        LeftEdge = 1,
        RightEdge = 2,
        TopEdge = 4,
        BottomEdge = 8
    };

    std::unique_ptr<Database> m_database;
    std::unique_ptr<TaskPositions> m_taskPositions;

    std::unique_ptr<ChatManager> m_chatManager;
    std::unique_ptr<ThemeManager> m_themeManager;

    QWebChannel *m_webChannel = nullptr;

    int resizeEdgeAt(const QPoint &pos) const;
    void resizeWindow(const QPoint &globalPos);
    void updateResizeHandles();

    QWebEngineView *m_web = nullptr;
    TitleBar *m_titleBar = nullptr;
    FileWatcher *m_fileWatcher = nullptr;
    PageLoader *m_pageLoader = nullptr;

    QList<ResizeHandle *> m_resizeHandles;

    QString m_htmlPath;
    QString m_dir;

    bool m_resizing = false;
    int m_resizeEdges = NoEdge;
    QPoint m_resizeStartPos;
    QRect m_resizeStartGeometry;
};
