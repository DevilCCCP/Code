#include <QPainter>

#include "FormCalcPopup.h"
#include "ui_FormCalcPopup.h"
#include "Core.h"
#include "Decoration.h"
#include "Style.h"
#include "Puzzle.h"
#include "Account.h"


void FormCalcPopup::paintEvent(QPaintEvent* event)
{
  QWidgetB::paintEvent(event);

  switch (qAccount->getCalcWindow()) {
  case Account::eCalcWindowNone  : break;
  case Account::eCalcWindowSimple: DrawSimple(); break;
  case Account::eCalcWindowSmart : DrawSmart(); break;
  }
}

void FormCalcPopup::Setup()
{
  mCellWidth  = qDecoration->getCellWidth();
  mCellHeight = qDecoration->getCellHeight();
}

void FormCalcPopup::DrawSimple()
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(qStyle->getDigitWhiteColor());
  painter.drawRect(QRect(0, 0, width(), height()));
  painter.setFont(QFont(qStyle->getFont(), mCellHeight/2, mCellWidth/3 - 2));


  for (int j = 0; j < mDigits.size(); j++) {
    int y = 1 + (mCellHeight + 1) * j;
    const Line* digits = &mDigits.at(j);
    for (int i = 0; i < digits->size(); i++) {
      int x = 1 + (mCellWidth + 1) * i;
      painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter
                       , digits->at(i) >= 0? QString::number(digits->at(i)): QString("x"));
    }
  }
}

void FormCalcPopup::DrawSmart()
{
  QPainter painter(this);
  painter.setRenderHint(QPainter::Antialiasing);

  painter.setPen(qStyle->getDigitWhiteColor());
  painter.drawRect(QRect(0, 0, width(), height()));
  painter.setFont(QFont(qStyle->getFont(), mCellHeight/2, mCellWidth/3 - 2));

  switch (mOrientation) {
  case Qt::Horizontal:
    for (int j = 0; j < mDigits.size(); j++) {
      int y = 1 + (mCellHeight + 1) * j;
      const Line* digits = &mDigits.at(j);
      const Line* digitsColors = &mDigitsColors.at(j);
      for (int i = 0; i < digits->size(); i++) {
        int x = (mAlign == Qt::AlignLeft)? 1 + (mCellWidth + 1) * i: width() - (1 + (mCellWidth + 1) * i) - mCellWidth;

        switch (digitsColors->at(i)) {
        case 0:  painter.setPen(qStyle->getDigitWhiteColor()); break;
        case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
        default: painter.setPen(qStyle->getDigitRedColor()); break;
        }
        painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter
                         , digits->at(i) > 0? QString::number(digits->at(i)): QString("*"));
      }
    }
    break;

  case Qt::Vertical:
    for (int i = 0; i < mDigits.size(); i++) {
      int x = 1 + (mCellWidth + 1) * i;
      const Line* digits = &mDigits.at(i);
      const Line* digitsColors = &mDigitsColors.at(i);
      for (int j = 0; j < digits->size(); j++) {
        int y = (mAlign == Qt::AlignTop)? 1 + (mCellHeight + 1) * j: height() - (1 + (mCellHeight + 1) * j) - mCellHeight;

        switch (digitsColors->at(j)) {
        case 0:  painter.setPen(qStyle->getDigitWhiteColor()); break;
        case 1:  painter.setPen(qStyle->getDigitBlackColor()); break;
        default: painter.setPen(qStyle->getDigitRedColor()); break;
        }
        painter.drawText(x, y, mCellWidth, mCellHeight, Qt::AlignCenter
                         , digits->at(j) > 0? QString::number(digits->at(j)): QString("*"));
      }
    }
    break;
  }
}

