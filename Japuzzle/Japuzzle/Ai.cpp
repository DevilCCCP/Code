#include <QDateTime>

#include "Ai.h"
#include "Core.h"
#include "Puzzle.h"
#include "Account.h"
#include "Cell.h"


Ai* Ai::mSelf = nullptr;
const int kPropUpdatePeriodMs = 500;

void Ai::CalcAllDigits(Puzzle* puzzle)
{
  if (qAccount->getDigitStyle() == Account::eDigitManual) {
    return;
  }

  for (int j = 0; j < puzzle->getHeight(); j++) {
    CalcHorzDigits(puzzle, j);
  }
  for (int i = 0; i < puzzle->getWidth(); i++) {
    CalcVertDigits(puzzle, i);
  }
}

void Ai::CalcHorzDigits(Puzzle* puzzle, int j)
{
  Cells line(puzzle->getWidth());
  QVector<int> digits(puzzle->getWidth());
  QVector<int> digitsMark(puzzle->getWidth());
  for (int i = 0; i < puzzle->getWidth(); i++) {
    line[i] = puzzle->At(i, j);
    digits[i] = puzzle->mDigitsHorz[i][j];
  }

  if (qAccount->getDigitStyle() == Account::eDigitAuto) {
    CalcDigitsSimple(line, digits, digitsMark);
  } else if (qAccount->getDigitStyle() == Account::eDigitSmart) {
    CalcDigitsSmart(line, digits, digitsMark);
  } else {
    Q_ASSERT(0);
  }

  for (int i = 0; i < puzzle->getWidth(); i++) {
    puzzle->mDigitsMarkHorz[i][j] = digitsMark[i];
  }
}

void Ai::CalcVertDigits(Puzzle* puzzle, int i)
{
  Cells line(puzzle->getHeight());
  QVector<int> digits(puzzle->getHeight());
  QVector<int> digitsMark(puzzle->getHeight());
  for (int j = 0; j < puzzle->getHeight(); j++) {
    line[j] = puzzle->At(i, j);
    digits[j] = puzzle->mDigitsVert[i][j];
  }

  if (qAccount->getDigitStyle() == Account::eDigitAuto) {
    CalcDigitsSimple(line, digits, digitsMark);
  } else if (qAccount->getDigitStyle() == Account::eDigitSmart) {
    CalcDigitsSmart(line, digits, digitsMark);
  } else {
    Q_ASSERT(0);
  }

  for (int j = 0; j < puzzle->getHeight(); j++) {
    puzzle->mDigitsMarkVert[i][j] = digitsMark[j];
  }
}

bool Ai::Test(Puzzle* puzzle)
{
  mCurrentPuzzle = puzzle;
  mCurrentLevel  = 0;

  Cells line(mCurrentPuzzle->getWidth());
  QVector<int> digits(mCurrentPuzzle->getWidth());
  for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      line[i] = mCurrentPuzzle->At(i, j);
      digits[i] = mCurrentPuzzle->mDigitsHorz[i][j];
    }

    if (!LineSolve(line, digits)) {
      return false;
    }
  }

  line.resize(mCurrentPuzzle->getHeight());
  digits.resize(mCurrentPuzzle->getHeight());
  for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
      line[j] = mCurrentPuzzle->At(i, j);
      digits[j] = mCurrentPuzzle->mDigitsVert[i][j];
    }

    if (!LineSolve(line, digits)) {
      return false;
    }
  }
  return true;

//    QVector<int> colors(mCurrentPuzzle->getWidth());
//    CalcDigitsSmart(line, digits, colors);

//    LineDigits lineDigits;
//    if (LineMakeRight(line, digits, lineDigits)) {
//      for (int i = 0; i < lineDigits.size(); i++) {
//        if (lineDigits[i].Left >= 0) {
//          for (int ii = lineDigits[i].Left; ii <= lineDigits[i].Right; ii++) {
//            mCurrentPuzzle->Value(ii, j).SetMark(1, 1);
//          }
//        }
//      }
//    }

