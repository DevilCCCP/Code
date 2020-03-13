#include <QPaintEvent>
#include <QPainter>

#include "DigitsWidget.h"
#include "Core.h"
#include "Style.h"
#include "Puzzle.h"
#include "Decoration.h"
#include "Account.h"


void DigitsWidget::paintEvent(QPaintEvent* event)
{
  QWidgetB::paintEvent(event);

  if (!qPuzzle) {
    return;
  }

  if (mOrientation == Qt::Horizontal) {
    DrawHorz(event->rect());
  } else {
    DrawVert(event->rect());
  }
}

void DigitsWidget::mouseMoveEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton && mModeSetDigit > 0) {
    SetDigits(event->pos());
  } else if (event->buttons() == Qt::RightButton && mModeSetDigit < 0) {
    SetDigits(event->pos());
  }
}

void DigitsWidget::mousePressEvent(QMouseEvent* event)
{
  if (event->buttons() == Qt::LeftButton) {
    mModeSetDigit = 1;
    mLastDigitPoint = event->pos();

    SetDigits(event->pos());
  } else if (event->buttons() == Qt::RightButton) {
    mModeSetDigit = -1;
    mLastDigitPoint = event->pos();

    SetDigits(event->pos());
  } else {
    mModeSetDigit = 0;
  }
}

void DigitsWidget::mouseReleaseEvent(QMouseEvent* event)
{
  Q_UNUSED(event);

  if (mModeSetDigit) {
    mModeSetDigit = 0;
  }
}

void DigitsWidget::Init(Qt::Orientation _Orientation, Qt::AlignmentFlag _Align)
{
  mOrientation = _Orientation;
  mAlign       = _Align;
}

void DigitsWidget::Setup()
{
  mCellWidth  = qDecoration->getCellWidth();
  mCellHeight = qDecoration->getCellHeight();

  if (!qPuzzle) {
    mWidth = mHeight = 0;
    return;
  }

  if (mOrientation == Qt::Horizontal) {
    mWidth  = (mCellWidth + 1) * qPuzzle->getDigitsHorzMax();
    mHeight = qDecoration->GetTableHeight();
  } else {
    mWidth  = qDecoration->GetTableWidth();
    mHeight = (mCellHeight + 1) * qPuzzle->getDigitsVertMax();
  }

//  setMinimumSize(mWidth, mHeight);
//  setMaximumSize(mWidth, mHeight);
  resize(mWidth, mHeight);
}

void DigitsWidget::SetViewCut(int value)
{
  if (qAccount->getCompactDigits() == 0)  {
    mCompactMode = false;
    mCompactCount = 0;
    return;
  }

  if (mOrientation == Qt::Horizontal) {
    mCompactMode  = value > mCellWidth;
    mCompactCount = (mWidth - value - mCellWidth/2*2) / (mCellWidth + 1);
    mCompactTail  = (mWidth - value - mCellWidth/2*2) % (mCellWidth + 1);
    if (mAlign == Qt::AlignLeft) {
      mCompactStart  = mCellWidth/2;
      mCompactFinish = mCompactStart + mCompactCount * (mCellWidth + 1);
    } else {
      mCompactFinish = mWidth - mCellWidth/2;
      mCompactStart  = mCompactFinish - mCompactCount * (mCellWidth + 1);
    }
  } else {
    mCompactMode  = value > mCellHeight;
    mCompactCount = (mHeight - value - mCellHeight/2*2) / (mCellHeight + 1);
    mCompactTail  = (mHeight - value - mCellHeight/2*2) % (mCellHeight + 1);
    if (mAlign == Qt::AlignTop) {
      mCompactStart  = mCellHeight/2;
      mCompactFinish = mCompactStart + mCompactCount * (mCellHeight + 1);
    } else {
      mCompactFinish = mHeight - mCellHeight/2;
      mCompactStart  = mCompactFinish - mCompactCount * (mCellHeight + 1);
    }
  }

  update();
}