void FormCalcPopup::CalcHorzYes(const QPoint& p1, const QPoint& p2)
{
  int digitWidth = 0;
  if (p1.x() < p2.x()) {
    mAlign = Qt::AlignLeft;
    int y1 = qMin(p1.y(), p2.y());
    int y2 = qMax(p1.y(), p2.y());
    for (int j = y1; j <= y2; j++) {
      int count = 0;
      bool allMarked = true;
      for (int i = p1.x(); i >= 0; i--) {
        if (qPuzzle->At(i, j).IsMarkNo()) {
          for (i--; i >= 0; i--) {
            if (!qPuzzle->At(i, j).HasMark()) {
              allMarked = false;
              break;
            }
          }
          break;
        }
        count++;
      }
      count += p2.x() - p1.x();

      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      if (allMarked) {
        int iStart = qPuzzle->getWidth();
        for (int i = 0; i < qPuzzle->getWidth(); i++) {
          if (qPuzzle->getDigitsMarkHorz()[i][j] == 0) {
            iStart = i;
            break;
          }
        }

        for (int i = iStart; count >= 0; i++) {
          if (i >= qPuzzle->getWidth() || qPuzzle->getDigitsHorz()[i][j] <= 0 || qPuzzle->getDigitsMarkHorz()[i][j] != 0) {
            if (!digits->isEmpty() && digits->last() == 0) {
              digitsColors->last() = 2;
            }
            break;
          }
          if (count == 0) {
            break;
          }
          int d = qMin(qPuzzle->getDigitsHorz()[i][j], count);
          digits->append(d);
          digitsColors->append(qPuzzle->getDigitsHorz()[i][j] == d? 1: 0);
          count -= d;
          if (count > 0) {
            digits->append(0);
            digitsColors->append(1);
            count--;
          }
        }
        if (count > 0) {
          digits->append(count);
          digitsColors->append(2);
        }
      } else {
        digits->append(count);
        digitsColors->append(0);
      }
      digitWidth = qMax(digitWidth, digits->size());
    }
  } else if (p1.x() > p2.x()) {
    mAlign = Qt::AlignRight;
    int y1 = qMin(p1.y(), p2.y());
    int y2 = qMax(p1.y(), p2.y());
    for (int j = y1; j <= y2; j++) {
      int count = 0;
      bool allMarked = true;
      for (int i = p1.x(); i < qPuzzle->getWidth(); i++) {
        if (qPuzzle->At(i, j).IsMarkNo()) {
          for (i++; i < qPuzzle->getWidth(); i++) {
            if (!qPuzzle->At(i, j).HasMark()) {
              allMarked = false;
              break;
            }
          }
          break;
        }
        count++;
      }
      count += p1.x() - p2.x();

      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      if (allMarked) {
        int iStart = -1;
        for (int i = qPuzzle->getWidth() - 1; i >= 0; i--) {
          if (qPuzzle->getDigitsHorz()[i][j] > 0) {
            for (; i >= 0; i--) {
              if (qPuzzle->getDigitsMarkHorz()[i][j] == 0) {
                iStart = i;
                break;
              }
            }
            break;
          }
        }

        for (int i = iStart; count >= 0; i--) {
          if (i < 0 || qPuzzle->getDigitsHorz()[i][j] <= 0 || qPuzzle->getDigitsMarkHorz()[i][j] != 0) {
            if (!digits->isEmpty() && digits->last() == 0) {
              digitsColors->last() = 2;
            }
            break;
          }
          if (count == 0) {
            break;
          }
          int d = qMin(qPuzzle->getDigitsHorz()[i][j], count);
          digits->append(d);
          digitsColors->append(qPuzzle->getDigitsHorz()[i][j] == d? 1: 0);
          count -= d;
          if (count > 0) {
            digits->append(0);
            digitsColors->append(1);
            count--;
          }
        }
        if (count > 0) {
          digits->append(count);
          digitsColors->append(2);
        }
      } else {
        digits->append(count);
        digitsColors->append(0);
      }
      digitWidth = qMax(digitWidth, digits->size());
    }
  }
  int digitHeight = mDigits.size();
  resize(digitWidth * (mCellWidth + 1) + 2, digitHeight * (mCellHeight + 1) + 2);
}

