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
#include <QColor>
#include <QGraphicsDropShadowEffect>
#include <QVariantAnimation>
#include <QEasingCurve>

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

class GlowButton : public QPushButton
{
  public:
    GlowButton(const QString &text, const QColor &glowColor, QWidget *parent = nullptr)
        : QPushButton(text, parent), m_glowColor(glowColor)
    {
        setFixedSize(30, 30);
        setCursor(Qt::PointingHandCursor);
        setFocusPolicy(Qt::NoFocus);

        // Начальный фон и цвет символа
        m_background = QColor(0, 0, 0, 110);
        m_foreground = QColor(Qt::white);

        setStyleSheet(QString(R"(
    QPushButton {
        background-color: %1;
        border: 1px solid %2;
        border-radius: 12px;
        color: %3;
        font-size: 16px;
        font-weight: bold;
        padding: 0px;
    }
)")
                          .arg(m_background.name(QColor::HexArgb), m_glowColor.name(), m_foreground.name()));

        // Эффект свечения
        m_shadow = new QGraphicsDropShadowEffect(this);
        m_shadow->setOffset(0, 0);
        m_shadow->setBlurRadius(12);

        QColor shadowColor = m_glowColor;
        shadowColor.setAlpha(110);
        m_shadow->setColor(shadowColor);

        setGraphicsEffect(m_shadow);

        // Анимация
        m_animation = new QVariantAnimation(this);
        m_animation->setDuration(220);
        m_animation->setStartValue(0.0);
        m_animation->setEndValue(1.0);
        m_animation->setEasingCurve(QEasingCurve::OutCubic);

        connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
            const qreal t = value.toReal();

            // Прозрачный фон плавно заполняется цветом кнопки
            QColor bg = m_glowColor;
            bg.setAlpha(qRound(35 + 220 * t));

            // Белый символ плавно становится тёмным
            const int channel = qRound(255 * (1.0 - t));
            QColor fg(channel, channel, channel);

            setStyleSheet(QString(R"(
                QPushButton {
                    background-color: %1;
                    border: 1px solid %2;
                    border-radius: 12px;
                    color: %3;
                    font-size: 16px;
                    font-weight: bold;
                    padding: 0px;
                }
            )")
                              .arg(bg.name(QColor::HexArgb), m_glowColor.name(), fg.name()));

            // Усиление свечения
            m_shadow->setBlurRadius(12 + 12 * t);

            QColor glow = m_glowColor;
            glow.setAlpha(qRound(110 + 145 * t));
            m_shadow->setColor(glow);
        });

        m_animation->setCurrentTime(0);
    }

  protected:
    void
    enterEvent(QEnterEvent *event) override
    {
        QPushButton::enterEvent(event);
        animateTo(1.0);
    }

    void
    leaveEvent(QEvent *event) override
    {
        QPushButton::leaveEvent(event);
        animateTo(0.0);
    }

  private:
    QColor m_glowColor;
    QColor m_background;
    QColor m_foreground;

    QGraphicsDropShadowEffect *m_shadow = nullptr;
    QVariantAnimation *m_animation = nullptr;

    void
    animateTo(qreal target)
    {
        // Начинаем с текущего положения анимации,
        // чтобы движение не прерывалось рывком.
        const QVariant current = m_animation->currentValue();

        m_animation->stop();
        m_animation->setStartValue(current.isValid() ? current : QVariant(0.0));
        m_animation->setEndValue(target);
        m_animation->start();
    }
};

class MainWindow : public QWidget
{
    // --- Поля класса объявлены В НАЧАЛЕ, чтобы конструктор их видел ---
    QWebEngineView *m_web = nullptr;
    DragBar *m_bar = nullptr;
    QPushButton *m_closeBtn = nullptr;
    QPushButton *m_minimizeBtn = nullptr;
    QPushButton *m_maximizeBtn = nullptr;
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

        // --- Красная кнопка закрытия ---
        m_closeBtn = new GlowButton("✖", QColor(255, 55, 65), this);

        connect(m_closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);

        // --- Зелёная кнопка сворачивания ---
        m_minimizeBtn = new GlowButton("—", QColor(0, 255, 125), this);

        connect(m_minimizeBtn, &QPushButton::clicked, this, &QWidget::showMinimized);

        // --- Жёлтая кнопка разворачивания ---
        m_maximizeBtn = new GlowButton("□", QColor(255, 215, 0), this);

        connect(m_maximizeBtn, &QPushButton::clicked, this, [this]() {
            if (isMaximized()) {
                showNormal();
            } else {
                showMaximized();
            }
        });

        // --- Порядок отображения элементов ---
        m_bar->raise();
        m_minimizeBtn->raise();
        m_maximizeBtn->raise();
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
        const int margin = 8;
        const int spacing = 6;
        const int y = margin;

        if (m_closeBtn) {
            m_closeBtn->move(width() - m_closeBtn->width() - margin, y);
        }

        if (m_maximizeBtn && m_closeBtn) {
            m_maximizeBtn->move(m_closeBtn->x() - spacing - m_maximizeBtn->width(), y);
        }

        if (m_minimizeBtn && m_maximizeBtn) {
            m_minimizeBtn->move(m_maximizeBtn->x() - spacing - m_minimizeBtn->width(), y);
        }
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
