
#pragma once

#include <QPushButton>
#include <QColor>

class QGraphicsDropShadowEffect;
class QVariantAnimation;
class QEnterEvent;
class QEvent;

class GlowButton : public QPushButton
{
  public:
    GlowButton(const QString &text, const QColor &glowColor, QWidget *parent = nullptr);

  protected:
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

  private:
    QColor m_glowColor;
    QColor m_background;
    QColor m_foreground;

    QGraphicsDropShadowEffect *m_shadow = nullptr;
    QVariantAnimation *m_animation = nullptr;
    void updateAppearance(qreal t);

    void animateTo(qreal target);
};
