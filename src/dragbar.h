#pragma once

#include <QWidget>

class QMouseEvent;

class DragBar : public QWidget
{
  public:
    explicit DragBar(QWidget *parent = nullptr);

  protected:
    void mousePressEvent(QMouseEvent *event) override;
};