int DigitsWidget::ExpandToMinView()
{
  if (!mCompactMode) {
    return 0;
  }

  int maxDigits = qMin(qAccount->getCompactDigits(), (mOrientation == Qt::Horizontal? qPuzzle->getDigitsHorzMax(): qPuzzle->getDigitsVertMax()));
  if (mCompactCount < maxDigits)  {
    int cellSize = (mOrientation == Qt::Horizontal)? mCellWidth: mCellHeight;
    int expandDigits = maxDigits - mCompactCount;
    int expand = expandDigits * (cellSize + 1) - mCompactTail;
    mCompactCount = maxDigits;
    if (mAlign == Qt::AlignLeft || mAlign == Qt::AlignTop) {
//      mCompactStart  -= mCompactTail;
      mCompactFinish += expand + mCompactTail;
    } else {
      mCompactStart  -= mCompactTail;
      mCompactFinish += expand;
    }
    return expand;
  }
  return 0;
}

void DigitsWidget::UpdateHighlight()
{
  if (mOrientation == Qt::Horizontal) {
    int j = qDecoration->getLastHighlightPos().y();
    if (j >= 0) {
      int y = qDecoration->CellToPointY(j);
      update(QRect(0, y, width(), mCellHeight));
    }
    j = qDecoration->getHighlightPos().y();
    if (j >= 0) {
      int y = qDecoration->CellToPointY(j);
      update(QRect(0, y, width(), mCellHeight));
    }
  } else {
    int i = qDecoration->getLastHighlightPos().x();
    if (i >= 0) {
      int x = qDecoration->CellToPointX(i);
      update(QRect(x, 0, mCellWidth, height()));
    }
    i = qDecoration->getHighlightPos().x();
    if (i >= 0) {
      int x = qDecoration->CellToPointX(i);
      update(QRect(x, 0, mCellWidth, height()));
    }
  }
}

void DigitsWidget::UpdateDigits(const QPoint& p1, const QPoint& p2)
{
  if (mOrientation == Qt::Horizontal) {
    int y1 = qMin(p1.y(), p2.y());
    int y2 = qMax(p1.y(), p2.y());
    int yl = qDecoration->CellToPointY(y1);
    int yh = qDecoration->CellToPointY(y2) + mCellHeight;
    update(QRect(0, yl, width(), yh - yl + 1));
  } else {
    int x1 = qMin(p1.x(), p2.x());
    int x2 = qMax(p1.x(), p2.x());
    int xl = qDecoration->CellToPointX(x1);
    int xh = qDecoration->CellToPointX(x2) + mCellWidth;
    update(QRect(xl, 0, xh - xl + 1, height()));
  }
}

int MaxColor(int color1, int color2)
{
  if (color1 == 2 || color2 == 2) {
    return 2;
  } else if (color1 == 0 || color2 == 0) {
    return 0;
  } else if (color1 == 1 || color2 == 1) {
    return 1;
  } else {
    return -1;
  }
}

