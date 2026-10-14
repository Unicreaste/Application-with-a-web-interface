#pragma once

#include <QWidget>
#include <Qt>

class QMouseEvent;

class ResizeHandle : public QWidget
{
  public:
    ResizeHandle(Qt::Edges edges, QWidget *parent = nullptr);

  protected:
    void mousePressEvent(QMouseEvent *event) override;

  private:
    Qt::Edges m_edges;
};
