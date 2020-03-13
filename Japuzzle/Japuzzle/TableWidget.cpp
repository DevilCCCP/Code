#include <QPaintEvent>
#include <QPainter>
#include <QMouseEvent>

#include "TableWidget.h"
#include "Core.h"
#include "Style.h"
#include "Puzzle.h"
#include "Editing.h"
#include "Decoration.h"
#include "Account.h"


void TableWidget::paintEvent(QPaintEvent* event)
{
  QWidgetB::paintEvent(event);

  if (!qPuzzle) {
    return;
  }

  const QRect& rect = event->rect();
  QPainter painter(this);
  painter.setPen(qStyle->getLineColor());
  for (int j = 0; j <= qPuzzle->getHeight(); j++) {
    int y = qDecoration->CellToPointY(j) - 1;
    if (y < rect.top() || y - 1 > rect.bottom()) {
      continue;
    }
    painter.drawLine(rect.left(), y, rect.right(), y);
    if (j % 5 == 0) {
      painter.drawLine(rect.left(), y - 1, rect.right(), y - 1);
    }
  }
  for (int i = 0; i <= qPuzzle->getWidth(); i++) {
    int x = qDecoration->CellToPointX(i) - 1;
    if (x < rect.left() || x - 1 > rect.right()) {
      continue;
    }
    painter.drawLine(x, rect.top(), x, rect.bottom());
    if (i % 5 == 0) {
      painter.drawLine(x - 1, rect.top(), x - 1, rect.bottom());
    }
  }
  painter.setRenderHint(QPainter::Antialiasing);
  for (int j = 0; j < qPuzzle->getHeight(); j++) {
    int y = qDecoration->CellToPointY(j);
    if (y + mCellWidth < rect.top() || y > rect.bottom()) {
      continue;
    }
    for (int i = 0; i < qPuzzle->getWidth(); i++) {
      int x = qDecoration->CellToPointX(i);
      if (x + mCellWidth < rect.left() || x > rect.right()) {
        continue;
      }

      const Cell& cell = qPuzzle->At(i, j);
      if (HasSpot() && mSpotPos.x() == i && mSpotPos.y() == j) {
        painter.drawImage(QRect(x, y, mCellWidth, mCellHeight), qStyle->getSpot());
      } else if (cell.IsMarkYes()) {
        painter.drawImage(QRect(x, y, mCellWidth, mCellHeight), qStyle->getYes().value(cell.MarkLevel()));
      } else if (cell.IsMarkNo()) {
        painter.drawImage(QRect(x, y, mCellWidth, mCellHeight), qStyle->getNo().value(cell.MarkLevel()));
      } else {
        painter.drawImage(QRect(x, y, mCellWidth, mCellHeight), qStyle->getNull());
      }
    }
  }
}

void TableWidget::mouseMoveEvent(QMouseEvent* event)
{
  bool showCalcWindow = false;
  QPoint lastPos = mCurrentPos;
  if (TranslateCell(event->x(), event->y())) {
    if (HasSpot()) {
      if (mSpotPos == mCurrentPos) {
        qDecoration->CursorChange(mSpotMark > 0? Decoration::eCursorYesCell: Decoration::eCursorNoCell);
      } else {
        if (mSpotPos.x() == mCurrentPos.x()) {
          qDecoration->CursorChange(mSpotMark > 0? Decoration::eCursorYesVLine: Decoration::eCursorNoVLine);
        } else if (mSpotPos.y() == mCurrentPos.y()) {
          qDecoration->CursorChange(mSpotMark > 0? Decoration::eCursorYesHLine: Decoration::eCursorNoHLine);
        } else {
          qDecoration->CursorChange(mSpotMark > 0? Decoration::eCursorYesBlock: Decoration::eCursorNoBlock);
        }
        showCalcWindow = true;
      }
    } else {
      if (qEditing->getMode() == Editing::eModeErase && event->buttons()) {
        qPuzzle->Value(mCurrentPos).Clear();
        UpdateCells(mCurrentPos, mCurrentPos);
        emit UpdateDigits(mCurrentPos, mCurrentPos);
      }
      qDecoration->CursorChange(Decoration::eCursorNormalCell);
    }
  } else {
    qDecoration->CursorChange(Decoration::eCursorArrow);
  }

  if (mCurrentPos != qDecoration->getHighlightPos()) {
    if (qAccount->getDigitHighlight()) {
      qDecoration->HighlightPosChange(mCurrentPos);
    }
  }

  if (mShowCalcWindow != showCalcWindow || mShowCalcPos != mCurrentPos) {
    Qt::AlignmentFlag flag = (Qt::AlignmentFlag)0;
    if (lastPos.x() < mCurrentPos.x()) {
      flag = Qt::AlignRight;
    } else if (lastPos.x() > mCurrentPos.x()) {
      flag = Qt::AlignLeft;
    } else if (lastPos.y() < mCurrentPos.y()) {
      flag = Qt::AlignBottom;
    } else if (lastPos.y() > mCurrentPos.y()) {
      flag = Qt::AlignTop;
    } else {
      showCalcWindow = false;
    }

    mShowCalcWindow = showCalcWindow;
    mShowCalcPos = mCurrentPos;

    int x = qDecoration->CellToPointX(mShowCalcPos.x()) + mCellWidth / 2;
    int y = qDecoration->CellToPointY(mShowCalcPos.y()) + mCellHeight / 2;
    QPoint pos = QPoint(x, y);
    if (mShowCalcWindow) {
      emit ShowCalcWindow(mapToGlobal(pos), mSpotPos, mCurrentPos, mSpotMark, flag);
    } else {
      emit HideCalcWindow();
    }
  }
}