void DigitsWidget::DrawHorz(const QRect& rect)
{
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

  if (!qAccount->getCompactDigits() || !mCompactMode) {
    mCompactCount  = qPuzzle->getDigitsHorzMax();
    mCompactStart  = 0;
    mCompactFinish = mWidth;
  }

  painter.setFont(QFont(qStyle->getFont(), mCellHeight/2, mCellWidth/3 - 2));
  for (int j = 0; j < qPuzzle->getHeight(); j++) {
    int y = qDecoration->CellToPointY(j);
    if (y + mCellHeight < rect.top() || y - mCellHeight > rect.bottom()) {
      continue;
    }

    int count = 0;
    for (; count < qPuzzle->getDigitsHorz().size() && qPuzzle->getDigitsHorz().at(count).at(j); count++) {
    }
    int horzDigitStart  = 0;
    int horzDigitFinish = count;
    int colorFront = -1;
    int colorBack  = -1;
    if (mCompactMode) {
      for (int i = horzDigitStart; i < horzDigitFinish; i++) {
        if (qPuzzle->getDigitsMarkHorz().at(i).at(j) != 1) {
          if (i > horzDigitStart) {
            horzDigitStart = i;
            colorFront = 1;
          }
          break;
        }
      }
      for (int i = horzDigitFinish - 1; i >= horzDigitStart; i--) {
        if (qPuzzle->getDigitsMarkHorz().at(i).at(j) != 1) {
          if (i + 1 < horzDigitFinish) {
            horzDigitFinish = i + 1;
            colorBack = 1;
          }
          break;
        }
      }
      if (mAlign == Qt::AlignRight) {
        while (horzDigitFinish - horzDigitStart > mCompactCount) {
          horzDigitFinish--;
          colorBack = MaxColor(colorBack, qPuzzle->getDigitsMarkHorz().at(horzDigitFinish).at(j));
        }
        if (horzDigitFinish - horzDigitStart < mCompactCount) {
          horzDigitStart = horzDigitFinish - mCompactCount;
          if (horzDigitStart <= 0) {
            horzDigitStart = 0;
            colorFront = -1;
          }
        }
        if (horzDigitFinish - horzDigitStart < mCompactCount) {
          horzDigitFinish = mCompactCount;
          if (horzDigitFinish >= count) {
            horzDigitFinish = count;
            colorBack = -1;
          }
        }
      } else {
        while (horzDigitFinish - horzDigitStart > mCompactCount) {
          horzDigitStart++;
          colorFront = MaxColor(colorFront, qPuzzle->getDigitsMarkHorz().at(horzDigitStart - 1).at(j));
        }
        if (horzDigitFinish - horzDigitStart < mCompactCount) {
          horzDigitFinish = horzDigitStart + mCompactCount;
          if (horzDigitFinish >= count) {
            horzDigitFinish = count;
            colorBack = -1;
          }
        }
        if (horzDigitFinish - horzDigitStart < mCompactCount) {
          horzDigitStart = horzDigitFinish - mCompactCount;
          if (horzDigitStart <= 0) {
            horzDigitStart = 0;
            colorFront = -1;
          }
        }
      }
    }

    if (qDecoration->getHighlightPos().y() == j) {
      painter.fillRect(QRect(rect.x(), y, rect.width(), mCellHeight), qStyle->getHighlightColor());
    }
    if (colorFront >= 0) {
      switch (colorFront) {
      case 0:
      default: painter.setPen(qStyle->getDigitWhiteColor()); break;
      case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
      case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
      }
      painter.drawLine(mCompactStart - mCellHeight/8*3, y + mCellHeight/2,
                       mCompactStart - mCellHeight/8, y + mCellHeight/2 - mCellHeight/8*2);
      painter.drawLine(mCompactStart - mCellHeight/8*3, y + mCellHeight/2,
                       mCompactStart - mCellHeight/8, y + mCellHeight/2 + mCellHeight/8*2);
    }
    if (colorBack >= 0) {
      switch (colorBack) {
      case 0:
      default: painter.setPen(qStyle->getDigitWhiteColor()); break;
      case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
      case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
      }
      painter.drawLine(mCompactFinish + mCellHeight/8*3, y + mCellHeight/2,
                       mCompactFinish + mCellHeight/8, y + mCellHeight/2 - mCellHeight/8*2);
      painter.drawLine(mCompactFinish + mCellHeight/8*3, y + mCellHeight/2,
                       mCompactFinish + mCellHeight/8, y + mCellHeight/2 + mCellHeight/8*2);
    }
    if (mAlign == Qt::AlignLeft) {
      for (int i = horzDigitStart; i < horzDigitFinish; i++) {
        int x = mCompactStart + (mCellWidth + 1) * (i - horzDigitStart);
        if (x + mCellWidth < rect.left() || x > rect.right()) {
          continue;
        }

        switch (qPuzzle->getDigitsMarkHorz().at(i).at(j)) {
        case 0:
        default: painter.setPen(qStyle->getDigitWhiteColor()); break;
        case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
        case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
        }
        painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter, QString::number(qPuzzle->getDigitsHorz().at(i).at(j)));
      }
    } else {
      for (int i = horzDigitStart; i < horzDigitFinish; i++) {
        int x = mCompactFinish - (mCellWidth + 1) * (horzDigitFinish - i - 1) - mCellWidth;
        if (x + mCellWidth < rect.left() || x > rect.right()) {
          continue;
        }

        switch (qPuzzle->getDigitsMarkHorz().at(i).at(j)) {
        case 0:
        default: painter.setPen(qStyle->getDigitWhiteColor()); break;
        case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
        case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
        }
        painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter, QString::number(qPuzzle->getDigitsHorz().at(i).at(j)));
      }
    }
  }
}

