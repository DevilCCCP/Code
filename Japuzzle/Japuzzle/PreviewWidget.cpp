#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>

#include "PreviewWidget.h"
#include "Core.h"
#include "Puzzle.h"
#include "Style.h"
#include "Cell.h"
#include "Account.h"


void PreviewWidget::paintEvent(QPaintEvent* event)
{
  const QRect rect = event->rect();
  QPainter painter(this);
  painter.drawPixmap(rect, mBackground);
}

void PreviewWidget::resizeEvent(QResizeEvent* event)
{
  QWidget::resizeEvent(event);
}

void PreviewWidget::mousePressEvent(QMouseEvent* event)
{
  if (!qPuzzle) {
    return;
  }

  int i = qBound(0, event->pos().x() * qPuzzle->getWidth() / width(), qPuzzle->getWidth() - 1);
  int j = qBound(0, event->pos().y() * qPuzzle->getHeight() / height(), qPuzzle->getHeight() - 1);

  emit MoveToLocation(i, j);
}

void PreviewWidget::Setup()
{
  if (!qPuzzle) {
    resize(0, 0);
    return;
  }

  mLocation = QRect(0, 0, qPuzzle->getWidth() - 1, qPuzzle->getHeight() - 1);
  mFullRect = QRect(0, 0, qPuzzle->getWidth() + 1, qPuzzle->getHeight() + 1);
  DrawPreview();
  setMinimumSize(DefaultSize());
  setMaximumSize(DefaultSize());
  resize(DefaultSize());

  update();
}

void PreviewWidget::SetLocation(const QRect& _Location)
{
  mLocation = _Location;

  DrawPreview();
}

void PreviewWidget::UpdateFull()
{
  DrawPreview();
  update();
}

QSize PreviewWidget::DefaultSize() const
{
  return qAccount->getPreviewSize()*mBackground.size();
}

void PreviewWidget::DrawPreview()
{
  mBackground = QPixmap(qPuzzle->getWidth() + 2, qPuzzle->getHeight() + 2);
  mBackground.fill(qStyle->getBackColor());
  QPainter painter(&mBackground);
  painter.setPen(qStyle->getBackColor());

  DrawPicture(&painter);
  DrawLocation(&painter);

  update();
}

void PreviewWidget::DrawPicture(QPainter* painter)
{
  for (int j = 0; j < qPuzzle->getHeight(); j++) {
    for (int i = 0; i < qPuzzle->getWidth(); i++) {
      const Cell& cell = qPuzzle->At(i, j);
      if (cell.IsMarkYes()) {
        painter->setPen(cell.MarkLevel() > 0? qStyle->getYes1Color(): qStyle->getYesColor());
      } else if (cell.IsMarkNo()) {
        painter->setPen(cell.MarkLevel() > 0? qStyle->getNo1Color(): qStyle->getNoColor());
      } else {
        painter->setPen(qStyle->getBackColor());
      }
      painter->drawPoint(i + 1, j + 1);
    }
  }
}

void PreviewWidget::DrawLocation(QPainter* painter)
{
  QRect visibleRect(mLocation.left(), mLocation.top(), mLocation.width() + 2, mLocation.height() + 2);
  painter->setPen(qStyle->getPreviewLineColor());
  painter->drawRect(visibleRect);
}


PreviewWidget::PreviewWidget(QWidget* parent)
  : QWidget(parent)
{
}