//  int quality = 0;
//  Cells line(mCurrentPuzzle->getWidth());
//  for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
//    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
//      line[i] = mCurrentPuzzle->At(i, j);
//    }
//    SolveLine(line);
//  }

//  line.resize(mCurrentPuzzle->getHeight());
//  for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
//    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
//      line[j] = mCurrentPuzzle->At(i, j);
//    }
//  }
//  return true;
}

bool Ai::Hint(Puzzle* puzzle, int level, bool& hasSolve)
{
  mCurrentPuzzle = puzzle;
  mCurrentLevel  = level;
  mCurrentPuzzle->MakeUndo();

  int bestPrice = 0;
  int bestLine = 0;
  bool bestIsHorz = false;

  Cells line(mCurrentPuzzle->getWidth());
  QVector<int> digits(mCurrentPuzzle->getWidth());
  for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      line[i] = mCurrentPuzzle->At(i, j);
      digits[i] = mCurrentPuzzle->mDigitsHorz[i][j];
    }

    if (!LineSolve(line, digits)) {
      for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
        if (!mCurrentPuzzle->Value(i, j).HasMark() && line[i].HasMark()) {
          mCurrentPuzzle->Value(i, j) = line[i];
        }
      }
      return false;
    }

    int price = 0;
    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      if (!mCurrentPuzzle->Value(i, j).HasMark() && line[i].HasMark()) {
        price += HintCalcCellPrice(i, j, line[i].IsMarkYes());
      }
    }
    if (price > bestPrice) {
      bestPrice = price;
      bestLine = j;
      bestIsHorz = true;
    }
  }

  line.resize(mCurrentPuzzle->getHeight());
  digits.resize(mCurrentPuzzle->getHeight());
  for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
      line[j] = mCurrentPuzzle->At(i, j);
      digits[j] = mCurrentPuzzle->mDigitsVert[i][j];
    }

    if (!LineSolve(line, digits)) {
      for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
        if (!mCurrentPuzzle->Value(i, j).HasMark() && line[j].HasMark()) {
          mCurrentPuzzle->Value(i, j) = line[j];
        }
      }
      return false;
    }

    int price = 0;
    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
      if (!mCurrentPuzzle->Value(i, j).HasMark() && line[j].HasMark()) {
        price += HintCalcCellPrice(i, j, line[j].IsMarkYes());
      }
    }
    if (price > bestPrice) {
      bestPrice = price;
      bestLine = i;
      bestIsHorz = false;
    }
  }

  hasSolve = bestPrice > 0;
  if (hasSolve) {
    if (bestIsHorz) {
      int j = bestLine;

      line.resize(mCurrentPuzzle->getWidth());
      digits.resize(mCurrentPuzzle->getWidth());
      for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
        line[i] = mCurrentPuzzle->At(i, j);
        digits[i] = mCurrentPuzzle->mDigitsHorz[i][j];
      }

      if (!LineSolve(line, digits)) {
        return false;
      }

      for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
        if (!mCurrentPuzzle->Value(i, j).HasMark() && line[i].HasMark()) {
          mCurrentPuzzle->Value(i, j) = line[i];
        }
      }
    } else {
      int i = bestLine;

      for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
        line[j] = mCurrentPuzzle->At(i, j);
        digits[j] = mCurrentPuzzle->mDigitsVert[i][j];
      }

      if (!LineSolve(line, digits)) {
        return false;
      }

      for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
        if (!mCurrentPuzzle->Value(i, j).HasMark() && line[j].HasMark()) {
          mCurrentPuzzle->Value(i, j) = line[j];
        }
      }
    }
  }
  return true;
}

