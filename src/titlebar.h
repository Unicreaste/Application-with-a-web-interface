
#pragma once

#include <QWidget>

class DragBar;
class GlowButton;

class TitleBar
{
  public:
    explicit TitleBar(QWidget *window);

    void updateGeometry(int windowWidth);

  private:
    QWidget *m_window = nullptr;
    DragBar *m_bar = nullptr;

    GlowButton *m_closeBtn = nullptr;
    GlowButton *m_minimizeBtn = nullptr;
    GlowButton *m_maximizeBtn = nullptr;
};
