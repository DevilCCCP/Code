#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>

#include "EditWidget.h"
#include "Core.h"
#include "Style.h"
#include "Puzzle.h"
#include "Editing.h"
#include "Decoration.h"
#include "Account.h"


void EditWidget::SetBrickSize(int value)
{
  mBrickSize = value;
}

void EditWidget::paintEvent(QPaintEvent* event)
{
  QWidgetB::paintEvent(event);

  if (!mPuzzle) {
    return;
  }

  const QRect& rect = event->rect();
  QPainter painter(this);
  painter.setPen(qStyle->getLineColor());
  for (int j = 0; j <= mPuzzle->getHeight(); j++) {
    int y = mDecoration->CellToPointY(j) - 1;
    if (y < rect.top() || y - 1 > rect.bottom()) {
      continue;
    }
    painter.drawLine(rect.left(), y, rect.right(), y);
    if (j % 5 == 0) {
      painter.drawLine(rect.left(), y - 1, rect.right(), y - 1);
    }
  }
  for (int i = 0; i <= mPuzzle->getWidth(); i++) {
    int x = mDecoration->CellToPointX(i) - 1;
    if (x < rect.left() || x - 1 > rect.right()) {
      continue;
    }
    painter.drawLine(x, rect.top(), x, rect.bottom());
    if (i % 5 == 0) {
      painter.drawLine(x - 1, rect.top(), x - 1, rect.bottom());
    }
  }
  painter.setRenderHint(QPainter::Antialiasing);
  for (int j = 0; j < mPuzzle->getHeight(); j++) {
    int y = mDecoration->CellToPointY(j);
    if (y + mCellWidth < rect.top() || y > rect.bottom()) {
      continue;
    }
    for (int i = 0; i < mPuzzle->getWidth(); i++) {
      int x = mDecoration->CellToPointX(i);
      if (x + mCellWidth < rect.left() || x > rect.right()) {
        continue;
      }

      const Cell& cell = mPuzzle->At(i, j);
      if (cell.Real()) {
        painter.drawImage(QRect(x, y, mCellWidth, mCellHeight), qStyle->getYes().value(0));
      } else {
        painter.drawImage(QRect(x, y, mCellWidth, mCellHeight), qStyle->getNull());
      }
    }
  }
}

void EditWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (mPuzzle && TranslateCell(event->x(), event->y())) {
    SetCells(mCurrentPos, mCurrentSet);
  }

  return QWidgetB::mousePressEvent(event);
}

void EditWidget::mousePressEvent(QMouseEvent* event)
{
  if (mPuzzle && TranslateCell(event->x(), event->y())) {
    mCurrentSet = !mPuzzle->At(mCurrentPos).Real();
    SetCells(mCurrentPos, mCurrentSet);
  }

  return QWidgetB::mousePressEvent(event);
}

void EditWidget::Setup(const PuzzleS& _Puzzle, const DecorationS& _Decoration)
{
  mPuzzle     = _Puzzle;
  mDecoration = _Decoration;
  mDecoration->SetPuzzle(mPuzzle);
  UpdatePuzzle();
  UpdateSize();

  mHasChanges = false;
}

void EditWidget::UpdateSize()
{
  mCellWidth  = mDecoration->getCellWidth();
  mCellHeight = mDecoration->getCellHeight();
  mCurrentPos = QPoint(-1, -1);

  if (mPuzzle) {
    mWidth  = mDecoration->GetTableWidth();
    mHeight = mDecoration->GetTableHeight();
  } else {
    mWidth = mHeight = 0;
  }

  setMinimumSize(mWidth, mHeight);
  setMaximumSize(mWidth, mHeight);
  resize(mWidth, mHeight);
}

void EditWidget::UpdatePuzzle()
{
  mWidth  = mPuzzle->getWidth();
  mHeight = mPuzzle->getHeight();

  update();
}

bool EditWidget::TranslateCell(int x, int y)
{
  int i = 0;
  int j = 0;
  if (!TranslatePointToCellX(x, i) || !TranslatePointToCellY(y, j)) {
    mCurrentPos = QPoint(-1, -1);
    return false;
  }

  mCurrentPos.setX(i);
  mCurrentPos.setY(j);
  return true;
}

bool EditWidget::TranslatePointToCellX(int x, int& i)
{
  i = (5*x - 10) / (5*mCellWidth + 5 + 1);
  if (i < 0 || i >= mPuzzle->getWidth()) {
    return false;
  }
  int l = mDecoration->CellToPointX(i);
  int r = l + mCellWidth;
  if (x < l || x > r) {
    return false;
  }
  return true;
}

bool EditWidget::TranslatePointToCellY(int y, int& j)
{
  j = (5*y - 10) / (5*mCellHeight + 5 + 1);
  if (j < 0 || j >= mPuzzle->getHeight()) {
    return false;
  }
  int t = mDecoration->CellToPointY(j);
  int b = t + mCellHeight;
  if (y < t || y > b) {
    return false;
  }
  return true;
}

void EditWidget::SetCells(const QPoint& p, bool set)
{
  bool hasChanges = false;
  QPoint p1 = p;
  QPoint p2 = p;
  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      int length = qAbs(i) + qAbs(j);
      if (length == 0 || (length == 1 && mBrickSize >= 5) || (length == 2 && mBrickSize >= 9)) {
        int iCell = p.x() + i;
        int jCell = p.y() + j;
        if (iCell >= 0 && iCell < mPuzzle->getWidth() && jCell >= 0 && jCell < mPuzzle->getHeight()) {
          if (mPuzzle->At(iCell, jCell).Real() != set) {
            hasChanges = true;
          }
        }
      }
    }
  }

  if (!hasChanges) {
    return;
  }

  mPuzzle->MakeUndo();
  mHasChanges = true;

  for (int j = -1; j <= 1; j++) {
    for (int i = -1; i <= 1; i++) {
      int length = qAbs(i) + qAbs(j);
      if (length == 0 || (length == 1 && mBrickSize >= 5) || (length == 2 && mBrickSize >= 9)) {
        int iCell = p.x() + i;
        int jCell = p.y() + j;
        if (iCell >= 0 && iCell < mPuzzle->getWidth() && jCell >= 0 && jCell < mPuzzle->getHeight()) {
          Cell* cell = &mPuzzle->Value(iCell, jCell);
          cell->SetReal(set);
        }
      }
    }
  }

  if (mBrickSize >= 5) {
    p1.rx()--;
    p1.ry()--;
    p2.rx()++;
    p2.ry()++;
  }

  UpdateCells(p1, p2);
}

void EditWidget::UpdateCells(const QPoint& p1, const QPoint& p2)
{
  int x1 = mDecoration->CellToPointX(qMin(p1.x(), p2.x()));
  int x2 = mDecoration->CellToPointX(qMax(p1.x(), p2.x())) + mCellWidth;
  int y1 = mDecoration->CellToPointY(qMin(p1.y(), p2.y()));
  int y2 = mDecoration->CellToPointY(qMax(p1.y(), p2.y())) + mCellHeight;

  update(x1, y1, x2 - x1, y2 - y1);
}


EditWidget::EditWidget(QWidget* parent)
  : QWidgetB(parent)
  , mCurrentSet(true)
{
}

