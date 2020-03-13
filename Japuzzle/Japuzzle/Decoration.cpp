#include "Decoration.h"
#include "Core.h"
#include "Puzzle.h"


Decoration* Decoration::mSelf = nullptr;
const int kMinZoom = 5;
const int kMaxZoom = 15;
const int kBaseCellWidth = 30;
const int kBaseCellHeight = 30;

int Decoration::GetTableWidth() const
{
  return (mCellWidth + 1) * mPuzzle->getWidth() + mPuzzle->getWidth()/5 + 2;
}

int Decoration::GetTableHeight() const
{
  return (mCellHeight + 1) * mPuzzle->getHeight() + mPuzzle->getHeight()/5 + 2;
}

bool Decoration::HasZoomIn() const
{
  return mZoom < kMaxZoom;
}

bool Decoration::HasZoomOut() const
{
  return mZoom > kMinZoom;
}

void Decoration::SetPuzzle(const PuzzleS& _Puzzle)
{
  mPuzzle = _Puzzle;
}

void Decoration::Setup()
{
  mCellWidth  = kBaseCellWidth * mZoom / 10;
  mCellHeight = kBaseCellHeight * mZoom / 10;
}

bool Decoration::TranslatePointToCellX(const QPoint& lastPos, int x, int& i)
{
  i = (5*x - 10) / (5*mCellWidth + 5 + 1);
  if (i < 0 || i >= mPuzzle->getWidth()) {
    return false;
  }
  int l = CellToPointX(i);
  int r = l + mCellWidth;
  if (x < l || x > r) {
    if (qAbs(i - lastPos.x()) <= 1 && (qAbs(x - l) <= 2 || qAbs(x - r) <= 2)) {
      i = lastPos.x();
    } else {
      return false;
    }
  }
  return i >= 0;
}

bool Decoration::TranslatePointToCellY(const QPoint& lastPos, int y, int& j)
{
  j = (5*y - 10) / (5*mCellHeight + 5 + 1);
  if (j < 0 || j >= mPuzzle->getHeight()) {
    return false;
  }
  int t = CellToPointY(j);
  int b = t + mCellHeight;
  if (y < t || y > b) {
    if (qAbs(j - lastPos.y()) <= 1 && (qAbs(y - t) <= 2 || qAbs(y - b) <= 2)) {
      j = lastPos.y();
    } else {
      return false;
    }
  }
  return j >= 0;
}

void Decoration::ZoomChange(int value)
{
  if (mZoom != value) {
    mZoom = value;

    Setup();
    emit ZoomChanged();
  }
}

void Decoration::ZoomIn()
{
  if (!HasZoomIn()) {
    return;
  }

  mZoom++;

  Setup();
  emit ZoomChanged();
}

void Decoration::ZoomOut()
{
  if (!HasZoomOut()) {
    return;
  }

  mZoom--;

  Setup();
  emit ZoomChanged();
}

void Decoration::CursorChange(Decoration::ECursor value)
{
  if (mCursor != value) {
    mCursor = value;

    emit CursorChanged();
  }
}

void Decoration::HighlightPosChange(const QPoint& value)
{
  if (mHighlightPos != value) {
    mLastHighlightPos = mHighlightPos;
    mHighlightPos = value;

    emit HighlightPosChanged();
  }
}


Decoration::Decoration()
  : mZoom(10), mCellWidth(kBaseCellWidth), mCellHeight(kBaseCellHeight)
  , mCursor(), mHighlightPos(-1, -1), mLastHighlightPos(-1, -1)
{
  if (!mSelf) {
    mSelf = this;
  }

  Setup();
}

