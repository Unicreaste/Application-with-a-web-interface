
#include "../pch.h"
#include "titlebar.h"

#include "dragbar.h"
#include "glowbutton.h"

#include <QApplication>
#include <QPushButton>
#include <QColor>

TitleBar::TitleBar(QWidget *window) : m_window(window)
{
    // Полоса для перетаскивания окна.
    m_bar = new DragBar(m_window);
    m_bar->setGeometry(0, 0, m_window->width(), 28);

    // Кнопка закрытия.
    m_closeBtn = new GlowButton("✖", QColor(255, 55, 65), m_window);

    QObject::connect(m_closeBtn, &QPushButton::clicked, qApp, &QApplication::quit);

    // Кнопка сворачивания.
    m_minimizeBtn = new GlowButton("—", QColor(0, 255, 125), m_window);

    QObject::connect(m_minimizeBtn, &QPushButton::clicked, m_window, &QWidget::showMinimized);

    // Кнопка разворачивания и восстановления.
    m_maximizeBtn = new GlowButton("□", QColor(255, 215, 0), m_window);

    QObject::connect(m_maximizeBtn, &QPushButton::clicked, m_window, [this]() {
        if (m_window->isMaximized()) {
            m_window->showNormal();
        } else {
            m_window->showMaximized();
        }
    });

    updateGeometry(m_window->width());

    // Сохраняем порядок отображения элементов.
    m_bar->raise();
    m_minimizeBtn->raise();
    m_maximizeBtn->raise();
    m_closeBtn->raise();
}

void
TitleBar::updateGeometry(int windowWidth)
{
    if (m_bar) {
        m_bar->setGeometry(0, 0, windowWidth, 28);
    }

    const int margin = 8;
    const int spacing = 6;
    const int y = margin;

    if (m_closeBtn) {
        m_closeBtn->move(windowWidth - m_closeBtn->width() - margin, y);
    }

    if (m_maximizeBtn && m_closeBtn) {
        m_maximizeBtn->move(m_closeBtn->x() - spacing - m_maximizeBtn->width(), y);
    }

    if (m_minimizeBtn && m_maximizeBtn) {
        m_minimizeBtn->move(m_maximizeBtn->x() - spacing - m_minimizeBtn->width(), y);
    }
}
