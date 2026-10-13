#include "glowbutton.h"

#include <QGraphicsDropShadowEffect>
#include <QVariantAnimation>
#include <QEasingCurve>
#include <QEnterEvent>
#include <QEvent>

GlowButton::GlowButton(const QString &text, const QColor &glowColor, QWidget *parent)
    : QPushButton(text, parent), m_glowColor(glowColor)
{
    setFixedSize(30, 30);
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);

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

    m_shadow = new QGraphicsDropShadowEffect(this);
    m_shadow->setOffset(0, 0);
    m_shadow->setBlurRadius(12);

    QColor shadowColor = m_glowColor;
    shadowColor.setAlpha(110);
    m_shadow->setColor(shadowColor);

    setGraphicsEffect(m_shadow);

    m_animation = new QVariantAnimation(this);
    m_animation->setDuration(220);
    m_animation->setStartValue(0.0);
    m_animation->setEndValue(1.0);
    m_animation->setEasingCurve(QEasingCurve::OutCubic);

    connect(m_animation, &QVariantAnimation::valueChanged, this, [this](const QVariant &value) {
        const qreal t = value.toReal();

        QColor bg = m_glowColor;
        bg.setAlpha(qRound(35 + 220 * t));

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

        m_shadow->setBlurRadius(12 + 12 * t);

        QColor glow = m_glowColor;
        glow.setAlpha(qRound(110 + 145 * t));
        m_shadow->setColor(glow);
    });

    m_animation->setCurrentTime(0);
}

void
GlowButton::enterEvent(QEnterEvent *event)
{
    QPushButton::enterEvent(event);
    animateTo(1.0);
}

void
GlowButton::leaveEvent(QEvent *event)
{
    QPushButton::leaveEvent(event);
    animateTo(0.0);
}

void
GlowButton::animateTo(qreal target)
{
    const QVariant current = m_animation->currentValue();

    m_animation->stop();
    m_animation->setStartValue(current.isValid() ? current : QVariant(0.0));
    m_animation->setEndValue(target);
    m_animation->start();
}