void Ai::Solve(Puzzle* puzzle, int level, int maxProp)
{
  mCurrentPuzzle = puzzle;
  mCurrentLevel  = level;
  mPuzzleSolved  = mCurrentPuzzle->Count();
  mPropCommited  = 0;
  mStop          = false;

  SolveDoAll();
  emit SolveChanged(mPuzzleSolved, mPropCommited);

  if (mSolveResult == 0 && maxProp > 0) {
    SolvePrepareProp();
    for (; mPropCommited < maxProp; mPropCommited++) {
      if (!SolveDoProp() || mStop) {
        break;
      }
      emit SolveChanged(mPuzzleSolved, mPropCommited);

      SolveUpdatePropInfo();
    }
  }

  QByteArray solveInfo;
  mCurrentPuzzle->ToByteArray(solveInfo);
  emit SolveInfo(solveInfo);

  emit SolveDone(mSolveResult, mPropCommited);
}

void Ai::Stars(Puzzle* puzzle)
{
  const int kIgnoreCellCount = 8;
  const int kEasyDigitsPercent = 20;
  const int kPropMax3 = 2;
  const int kPropMax4 = 20;

  mCurrentPuzzle = puzzle;
  mCurrentLevel  = 0;
  mPuzzleSolved  = mCurrentPuzzle->Count();
  mPropCommited  = 0;
  mStop          = false;

  SolveDoAll();
  int totalCells = mCurrentPuzzle->Size();

  if (mPuzzleSolved >= totalCells - kIgnoreCellCount) {
    int digitsCount = 0;
    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
      for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
        if (mCurrentPuzzle->mDigitsHorz[i][j] > 0) {
          digitsCount++;
        } else {
          break;
        }
      }
    }
    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
        if (mCurrentPuzzle->mDigitsVert[i][j] > 0) {
          digitsCount++;
        } else {
          break;
        }
      }
    }

    puzzle->SetStars(digitsCount > totalCells * kEasyDigitsPercent/100? 2: 1);
    return;
  }

  SolvePrepareProp();
  for (; mPropCommited <= kPropMax4; mPropCommited++) {
    if (!SolveDoProp() || mStop) {
      break;
    }
  }

  if (mPropCommited <= kPropMax3) {
    puzzle->SetStars(3);
  } else if (mPropCommited <= kPropMax4) {
    puzzle->SetStars(4);
  } else {
    puzzle->SetStars(5);
  }
}

void Ai::CalcDigitsSimple(const Cells& line, const QVector<int>& digits, QVector<int>& colors)
{
  colors.fill(0);
  int currentCount = 0;
  int currentDigit = 0;
  int left = -1;
  int right = line.size() - 1;
  for (int i = 0; i < line.size(); i++) {
    const Cell& cell = line[i];
    if (!cell.HasMark()) {
      left = i - currentCount;
      break;
    }
    if (cell.IsMarkYes()) {
      currentCount++;
    } else {
      if (currentCount > 0) {
        if (!digits[currentDigit]) {
          colors.fill(2);
          return;
        } else if (digits[currentDigit] != currentCount) {
          colors[currentDigit] = 2;
        } else if (!colors[currentDigit]) {
          colors[currentDigit] = 1;
        } else {
          colors[currentDigit] = 2;
        }
        currentDigit++;
      }
      currentCount = 0;
    }

    if (i == line.size() - 1) { // full line
      if (cell.IsMarkYes()) {
        if (digits[currentDigit] != currentCount) {
          colors[currentDigit] = 2;
        } else if (!colors[currentDigit]) {
          colors[currentDigit] = 1;
        } else {
          colors[currentDigit] = 2;
        }
        currentDigit++;
      }
      if (digits[currentDigit]) {
        colors.fill(2);
        return;
      }
    }
  }

  if (left < 0) {
    return;
  }

  currentCount = 0;
  for (currentDigit = line.size() - 1; currentDigit > 0; currentDigit--) {
    if (digits[currentDigit]) {
      break;
    }
  }
  for (int i = line.size() - 1; i >= 0; i--) {
    const Cell& cell = line[i];
    if (!cell.HasMark()) {
      right = i + currentCount;
      break;
    }
    if (cell.IsMarkYes()) {
      currentCount++;
    } else {
      if (currentCount > 0) {
        if (currentDigit < 0) {
          colors.fill(2);
          return;
        } else if (digits[currentDigit] != currentCount) {
          colors[currentDigit] = 2;
        } else if (!colors[currentDigit]) {
          colors[currentDigit] = 1;
        } else {
          colors[currentDigit] = 2;
        }
        currentDigit--;
      }
      currentCount = 0;
    }
  }

  bool allDigits = true;
  for (int i = 0; i < line.size(); i++) {
    if (digits.at(i) && colors.at(i) != 1) {
      allDigits = false;
      break;
    }
  }

  if (allDigits) {
    for (int i = left; i <= right; i++) {
      const Cell& cell = line[i];
      if (cell.IsMarkYes()) {
        colors.fill(2);
        return;
      }
    }
  }
}

