#include "resizehandle.h"

#include <QMouseEvent>
#include <QWindow>

ResizeHandle::ResizeHandle(Qt::Edges edges, QWidget *parent) : QWidget(parent), m_edges(edges)
{
    setAttribute(Qt::WA_NoSystemBackground);
    setAttribute(Qt::WA_TranslucentBackground);
    setMouseTracking(false);

    if ((edges.testFlag(Qt::LeftEdge) || edges.testFlag(Qt::RightEdge)) &&
        (edges.testFlag(Qt::TopEdge) || edges.testFlag(Qt::BottomEdge))) {
        if ((edges.testFlag(Qt::LeftEdge) && edges.testFlag(Qt::TopEdge)) ||
            (edges.testFlag(Qt::RightEdge) && edges.testFlag(Qt::BottomEdge))) {
            setCursor(Qt::SizeFDiagCursor);
        } else {
            setCursor(Qt::SizeBDiagCursor);
        }
    } else if (edges.testFlag(Qt::LeftEdge) || edges.testFlag(Qt::RightEdge)) {
        setCursor(Qt::SizeHorCursor);
    } else {
        setCursor(Qt::SizeVerCursor);
    }
}

void
ResizeHandle::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && window()->windowHandle()) {
        if (window()->windowHandle()->startSystemResize(m_edges)) {
            event->accept();
            return;
        }
    }

    QWidget::mousePressEvent(event);
}
