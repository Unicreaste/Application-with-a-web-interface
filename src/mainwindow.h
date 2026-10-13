#pragma once

#include <QWidget>
#include <QString>

class QWebEngineView;
class TitleBar;
class FileWatcher;
class PageLoader;
class QResizeEvent;

class MainWindow : public QWidget
{

  public:
    explicit MainWindow(QWidget *parent = nullptr);

  protected:
    void resizeEvent(QResizeEvent *event) override;

  private:
    QWebEngineView *m_web = nullptr;
    TitleBar *m_titleBar = nullptr;
    FileWatcher *m_fileWatcher = nullptr;
    PageLoader *m_pageLoader = nullptr;

    QString m_htmlPath;
    QString m_dir;
};