void FormCalcPopup::CalcVertYes(const QPoint& p1, const QPoint& p2)
{
  int digitHeight = 0;
  if (p1.y() < p2.y()) {
    mAlign = Qt::AlignTop;
    int x1 = qMin(p1.x(), p2.x());
    int x2 = qMax(p1.x(), p2.x());
    for (int i = x1; i <= x2; i++) {
      int count = 0;
      bool allMarked = true;
      for (int j = p1.y(); j >= 0; j--) {
        if (qPuzzle->At(i, j).IsMarkNo()) {
          for (j--; j >= 0; j--) {
            if (!qPuzzle->At(i, j).HasMark()) {
              allMarked = false;
              break;
            }
          }
          break;
        }
        count++;
      }
      count += p2.y() - p1.y();

      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      if (allMarked) {
        int jStart = qPuzzle->getHeight();
        for (int j = 0; j < qPuzzle->getHeight(); j++) {
          if (qPuzzle->getDigitsMarkVert()[i][j] == 0) {
            jStart = j;
            break;
          }
        }

        for (int j = jStart; count >= 0; j++) {
          if (j >= qPuzzle->getHeight() || qPuzzle->getDigitsVert()[i][j] <= 0 || qPuzzle->getDigitsMarkVert()[i][j] != 0) {
            if (!digits->isEmpty() && digits->last() == 0) {
              digitsColors->last() = 2;
            }
            break;
          }
          if (count == 0) {
            break;
          }
          int d = qMin(qPuzzle->getDigitsVert()[i][j], count);
          digits->append(d);
          digitsColors->append(qPuzzle->getDigitsVert()[i][j] == d? 1: 0);
          count -= d;
          if (count > 0) {
            digits->append(0);
            digitsColors->append(1);
            count--;
          }
        }
        if (count > 0) {
          digits->append(count);
          digitsColors->append(2);
        }
      } else {
        digits->append(count);
        digitsColors->append(0);
      }
      digitHeight = qMax(digitHeight, digits->size());
    }
  } else if (p1.y() > p2.y()) {
    mAlign = Qt::AlignBottom;
    int x1 = qMin(p1.x(), p2.x());
    int x2 = qMax(p1.x(), p2.x());
    for (int i = x1; i <= x2; i++) {
      int count = 0;
      bool allMarked = true;
      for (int j = p1.y(); j < qPuzzle->getHeight(); j++) {
        if (qPuzzle->At(i, j).IsMarkNo()) {
          for (j++; j < qPuzzle->getHeight(); j++) {
            if (!qPuzzle->At(i, j).HasMark()) {
              allMarked = false;
              break;
            }
          }
          break;
        }
        count++;
      }
      count += p1.y() - p2.y();

      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      if (allMarked) {
        int jStart = -1;
        for (int j = qPuzzle->getHeight() - 1; j >= 0; j--) {
          if (qPuzzle->getDigitsVert()[i][j] > 0) {
            for (; j >= 0; j--) {
              if (qPuzzle->getDigitsMarkVert()[i][j] == 0) {
                jStart = j;
                break;
              }
            }
            break;
          }
        }

        for (int j = jStart; count >= 0; j--) {
          if (j < 0 || qPuzzle->getDigitsVert()[i][j] <= 0 || qPuzzle->getDigitsMarkVert()[i][j] != 0) {
            if (!digits->isEmpty() && digits->last() == 0) {
              digitsColors->last() = 2;
            }
            break;
          }
          if (count == 0) {
            break;
          }
          int d = qMin(qPuzzle->getDigitsVert()[i][j], count);
          digits->append(d);
          digitsColors->append(qPuzzle->getDigitsVert()[i][j] == d? 1: 0);
          count -= d;
          if (count > 0) {
            digits->append(0);
            digitsColors->append(1);
            count--;
          }
        }
        if (count > 0) {
          digits->append(count);
          digitsColors->append(2);
        }
      } else {
        digits->append(count);
        digitsColors->append(0);
      }
      digitHeight = qMax(digitHeight, digits->size());
    }
  }
  int digitWidth = mDigits.size();
  resize(digitWidth * (mCellWidth + 1) + 2, digitHeight * (mCellHeight + 1) + 2);
}

void FormCalcPopup::CalcHorzNo(const QPoint& p1, const QPoint& p2)
{
  int digitWidth = 1;
  if (p1.x() < p2.x()) {
    mAlign = Qt::AlignLeft;
    int y1 = qMin(p1.y(), p2.y());
    int y2 = qMax(p1.y(), p2.y());
    for (int j = y1; j <= y2; j++) {
      int count = 0;
      if (!qPuzzle->At(p2.x(), j).HasMark()) {
        for (int i = p2.x() + 1; i < qPuzzle->getWidth(); i++) {
          if (qPuzzle->At(i, j).IsMarkNo()) {
            break;
          } else if (qPuzzle->At(i, j).IsMarkYes()) {
            for (; i < qPuzzle->getWidth() && qPuzzle->At(i, j).IsMarkYes(); i++) {
              count++;
            }
            break;
          }
          count++;
        }
      }
      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      digits->append(count);
      digitsColors->append(0);
    }
  } else {
    mAlign = Qt::AlignRight;
    int y1 = qMin(p1.y(), p2.y());
    int y2 = qMax(p1.y(), p2.y());
    for (int j = y1; j <= y2; j++) {
      int count = 0;
      if (!qPuzzle->At(p2.x(), j).HasMark()) {
        for (int i = p2.x() - 1; i >= 0; i--) {
          if (qPuzzle->At(i, j).IsMarkNo()) {
            break;
          } else if (qPuzzle->At(i, j).IsMarkYes()) {
            for (; i >= 0 && qPuzzle->At(i, j).IsMarkYes(); i--) {
              count++;
            }
            break;
          }
          count++;
        }
      }
      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      digits->append(count);
      digitsColors->append(0);
    }
  }
  int digitHeight = mDigits.size();
  resize(digitWidth * (mCellWidth + 1) + 2, digitHeight * (mCellHeight + 1) + 2);
}