void Ai::CalcDigitsSmart(const Cells& line, const QVector<int>& digits, QVector<int>& colors)
{
  LineDigits lineDigitsLeft;
  LineDigits lineDigitsRight;
  if (LineMakeLeft(line, digits, lineDigitsLeft)
      && LineMakeRight(line, digits, lineDigitsRight)) {
    for (int i = 0; i < lineDigitsLeft.size(); i++) {
      if (lineDigitsLeft[i].Left < 0 || lineDigitsRight[i].Left < 0) {
        colors[i] = 2;
        continue;
      }
      colors[i] = 0;
      if (lineDigitsLeft[i].Left == lineDigitsRight[i].Left) {
        int  left = lineDigitsLeft[i].Left;
        int right = lineDigitsLeft[i].Right;
        if (left > 0 && !line.at(left - 1).IsMarkNo()) {
          continue;
        }
        if (right < line.size() - 1 && !line.at(right + 1).IsMarkNo()) {
          continue;
        }
        bool ok = true;
        for (int ii = left; ii <= right; ii++) {
          if (!line.at(ii).IsMarkYes()) {
            ok = false;
            break;
          }
        }
        if (ok) {
          colors[i] = 1;
        }
      }
    }
  } else {
    CalcDigitsSimple(line, digits, colors);
    for (int i = 0; i < colors.size(); i++) {
      if (colors[i] == 0) {
        colors[i] = 2;
      }
    }
  }
}

