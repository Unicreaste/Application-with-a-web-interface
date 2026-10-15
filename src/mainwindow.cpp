
#include "mainwindow.h"

#include "titlebar.h"
#include "filewatcher.h"
#include "pageloader.h"
#include "database_manager.h"
#include "task_positions.h"
#include "chat_manager.h"
#include "theme_manager.h"

#include <QWebEngineView>
#include <QWebEngineSettings>
#include <QFileInfo>
#include <QVBoxLayout>
#include <QDebug>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QCursor>
#include <QApplication>
#include <QWindow>

#include <QStandardPaths>
#include <QDir>
#include <QDebug>
#include <memory>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineScript>
#include <QWebEngineScriptCollection>

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    setMinimumSize(400, 300);
    resize(900, 700);
    setMouseTracking(true);

    m_htmlPath = QFileInfo("index.html").absoluteFilePath();
    m_dir = QFileInfo(m_htmlPath).absolutePath();

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    qDebug() << "App data directory:" << dataDir;

    if (!QDir().mkpath(dataDir)) {
        qDebug() << "Failed to create directory:" << dataDir;
    }

    QString dbPath = dataDir + "/motiondesk.db";

    m_database = std::make_unique<Database>(dbPath.toStdString());

    if (!m_database->isOpen()) {
        qDebug() << "Failed to open database:" << dbPath;
    } else {
        qDebug() << "Database opened successfully:" << dbPath;
    }

    qDebug() << "HTML path:" << m_htmlPath;
    qDebug() << "HTML exists:" << QFileInfo::exists(m_htmlPath);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_web = new QWebEngineView(this);
    layout->addWidget(m_web);

    if (m_database && m_database->isOpen()) {
        m_taskPositions = std::make_unique<TaskPositions>(*m_database);

        // Создаём менеджер чата до регистрации в WebChannel
        m_chatManager = std::make_unique<ChatManager>(*m_database);
        m_themeManager = std::make_unique<ThemeManager>();

        m_webChannel = new QWebChannel(this);

        m_webChannel->registerObject(QStringLiteral("taskPositions"), m_taskPositions.get());
        m_webChannel->registerObject(QStringLiteral("chatManager"), m_chatManager.get());
        m_webChannel->registerObject(QStringLiteral("themeManager"), m_themeManager.get());

        m_web->page()->setWebChannel(m_webChannel);

        qDebug() << "Task positions and chat bridge initialized";
    }

    QWebEngineScript themeScript;
    themeScript.setName(QStringLiteral("initial-theme"));
    themeScript.setInjectionPoint(QWebEngineScript::DocumentCreation);
    themeScript.setWorldId(QWebEngineScript::MainWorld);
    themeScript.setRunsOnSubFrames(false);

    const QString theme = m_themeManager->currentTheme();

    QString script = QStringLiteral(R"JS(
(function() {
    const theme = "%1";

    function applyInitialTheme() {
        const root = document.documentElement;

        if (!root) {
            requestAnimationFrame(applyInitialTheme);
            return;
        }

        root.dataset.theme = theme;

        const style = document.createElement('style');
        style.id = 'initial-theme-no-animation';
        style.textContent =
            '*, *::before, *::after {' +
            ' transition: none !important;' +
            ' animation: none !important;' +
            '}';

        (document.head || root).appendChild(style);

        window.addEventListener('load', function() {
            requestAnimationFrame(function() {
                const initialStyle =
                    document.getElementById('initial-theme-no-animation');

                if (initialStyle)
                    initialStyle.remove();
            });
        }, { once: true });
    }

    applyInitialTheme();
})();
)JS")
                         .arg(theme);

    themeScript.setSourceCode(script);

    m_web->page()->scripts().insert(themeScript);

    m_web->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessFileUrls, true);
    m_web->settings()->setAttribute(QWebEngineSettings::LocalContentCanAccessRemoteUrls, true);

    connect(m_web, &QWebEngineView::loadFinished, this, [](bool ok) { qDebug() << "Page loaded:" << ok; });

    m_titleBar = new TitleBar(this);

    // Восемь зон изменения размера по периметру окна.
    // Каждая зона имеет ширину 5 пикселей.
    m_resizeHandles = {new ResizeHandle(Qt::TopEdge, this),
                       new ResizeHandle(Qt::BottomEdge, this),
                       new ResizeHandle(Qt::LeftEdge, this),
                       new ResizeHandle(Qt::RightEdge, this),

                       new ResizeHandle(Qt::TopEdge | Qt::LeftEdge, this),
                       new ResizeHandle(Qt::TopEdge | Qt::RightEdge, this),
                       new ResizeHandle(Qt::BottomEdge | Qt::LeftEdge, this),
                       new ResizeHandle(Qt::BottomEdge | Qt::RightEdge, this)};

    updateResizeHandles();

    m_fileWatcher = new FileWatcher(
        {m_htmlPath, m_dir + "/css/Dragg_elem.css", m_dir + "/js/drag_elem.js", m_dir + "/images/1.webp",
         m_dir + "/images/2.webp", m_dir + "/images/3.webp"},
        [this]() {
            qDebug() << "Reloading page after file changes";
            m_pageLoader->loadPage();
        },
        this);

    m_pageLoader = new PageLoader(m_web, m_htmlPath, this);
    m_pageLoader->loadPage();
}