void FormCalcPopup::CalcVertNo(const QPoint& p1, const QPoint& p2)
{
  int digitHeight = 1;
  if (p1.y() < p2.y()) {
    mAlign = Qt::AlignTop;
    int x1 = qMin(p1.x(), p2.x());
    int x2 = qMax(p1.x(), p2.x());
    for (int i = x1; i <= x2; i++) {
      int count = 0;
      if (!qPuzzle->At(i, p2.y()).HasMark()) {
        for (int j = p2.y() + 1; j < qPuzzle->getHeight(); j++) {
          if (qPuzzle->At(i, j).IsMarkNo()) {
            break;
          } else if (qPuzzle->At(i, j).IsMarkYes()) {
            for (; j < qPuzzle->getHeight() && qPuzzle->At(i, j).IsMarkYes(); j++) {
              count++;
            }
            break;
          }
          count++;
        }
      }
      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      digits->append(count);
      digitsColors->append(0);
    }
  } else {
    mAlign = Qt::AlignBottom;
    int x1 = qMin(p1.x(), p2.x());
    int x2 = qMax(p1.x(), p2.x());
    for (int i = x1; i <= x2; i++) {
      int count = 0;
      if (!qPuzzle->At(i, p2.y()).HasMark()) {
        for (int j = p2.y() - 1; j >= 0; j--) {
          if (qPuzzle->At(i, j).IsMarkNo()) {
            break;
          } else if (qPuzzle->At(i, j).IsMarkYes()) {
            for (; j >= 0 && qPuzzle->At(i, j).IsMarkYes(); j--) {
              count++;
            }
            break;
          }
          count++;
        }
      }
      mDigits.append(Line());
      mDigitsColors.append(Line());
      Line* digits = &mDigits.back();
      Line* digitsColors = &mDigitsColors.back();
      digits->append(count);
      digitsColors->append(0);
    }
  }
  int digitWidth = mDigits.size();
  resize(digitWidth * (mCellWidth + 1) + 2, digitHeight * (mCellHeight + 1) + 2);
}

void FormCalcPopup::OnShow(const QPoint& pos, const QPoint& p1, const QPoint& p2, int mark, int flag)
{
  if (qAccount->getCalcWindow() == Account::eCalcWindowNone) {
    hide();
    return;
  }

  QPoint newPos = pos;
  mDigits.clear();
  mDigitsColors.clear();
  mAlign = (Qt::Alignment)flag;
  switch (mAlign) {
  case Qt::AlignRight :
  case Qt::AlignLeft  : mOrientation = Qt::Horizontal; break;
  case Qt::AlignBottom:
  case Qt::AlignTop   : mOrientation = Qt::Vertical; break;
  default             : return OnHide();
  }

  newPos.rx() += 1 * (mCellWidth + 2);
  if (qAccount->getCalcWindow() == Account::eCalcWindowSmart) {
    if (mark > 0) {
      mOrientation == Qt::Horizontal? CalcHorzYes(p1, p2): CalcVertYes(p1, p2);
      move(newPos);
    } else if (mark < 0) {
      mOrientation == Qt::Horizontal? CalcHorzNo(p1, p2): CalcVertNo(p1, p2);
      move(newPos);
    } else {
      return OnHide();
    }
  } else if (qAccount->getCalcWindow() == Account::eCalcWindowSimple) {
    bool hasX = p1.x() != p2.x();
    bool hasY = p1.y() != p2.y();

    mDigits.clear();
    mDigits.append(Line());
    if (hasX && hasY) {
      mDigits.first().append(qAbs(p1.x() - p2.x()) + 1);
      mDigits.first().append(-1);
      mDigits.first().append(qAbs(p1.y() - p2.y()) + 1);
    } else if (hasX) {
      mDigits.first().append(qAbs(p1.x() - p2.x()) + 1);
    } else if (hasY) {
      mDigits.first().append(qAbs(p1.y() - p2.y()) + 1);
    }
    int count = mDigits.first().size();
    resize(count * (mCellWidth + 1) + 2, (count? 1: 0) * (mCellHeight + 1) + 2);
    move(newPos);
  }

  show();
  update();
}

void FormCalcPopup::OnHide()
{
  hide();
}


FormCalcPopup::FormCalcPopup(QWidget* parent)
  : QWidgetB(parent, Qt::ToolTip), ui(new Ui::FormCalcPopup)
{
  ui->setupUi(this);
}

FormCalcPopup::~FormCalcPopup()
{
  delete ui;
}