bool Ai::LineSolve(Cells& line, const QVector<int>& digits)
{
  if (digits.isEmpty() || digits.first() == 0) {
    for (int i = 0; i < line.size(); i++) {
      if (!line[i].HasMark()) {
        line[i].SetMark(-1, mCurrentLevel);
      } else if (line[i].IsMarkYes()) {
        return false;
      }
    }
    return true;
  }

  LineDigits lineDigitsLeft;
  LineDigits lineDigitsRight;
  if (!LineMakeLeft(line, digits, lineDigitsLeft)
      || !LineMakeRight(line, digits, lineDigitsRight)) {
    return false;
  }

  for (int curDigit = 0; curDigit < lineDigitsLeft.size(); curDigit++) {
    if (lineDigitsLeft[curDigit].Left < 0 || lineDigitsRight[curDigit].Left < 0) {
      return false;
    }

    int leftYes  = qMax(lineDigitsLeft[curDigit].Left, lineDigitsRight[curDigit].Left);
    int rightYes = qMin(lineDigitsLeft[curDigit].Right, lineDigitsRight[curDigit].Right);
    for (int i = leftYes; i <= rightYes; i++) {
      if (!line[i].HasMark()) {
        line[i].SetMark(1, mCurrentLevel);
      }
    }
  }

  for (int i = 0; i <= lineDigitsLeft.first().Left - 1; i++) {
    if (!line[i].HasMark()) {
      line[i].SetMark(-1, mCurrentLevel);
    }
  }
  for (int i = lineDigitsRight.last().Right + 1; i < line.size(); i++) {
    if (!line[i].HasMark()) {
      line[i].SetMark(-1, mCurrentLevel);
    }
  }
  if (lineDigitsLeft.size() < 2) {
    return true;
  }

  for (int curDigit = 0; curDigit < lineDigitsLeft.size(); curDigit++) {
    int left  = lineDigitsLeft.at(curDigit).Right + 1;
    int right = curDigit < lineDigitsLeft.size() - 1? lineDigitsLeft.at(curDigit + 1).Left - 1: lineDigitsRight.last().Left - 1;
    for (int i = left; i <= right; i++) {
      bool fail = (lineDigitsRight.at(curDigit).Left <= i && i <= lineDigitsRight.at(curDigit).Right)
          || lineDigitsRight.at(curDigit).Left > i;
      if (!fail && !line[i].HasMark()) {
        line[i].SetMark(-1, mCurrentLevel);
      }
    }
  }
//  for (int curDigit = 0; curDigit <= lineDigitsLeft.size(); curDigit++) {
//    int leftNo  = (curDigit > 0)? lineDigitsRight[curDigit - 1].Right + 1: 0;
//    int rightNo = (curDigit < lineDigitsLeft.size() - 1)? lineDigitsLeft[curDigit + 1].Left - 1: line.size() - 1;
//    for (int i = leftNo; i <= rightNo; i++) {
//      if (!line[i].HasMark()) {
//        line[i].SetMark(-1, mCurrentLevel);
//      }
//    }

//  }
  return true;
}

bool Ai::LineMakeLeft(const Cells& line, const QVector<int>& digits, Ai::LineDigits& lineDigits)
{
  mCurrentLine       = &line;
  mCurrentDigits     = &digits;
  mCurrentLineDigits = &lineDigits;

  int lineSize  = mCurrentLine->size();
  int digitSize = 0;
  for (; digitSize < mCurrentDigits->size() && mCurrentDigits->at(digitSize); ) {
    digitSize++;
  }
  mCurrentLineDigits->resize(digitSize);
  mCurrentInc = 1;

  return LineMakeFull(0, lineSize, 0, digitSize);
}

bool Ai::LineMakeRight(const Cells& line, const QVector<int>& digits, Ai::LineDigits& lineDigits)
{
  mCurrentLine       = &line;
  mCurrentDigits     = &digits;
  mCurrentLineDigits = &lineDigits;

  int lineSize  = mCurrentLine->size();
  int digitSize = 0;
  for (; digitSize < mCurrentDigits->size() && mCurrentDigits->at(digitSize); ) {
    digitSize++;
  }
  mCurrentLineDigits->resize(digitSize);
  mCurrentInc = -1;

  return LineMakeFull(lineSize - 1, -1, digitSize - 1, -1);
}

bool Ai::LineMakeFull(int lineBegin, int lineEnd, int digitBegin, int digitEnd)
{
  if (mCurrentInc > 0) {
    if (digitEnd <= digitBegin) {
      return true;
    } else if (lineEnd <= lineBegin) {
      return false;
    }
  } else {
    if (digitEnd >= digitBegin) {
      return true;
    } else if (lineEnd >= lineBegin) {
      return false;
    }
  }

  if (!LineMakeSimple(lineBegin, lineEnd, digitBegin, digitEnd)) {
    return false;
  }

  return LineMakeFindAndFixYes(lineBegin, lineEnd, digitBegin, digitEnd);
}

