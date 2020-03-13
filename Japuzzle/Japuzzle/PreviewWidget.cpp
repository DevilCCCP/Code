#include <QPaintEvent>
#include <QPainter>

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
  painter.drawPixmap(rect, mPreview);
}

void PreviewWidget::Setup()
{
  if (!qPuzzle) {
    resize(0, 0);
    return;
  }

  mLocation = QRect(0, 0, qPuzzle->getWidth() - 1, qPuzzle->getHeight() - 1);
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
  QPainter painter(&mBackground);
  painter.setPen(qStyle->getBackColor());
  painter.drawRect(QRect(0, 0, mBackground.width() - 1, mBackground.height() - 1));
//  painter.drawLine(0, 0, mBackground.width() - 1, 0);
//  painter.drawLine(0, mBackground.height() - 1, mBackground.width() - 1, mBackground.height() - 1);
//  painter.drawLine(0, 0, 0, mBackground.height() - 1);
//  painter.drawLine(mBackground.width() - 1, 0, mBackground.width() - 1, mBackground.height() - 1);
  for (int j = 0; j < qPuzzle->getHeight(); j++) {
    for (int i = 0; i < qPuzzle->getWidth(); i++) {
      const Cell& cell = qPuzzle->At(i, j);
      if (cell.IsMarkYes()) {
        painter.setPen(cell.MarkLevel() > 0? qStyle->getYes1Color(): qStyle->getYesColor());
      } else if (cell.IsMarkNo()) {
        painter.setPen(cell.MarkLevel() > 0? qStyle->getNo1Color(): qStyle->getNoColor());
      } else {
        painter.setPen(qStyle->getBackColor());
      }
      painter.drawPoint(i + 1, j + 1);
    }
  }
  DrawLocation();
}

void PreviewWidget::DrawLocation()
{
  mPreview = mBackground.copy(mBackground.rect());
  QRect border(mLocation.left(), mLocation.top(), mLocation.width() + 2, mLocation.height() + 2);

  QPainter painter(&mPreview);
  painter.drawPixmap(border, mBackground, border);

  painter.setPen(qStyle->getPreviewLineColor());
  painter.drawRect(border);

  update(border);
}


PreviewWidget::PreviewWidget(QWidget* parent)
  : QWidget(parent)
{
}

