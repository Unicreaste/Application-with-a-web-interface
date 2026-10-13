#include "mainwindow.h"

#include "titlebar.h"
#include "filewatcher.h"
#include "pageloader.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QDebug>
#include <QResizeEvent>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    resize(900, 700);

    m_htmlPath = QFileInfo("index.html").absoluteFilePath();
    m_dir = QFileInfo(m_htmlPath).absolutePath();

    qDebug() << "HTML path:" << m_htmlPath;
    qDebug() << "HTML exists:" << QFileInfo::exists(m_htmlPath);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_web = new QWebEngineView(this);
    layout->addWidget(m_web);

    m_web->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);

    m_web->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

    connect(m_web, &QWebEngineView::loadFinished, this, [](bool ok) { qDebug() << "Page loaded:" << ok; });

    m_titleBar = new TitleBar(this);

    m_fileWatcher = new FileWatcher(
        {m_htmlPath, m_dir + "/css/style.css", m_dir + "/js/script.js", m_dir + "/images/1.webp",
         m_dir + "/images/2.webp", m_dir + "/images/3.webp"},
        [this]() {
            qDebug() << "Reloading page after file changes";
            m_pageLoader->loadPage();
        },
        this);

    m_pageLoader = new PageLoader(m_web, m_htmlPath, this);
    m_pageLoader->loadPage();
}

void
MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_titleBar) {
        m_titleBar->updateGeometry(width());
    }
}