bool Ai::LineMakeSimple(int lineBegin, int lineEnd, int digitBegin, int digitEnd)
{
  int curLine = lineBegin;
  for (int curDigit = digitBegin; curDigit != digitEnd; curDigit += mCurrentInc) {
    int digit = mCurrentDigits->at(curDigit);
    int count = 0;
    for (; count != digit && curLine != lineEnd; curLine += mCurrentInc) {
      if (!mCurrentLine->at(curLine).IsMarkNo()) {
        count++;
      } else {
        count = 0;
      }
    }
    if (count == digit) {
      int begin = curLine - mCurrentInc * digit;
      int end   = curLine - mCurrentInc;
      (*mCurrentLineDigits)[curDigit].Left  = qMin(begin, end);
      (*mCurrentLineDigits)[curDigit].Right = qMax(begin, end);
    } else {
      return false;
    }
    if (curLine != lineEnd) {
      curLine += mCurrentInc;
    }
  }
  return true;
}

bool Ai::LineMakeFindAndFixYes(int lineBegin, int lineEnd, int digitBegin, int digitEnd)
{
  int realBorderLeft  = qMin(lineBegin, lineEnd - mCurrentInc);
  int realBorderRight = qMax(lineBegin, lineEnd - mCurrentInc);
  int curDigit = digitEnd - mCurrentInc;
  for (int i = lineEnd - mCurrentInc; i != lineBegin - mCurrentInc; i -= mCurrentInc) { // find all Yes and fix LineDigits
    if (!mCurrentLine->at(i).IsMarkYes()) {
      continue;
    }

    if (mCurrentInc > 0) {
      while (curDigit >= digitBegin && i < mCurrentLineDigits->at(curDigit).Left) {
        curDigit--;
      }
      if (curDigit < digitBegin) {
        return false;
      }
    } else {
      while (curDigit <= digitBegin && i > mCurrentLineDigits->at(curDigit).Right) {
        curDigit++;
      }
      if (curDigit > digitBegin) {
        return false;
      }
    }

    forever {
      if (i >= mCurrentLineDigits->at(curDigit).Left && i <= mCurrentLineDigits->at(curDigit).Right) { // already fixed
        break;
      }

      int digit = mCurrentDigits->at(curDigit);
      bool fixed = false;
      for (int disp = digit - 1; disp >= 0; disp--) { // try all digit positions from most left
        int left  = i - disp * mCurrentInc;
        int right = left + (digit - 1) * mCurrentInc;
        int realLeft  = qMin(left, right);
        int realRight = qMax(left, right);
        if (realLeft < realBorderLeft || realRight > realBorderRight) {
          continue;
        }
        if (realLeft > 0 && mCurrentLine->at(realLeft - 1).IsMarkYes()) {
          continue;
        }
        if (realRight < mCurrentLine->size() - 1 && mCurrentLine->at(realRight + 1).IsMarkYes()) {
          continue;
        }
        bool ok = true;
        for (int ii = left; ii != right + mCurrentInc; ii += mCurrentInc) {
          if (mCurrentLine->at(ii).IsMarkNo()) {
            ok = false;
            break;
          }
        }
        if (!ok) {
          continue;
        }
        if (LineMakeFull(lineBegin, left - mCurrentInc, digitBegin, curDigit)
            && LineMakeFull(right + 2*mCurrentInc, lineEnd, curDigit + mCurrentInc, digitEnd)) {
          fixed = true;
          (*mCurrentLineDigits)[curDigit].Left  = realLeft;
          (*mCurrentLineDigits)[curDigit].Right = realRight;
          break;
        }
      }

      if (fixed) {
        break;
      }
      if (curDigit == digitBegin) {
        return false;
      }
      curDigit -= mCurrentInc;
    }
  }
  return true;
}

