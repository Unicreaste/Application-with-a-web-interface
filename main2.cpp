#include <QApplication>
#include <QWidget>
#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QUrl>
#include <QTimer>
#include <QVBoxLayout>
#include <QWindow>
#include <QMouseEvent>
#include <QDebug>
#include <QPushButton>

class DragBar : public QWidget
{
  public:
    explicit DragBar(QWidget *parent = nullptr) : QWidget(parent)
    {
        setFixedHeight(28);
        setStyleSheet("background: transparent;");
        setCursor(Qt::SizeAllCursor);
    }

  protected:
    void
    mousePressEvent(QMouseEvent *event) override
    {
        if (event->button() == Qt::LeftButton) {
            if (window()->windowHandle()) {
                window()->windowHandle()->startSystemMove();
            }

            event->accept();
        }
    }
};

class MainWindow : public QWidget
{
    // --- Поля класса объявлены В НАЧАЛЕ, чтобы конструктор их видел ---
    QWebEngineView *m_web = nullptr;
    DragBar *m_bar = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QFileSystemWatcher m_watcher;
    QString m_htmlPath;
    QString m_dir;
    bool m_reloadPending = false;

  public:
    MainWindow()
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

        // --- Drag-полоса сверху ---
        m_bar = new DragBar(this);
        m_bar->setGeometry(0, 0, width(), 28);

        // --- Кнопка закрытия ---
        m_closeBtn = new QPushButton("✖", this);
        m_closeBtn->setFixedSize(34, 34);
        m_closeBtn->setCursor(Qt::PointingHandCursor);
        m_closeBtn->setStyleSheet(R"(
    QPushButton {
        background: rgba(0, 0, 0, 110);
        border: none;
        border-radius: 17px;
        color: #fff;
        font-size: 16px;
        font-weight: bold;
        padding: 0;
        text-shadow:
            0 0 4px rgba(0, 0, 0, 0.9),
            0 1px 2px rgba(0, 0, 0, 0.9);
    }
    QPushButton:hover {
        background: rgba(220, 40, 40, 220);
        color: #ffffff;
    }
    QPushButton:pressed {
        background: rgba(180, 20, 20, 240);
    }
)");
        connect(m_closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);

        // Порядок raise важен: bar снизу, кнопка сверху
        m_bar->raise();
        m_closeBtn->raise();

        updateCloseButtonGeometry();

        // --- Слежение за файлами ---
        connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this](const QString &path) {
            qDebug() << "File changed:" << path;

            QTimer::singleShot(250, this, [this, path]() {
                watchFile(path);
                reloadPage();
            });
        });

        watchFile(m_htmlPath);
        watchFile(m_dir + "/style.css");
        watchFile(m_dir + "/script.js");

        loadPage();
    }

  protected:
    void
    resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);

        if (m_bar) {
            m_bar->setGeometry(0, 0, width(), 28);
        }

        updateCloseButtonGeometry();
    }

  private:
    void
    updateCloseButtonGeometry()
    {
        if (!m_closeBtn) {
            return;
        }

        const int margin = 8;
        m_closeBtn->move(width() - m_closeBtn->width() - margin, margin);
    }

    void
    watchFile(const QString &path)
    {
        const QString absolutePath = QFileInfo(path).absoluteFilePath();

        if (QFileInfo::exists(absolutePath) && !m_watcher.files().contains(absolutePath)) {
            m_watcher.addPath(absolutePath);
            qDebug() << "Watching:" << absolutePath;
        }
    }

    void
    loadPage()
    {
        if (!QFileInfo::exists(m_htmlPath)) {
            m_web->setHtml("<html><body style='background:#222;color:white'>"
                           "<h2>index.html not found</h2>"
                           "<p>Expected file: " +
                           m_htmlPath.toHtmlEscaped() + "</p></body></html>");
            return;
        }

        m_web->load(QUrl::fromLocalFile(m_htmlPath));
    }

    void
    reloadPage()
    {
        if (m_reloadPending) {
            return;
        }

        m_reloadPending = true;

        QTimer::singleShot(200, this, [this]() {
            m_reloadPending = false;

            watchFile(m_htmlPath);
            watchFile(m_dir + "/style.css");
            watchFile(m_dir + "/script.js");

            loadPage();
        });
    }
};

int
main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow window;
    window.show();

    return app.exec();
}