void DigitsWidget::DrawVert(const QRect& rect)
{
  QPainter painter(this);
  painter.setPen(qStyle->getLineColor());
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

  if (!qAccount->getCompactDigits() || !mCompactMode) {
    mCompactCount  = qPuzzle->getDigitsVertMax();
    mCompactStart  = 0;
    mCompactFinish = mHeight;
  }

  painter.setFont(QFont(qStyle->getFont(), mCellHeight/2, mCellWidth/3 - 2));
  for (int i = 0; i < qPuzzle->getWidth(); i++) {
    int x = qDecoration->CellToPointX(i);
    if (x + mCellWidth < rect.left() || x > rect.right()) {
      continue;
    }

    int count = 0;
    for (; count < qPuzzle->getDigitsVert().at(i).size() && qPuzzle->getDigitsVert().at(i).at(count); count++) {
    }
    int vertDigitStart  = 0;
    int vertDigitFinish = count;
    int colorFront = -1;
    int colorBack  = -1;
    if (mCompactMode) {
      for (int j = vertDigitStart; j < vertDigitFinish; j++) {
        if (qPuzzle->getDigitsMarkVert().at(i).at(j) != 1) {
          if (j > vertDigitStart) {
            vertDigitStart = j;
            colorFront = 1;
          }
          break;
        }
      }
      for (int j = vertDigitFinish - 1; j >= vertDigitStart; j--) {
        if (qPuzzle->getDigitsMarkVert().at(i).at(j) != 1) {
          if (j + 1 < vertDigitFinish) {
            vertDigitFinish = j + 1;
            colorBack = 1;
          }
          break;
        }
      }
      if (mAlign == Qt::AlignBottom) {
        while (vertDigitFinish - vertDigitStart > mCompactCount) {
          vertDigitFinish--;
          colorBack = MaxColor(colorBack, qPuzzle->getDigitsMarkVert().at(i).at(vertDigitFinish));
        }
        if (vertDigitFinish - vertDigitStart < mCompactCount) {
          vertDigitStart = vertDigitFinish - mCompactCount;
          if (vertDigitStart <= 0) {
            vertDigitStart = 0;
            colorFront = -1;
          }
        }
        if (vertDigitFinish - vertDigitStart < mCompactCount) {
          vertDigitFinish = mCompactCount;
          if (vertDigitFinish >= count) {
            vertDigitFinish = count;
            colorBack = -1;
          }
        }
      } else {
        while (vertDigitFinish - vertDigitStart > mCompactCount) {
          vertDigitStart++;
          colorFront = MaxColor(colorFront, qPuzzle->getDigitsMarkVert().at(i).at(vertDigitStart - 1));
        }
        if (vertDigitFinish - vertDigitStart < mCompactCount) {
          vertDigitFinish = vertDigitStart + mCompactCount;
          if (vertDigitFinish >= count) {
            vertDigitFinish = count;
            colorBack = -1;
          }
        }
        if (vertDigitFinish - vertDigitStart < mCompactCount) {
          vertDigitStart = vertDigitFinish - mCompactCount;
          if (vertDigitStart <= 0) {
            vertDigitStart = 0;
            colorFront = -1;
          }
        }
      }
    }

    if (qDecoration->getHighlightPos().x() == i) {
      painter.fillRect(QRect(x, rect.y(), mCellWidth, rect.height()), qStyle->getHighlightColor());
    }
    if (colorFront >= 0) {
      switch (colorFront) {
      case 0:
      default: painter.setPen(qStyle->getDigitWhiteColor()); break;
      case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
      case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
      }
      painter.drawLine(x + mCellWidth/2, mCompactStart - mCellHeight/8*3,
                       x + mCellWidth/2 - mCellWidth/8*2, mCompactStart - mCellHeight/8);
      painter.drawLine(x + mCellWidth/2, mCompactStart - mCellHeight/8*3,
                       x + mCellWidth/2 + mCellWidth/8*2, mCompactStart - mCellHeight/8);
    }
    if (colorBack >= 0) {
      switch (colorBack) {
      case 0:
      default: painter.setPen(qStyle->getDigitWhiteColor()); break;
      case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
      case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
      }
      painter.drawLine(x + mCellWidth/2, mCompactFinish + mCellHeight/8*3,
                       x + mCellWidth/2 - mCellWidth/8*2, mCompactFinish + mCellHeight/8);
      painter.drawLine(x + mCellWidth/2, mCompactFinish + mCellHeight/8*3,
                       x + mCellWidth/2 + mCellWidth/8*2, mCompactFinish + mCellHeight/8);
    }
    if (mAlign == Qt::AlignTop) {
      for (int j = vertDigitStart; j < vertDigitFinish; j++) {
        int y = mCompactStart + (mCellHeight + 1) * (j - vertDigitStart);
        if (y + mCellHeight < rect.top() || y > rect.bottom()) {
          continue;
        }

        switch (qPuzzle->getDigitsMarkVert().at(i).at(j)) {
        case 0:
        default: painter.setPen(qStyle->getDigitWhiteColor()); break;
        case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
        case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
        }
        painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter, QString::number(qPuzzle->getDigitsVert().at(i).at(j)));
      }
    } else {
      for (int j = vertDigitStart; j < vertDigitFinish; j++) {
        int y = mCompactFinish - (mCellHeight + 1) * (vertDigitFinish - 1 - j) - mCellHeight;
        if (y + mCellHeight < rect.top() || y > rect.bottom()) {
          continue;
        }

        switch (qPuzzle->getDigitsMarkVert().at(i).at(j)) {
        case 0:
        default: painter.setPen(qStyle->getDigitWhiteColor()); break;
        case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
        case 2:  painter.setPen(qStyle->getDigitRedColor()); break;
        }
        painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter, QString::number(qPuzzle->getDigitsVert().at(i).at(j)));
      }
    }
  }
}