int Ai::HintCalcCellPrice(int i, int j, bool isYes)
{
  int price = isYes? 2: 1;
  int deltaI = qMin(i, mCurrentPuzzle->getWidth() - 1 - i);
  int deltaJ = qMin(j, mCurrentPuzzle->getHeight() - 1 - j);
  int delta = qMin(deltaI, deltaJ);
  switch (delta) {
  case 0: price *= 5; break;
  case 1: price *= 3; break;
  case 2: price *= 2; break;
  }
  int naibor = 0;
  if (i > 0 && mCurrentPuzzle->At(i - 1, j).HasMark() && mCurrentPuzzle->At(i - 1, j).IsMarkYes() != isYes) {
    naibor++;
  }
  if (j > 0 && mCurrentPuzzle->At(i, j - 1).HasMark() && mCurrentPuzzle->At(i, j - 1).IsMarkYes() != isYes) {
    naibor++;
  }
  if (j < mCurrentPuzzle->getHeight() - 1 && mCurrentPuzzle->At(i, j + 1).HasMark() && mCurrentPuzzle->At(i, j + 1).IsMarkYes() != isYes) {
    naibor++;
  }
  if (naibor > 0) {
    price *= (naibor + 1);
  }
  return price;
}

int Ai::SolveDoAll()
{
  mSolveResult = 0;
  int totalCount = mCurrentPuzzle->Size();

  forever {
    int solved = SolveAllLines();
    if (mStop || solved == 0) {
      break;
    } else if (solved < 0) {
      mSolveResult = -1;
      break;
    }

    mSolveResult = 1;
    mPuzzleSolved += solved;

    if (mPuzzleSolved >= totalCount) {
      return mSolveResult;
    }
  }
  return mSolveResult;
}

void Ai::SolvePrepareProp()
{
  mPropTimer.start();
  qsrand(QDateTime::currentMSecsSinceEpoch());

  SolveCreateProp();
}

void Ai::SolveUpdatePropInfo()
{
  if (mPropTimer.elapsed() < kPropUpdatePeriodMs) {
    return;
  }

  QByteArray solveInfo;
  mPropPuzzle->ToByteArray(solveInfo);
  emit SolveInfo(solveInfo);

  mPropTimer.restart();
}

void Ai::SolveCreateProp()
{
  mPropMap.resize(mCurrentPuzzle->Size());
  mPropValueList.clear();
  mPropValueSum = 0;

  for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      Prop* prop = &mPropMap[j*mCurrentPuzzle->getWidth() + i];
      if (mCurrentPuzzle->At(i, j).HasMark()) {
        continue;
      }

      prop->Value = 0;
      if (j < 3) {
        prop->Value = (3 - j) * qMax(1, prop->Value) * 5;
      } else if (j > mCurrentPuzzle->getHeight() - 4) {
        prop->Value = (j - (mCurrentPuzzle->getHeight() - 4)) * qMax(1, prop->Value) * 5;
      }
      if (i < 3) {
        prop->Value = (3 - i) * qMax(1, prop->Value) * 5;
      } else if (i > mCurrentPuzzle->getWidth() - 4) {
        prop->Value = (i - (mCurrentPuzzle->getWidth() - 4)) * qMax(1, prop->Value) * 5;
      }

      if (j > 0 && mCurrentPuzzle->At(i, j-1).HasMark()) {
        prop->Value = (mCurrentPuzzle->At(i, j-1).IsMarkYes()? 2: 1) * qMax(1, prop->Value) * 5;
      }
      if (j < mCurrentPuzzle->getHeight() - 1 && mCurrentPuzzle->At(i, j+1).HasMark()) {
        prop->Value = (mCurrentPuzzle->At(i, j+1).IsMarkYes()? 2: 1) * qMax(1, prop->Value) * 5;
      }
      if (i > 0 && mCurrentPuzzle->At(i-1, j).HasMark()) {
        prop->Value = (mCurrentPuzzle->At(i-1, j).IsMarkYes()? 2: 1) * qMax(1, prop->Value) * 5;
      }
      if (i < mCurrentPuzzle->getWidth() - 1 && mCurrentPuzzle->At(i+1, j).HasMark()) {
        prop->Value = (mCurrentPuzzle->At(i+1, j).IsMarkYes()? 2: 1) * qMax(1, prop->Value) * 5;
      }

      if (prop->Value > 0) {
        mPropValueList.append(prop);
        mPropValueSum += prop->EffectiveValue();
      }
    }
  }
}

