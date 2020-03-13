#include <QPainter>
#include <QPaintEvent>

#include "FrameLabel.h"


FrameLabel::FrameLabel(QWidget* parent)
  : QWidget(parent)
{
}


void FrameLabel::paintEvent(QPaintEvent* event)
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  event->accept();
}

void FrameLabel::SetFrame(const FrameS& _Frame)
{
  mFrame = _Frame;

  update();
}