MainWindow::~MainWindow() = default;

void
MainWindow::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    if (m_titleBar) {
        m_titleBar->updateGeometry(width());
    }

    updateResizeHandles();
}

void
MainWindow::updateResizeHandles()
{
    constexpr int border = 5;

    const int w = width();
    const int h = height();

    if (m_resizeHandles.size() != 8) {
        return;
    }

    // Верхняя и нижняя стороны.
    m_resizeHandles[0]->setGeometry(border, 0, qMax(0, w - 2 * border), border);

    m_resizeHandles[1]->setGeometry(border, h - border, qMax(0, w - 2 * border), border);

    // Левая и правая стороны.
    m_resizeHandles[2]->setGeometry(0, border, border, qMax(0, h - 2 * border));

    m_resizeHandles[3]->setGeometry(w - border, border, border, qMax(0, h - 2 * border));

    // Углы.
    m_resizeHandles[4]->setGeometry(0, 0, border, border);
    m_resizeHandles[5]->setGeometry(w - border, 0, border, border);
    m_resizeHandles[6]->setGeometry(0, h - border, border, border);
    m_resizeHandles[7]->setGeometry(w - border, h - border, border, border);

    // Зоны должны находиться поверх QWebEngineView.
    for (auto *handle : m_resizeHandles) {
        handle->raise();
    }
}

int
MainWindow::resizeEdgeAt(const QPoint &pos) const
{
    constexpr int border = 5;

    int edges = NoEdge;

    if (pos.x() < border) {
        edges |= LeftEdge;
    } else if (pos.x() >= width() - border) {
        edges |= RightEdge;
    }

    if (pos.y() < border) {
        edges |= TopEdge;
    } else if (pos.y() >= height() - border) {
        edges |= BottomEdge;
    }

    return edges;
}

void
MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !isMaximized()) {

        const int edges = resizeEdgeAt(event->position().toPoint());

        if (edges != NoEdge && windowHandle()) {
            Qt::Edges systemEdges;

            if (edges & LeftEdge) {
                systemEdges |= Qt::LeftEdge;
            }

            if (edges & RightEdge) {
                systemEdges |= Qt::RightEdge;
            }

            if (edges & TopEdge) {
                systemEdges |= Qt::TopEdge;
            }

            if (edges & BottomEdge) {
                systemEdges |= Qt::BottomEdge;
            }

            if (windowHandle()->startSystemResize(systemEdges)) {
                event->accept();
                return;
            }
        }
    }

    QWidget::mousePressEvent(event);
}

void
MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    const int edges = resizeEdgeAt(event->position().toPoint());

    if (isMaximized() || edges == NoEdge) {
        unsetCursor();
    } else if ((edges & LeftEdge && edges & TopEdge) || (edges & RightEdge && edges & BottomEdge)) {
        setCursor(Qt::SizeFDiagCursor);
    } else if ((edges & RightEdge && edges & TopEdge) || (edges & LeftEdge && edges & BottomEdge)) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (edges & LeftEdge || edges & RightEdge) {
        setCursor(Qt::SizeHorCursor);
    } else {
        setCursor(Qt::SizeVerCursor);
    }

    QWidget::mouseMoveEvent(event);
}

void
MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_resizing) {
        m_resizing = false;
        m_resizeEdges = NoEdge;
        event->accept();
        return;
    }

    QWidget::mouseReleaseEvent(event);
}

void
MainWindow::resizeWindow(const QPoint &globalPos)
{
    const QPoint delta = globalPos - m_resizeStartPos;
    QRect newGeometry = m_resizeStartGeometry;

    // Левая и правая стороны.
    if (m_resizeEdges & LeftEdge) {
        const int fixedRight = m_resizeStartGeometry.right();
        newGeometry.setLeft(m_resizeStartGeometry.left() + delta.x());
        newGeometry.setRight(fixedRight);
    } else if (m_resizeEdges & RightEdge) {
        newGeometry.setRight(m_resizeStartGeometry.right() + delta.x());
    }

    // Верхняя и нижняя стороны.
    if (m_resizeEdges & TopEdge) {
        const int fixedBottom = m_resizeStartGeometry.bottom();
        newGeometry.setTop(m_resizeStartGeometry.top() + delta.y());
        newGeometry.setBottom(fixedBottom);
    } else if (m_resizeEdges & BottomEdge) {
        newGeometry.setBottom(m_resizeStartGeometry.bottom() + delta.y());
    }

    // Сохраняем минимальный размер окна.
    if (newGeometry.width() < minimumWidth()) {
        if (m_resizeEdges & LeftEdge) {
            newGeometry.setLeft(newGeometry.right() - minimumWidth() + 1);
        } else {
            newGeometry.setRight(newGeometry.left() + minimumWidth() - 1);
        }
    }

    if (newGeometry.height() < minimumHeight()) {
        if (m_resizeEdges & TopEdge) {
            newGeometry.setTop(newGeometry.bottom() - minimumHeight() + 1);
        } else {
            newGeometry.setBottom(newGeometry.top() + minimumHeight() - 1);
        }
    }

    setGeometry(newGeometry);
}