bool Ai::SolveDoProp()
{
  int currentPuzzleSolved = mPuzzleSolved;
  if (mPropValueList.isEmpty()) {
    return false;
  }

  int iProp = -1;
  int jProp = -1;
  int valSel = qrand() % mPropValueSum;
  for (auto itr = mPropValueList.begin(); itr != mPropValueList.end(); itr++) {
    Prop* prop = *itr;
    int curVal = prop->EffectiveValue();
    if (valSel < curVal) {
      int indProp = prop - mPropMap.data();
      iProp = indProp % mCurrentPuzzle->getWidth();
      jProp = indProp / mCurrentPuzzle->getWidth();
      mPropValueList.erase(itr);
      mPropValueSum -= curVal;
      break;
    }
    valSel -= curVal;
  }

  if (iProp < 0 || jProp < 0 || jProp >= mCurrentPuzzle->getHeight()) {
    return false;
  }

  mPropPuzzle = mCurrentPuzzle;
  Puzzle tempPuzzle;
  mCurrentPuzzle = &tempPuzzle;

  mPropPuzzle->Value(iProp, jProp).SetProp();
  bool ok = false;
  if (!ok) {
    mCurrentPuzzle->Copy(*mPropPuzzle);
    mCurrentPuzzle->Value(iProp, jProp).SetMark(1, mCurrentLevel);
    if (SolveDoAll() < 0) {
      mPropPuzzle->Value(iProp, jProp).SetMark(-1, mCurrentLevel);
      ok = true;
    }
  }

  if (!ok) {
    mCurrentPuzzle->Copy(*mPropPuzzle);
    mCurrentPuzzle->Value(iProp, jProp).SetMark(-1, mCurrentLevel);
    if (SolveDoAll() < 0) {
      mPropPuzzle->Value(iProp, jProp).SetMark(1, mCurrentLevel);
      ok = true;
    }
  }

  mCurrentPuzzle = mPropPuzzle;
  mPuzzleSolved = currentPuzzleSolved;

  if (ok) {
    SolveDoAll();
    SolveCreateProp();
  } else {
    mPropMap[jProp*mCurrentPuzzle->getWidth() + iProp].Miss++;
    mSolveResult = 0;
  }

  return ok || !mPropValueList.isEmpty();
}

int Ai::SolveAllLines()
{
  int solved = 0;
  Cells line(mCurrentPuzzle->getWidth());
  QVector<int> digits(mCurrentPuzzle->getWidth());
  for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      line[i] = mCurrentPuzzle->At(i, j);
      digits[i] = mCurrentPuzzle->mDigitsHorz[i][j];
    }

    if (!LineSolve(line, digits)) {
      return -1;
    }

    for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
      if (!mCurrentPuzzle->Value(i, j).HasMark() && line[i].HasMark()) {
        mCurrentPuzzle->Value(i, j) = line[i];
        solved++;
      }
    }
  }

  line.resize(mCurrentPuzzle->getHeight());
  digits.resize(mCurrentPuzzle->getHeight());
  for (int i = 0; i < mCurrentPuzzle->getWidth(); i++) {
    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
      line[j] = mCurrentPuzzle->At(i, j);
      digits[j] = mCurrentPuzzle->mDigitsVert[i][j];
    }

    if (!LineSolve(line, digits)) {
      return -1;
    }

    for (int j = 0; j < mCurrentPuzzle->getHeight(); j++) {
      if (!mCurrentPuzzle->Value(i, j).HasMark() && line[j].HasMark()) {
        mCurrentPuzzle->Value(i, j) = line[j];
        solved++;
      }
    }
  }

  return solved;
}

void Ai::StopSolve()
{
  mStop = true;
}


Ai::Ai()
  : mCurrentPuzzle(nullptr), mCurrentLevel(0)
{
  if (!mSelf) {
    mSelf = this;
  }
}