void DigitsWidget::SetDigits(const QPoint& nextPos)
{
  if (qAccount->getDigitStyle() != Account::eDigitManual) {
    return;
  }

  QPoint p1, p2;
  if (!TranslateDigit(nextPos, p2)) {
    mLastDigitPoint = nextPos;
    return;
  }
  if (!TranslateDigit(mLastDigitPoint, p1)) {
    p1 = p2;
  }

  qPuzzle->SetDigit(mOrientation, p1, p2, mModeSetDigit > 0? 1: 0);
  ChangedDigits((int)mOrientation, p1, p2);
  mLastDigitPoint = nextPos;
}

bool DigitsWidget::TranslateDigit(const QPoint& pos, QPoint& p)
{
  if (mOrientation == Qt::Horizontal) {
    if (!qDecoration->TranslatePointToCellY(mLastDigitPoint, pos.y(), p.ry())) {
      return false;
    }
    int i = pos.x() / (mCellWidth + 1);
    if (i < 0 || i >= qPuzzle->getWidth()) {
      return false;
    }
    p.rx() = i;

    if (mAlign == Qt::AlignRight) {
      int dcount = 0;
      for (int i = 0; i < qPuzzle->getWidth(); i++) {
        if (qPuzzle->getDigitsHorz()[i][p.y()]) {
          dcount++;
        } else {
          break;
        }
      }
      p.rx() -= qPuzzle->getDigitsHorzMax() - dcount;
      return p.x() >= 0;
    }
  } else {
    if (!qDecoration->TranslatePointToCellX(mLastDigitPoint, pos.x(), p.rx())) {
      return false;
    }
    int j = pos.y() / (mCellHeight + 1);
    if (j < 0 || j >= qPuzzle->getHeight()) {
      return false;
    }
    p.ry() = j;

    if (mAlign == Qt::AlignBottom) {
      int dcount = 0;
      for (int j = 0; j < qPuzzle->getHeight(); j++) {
        if (qPuzzle->getDigitsVert()[p.x()][j]) {
          dcount++;
        } else {
          break;
        }
      }
      p.ry() -= qPuzzle->getDigitsVertMax() - dcount;
      return p.y() >= 0;
    }
  }
  return true;
}


DigitsWidget::DigitsWidget(QWidget* parent)
  : QWidgetB(parent)
  , mCompactMode(false)
{
}