void TableWidget::mousePressEvent(QMouseEvent* event)
{
  if (HasSpot()) {
    mSpotMark = 0;
    qDecoration->CursorChange(Decoration::eCursorArrow);
    UpdateCells(mSpotPos, mSpotPos);
    if (mShowCalcWindow) {
      mShowCalcWindow = false;
      emit HideCalcWindow();
    }
  } else if (TranslateCell(event->x(), event->y())) {
    if (qEditing->getMode() == Editing::eModeErase) {
      if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        qPuzzle->Value(mCurrentPos).Clear();
        UpdateCells(mCurrentPos, mCurrentPos);
        emit UpdateDigits(mCurrentPos, mCurrentPos);
      }
      qDecoration->CursorChange(Decoration::eCursorNormalCell);
    } else if (event->button() == Qt::LeftButton) {
      mSpotMark = 1;
      qDecoration->CursorChange(Decoration::eCursorYesCell);
    } else if (event->button() == Qt::RightButton) {
      mSpotMark = -1;
      qDecoration->CursorChange(Decoration::eCursorNoCell);
    }
    mSpotPos = mCurrentPos;
    UpdateCells(mSpotPos, mSpotPos);
  }
}

void TableWidget::mouseReleaseEvent(QMouseEvent* event)
{
  if (!HasSpot()) {
    return;
  }

  if (TranslateCell(event->x(), event->y())) {
    qPuzzle->SetCells(mSpotPos, mCurrentPos, mSpotMark, qEditing->getCurrentPropLevel());
    UpdateCells(mSpotPos, mCurrentPos);
    emit UpdateDigits(mSpotPos, mCurrentPos);
    emit UpdatePreview();
  } else {
    UpdateCells(mSpotPos, mSpotPos);
  }

  mSpotMark = 0;
  qDecoration->CursorChange(Decoration::eCursorArrow);

  if (mShowCalcWindow) {
    mShowCalcWindow = false;
    emit HideCalcWindow();
  }
}

void TableWidget::leaveEvent(QEvent* event)
{
  Q_UNUSED(event);

  qDecoration->HighlightPosChange(QPoint(-1, -1));
}

void TableWidget::Setup()
{
  mCellWidth  = qDecoration->getCellWidth();
  mCellHeight = qDecoration->getCellHeight();

  if (qPuzzle) {
    mWidth  = qDecoration->GetTableWidth();
    mHeight = qDecoration->GetTableHeight();
  } else {
    mWidth = mHeight = 0;
  }

  setMinimumSize(mWidth, mHeight);
  setMaximumSize(mWidth, mHeight);
  resize(mWidth, mHeight);
}

bool TableWidget::TranslateCell(int x, int y)
{
  int i, j;
  if (!qDecoration->TranslatePointToCellX(mCurrentPos, x, i) || !qDecoration->TranslatePointToCellY(mCurrentPos, y, j)) {
    mCurrentPos = QPoint(-1, -1);
    return false;
  }

  mCurrentPos.setX(i);
  mCurrentPos.setY(j);
  return true;
}

void TableWidget::UpdateCells(const QPoint& p1, const QPoint& p2)
{
  int x1 = qDecoration->CellToPointX(qMin(p1.x(), p2.x()));
  int x2 = qDecoration->CellToPointX(qMax(p1.x(), p2.x())) + mCellWidth;
  int y1 = qDecoration->CellToPointY(qMin(p1.y(), p2.y()));
  int y2 = qDecoration->CellToPointY(qMax(p1.y(), p2.y())) + mCellHeight;

  update(x1, y1, x2 - x1, y2 - y1);
}


TableWidget::TableWidget(QWidget* parent)
  : QWidgetB(parent)
  , mSpotMark(0), mShowCalcWindow(false)
{
}

