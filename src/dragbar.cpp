#include "dragbar.h"

#include <QMouseEvent>
#include <QWindow>

DragBar::DragBar(QWidget *parent)
    : QWidget(parent)
{
    setFixedHeight(28);
    setStyleSheet("background: transparent;");
    setCursor(Qt::SizeAllCursor);
}

void DragBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (window()->windowHandle()) {
            window()->windowHandle()->startSystemMove();
        }
        event->accept();
    }
}