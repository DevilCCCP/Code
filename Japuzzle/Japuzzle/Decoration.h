#pragma once

#include <QObject>
#include <QPoint>

#include <Lib/Include/Common.h>


DefineClassS(Decoration);
DefineClassS(Puzzle);

class Decoration: public QObject
{
public:
  enum ECursor {
    eCursorUndefined,
    eCursorArrow,
    eCursorNormalCell,
    eCursorYesCell,
    eCursorYesHLine,
    eCursorYesVLine,
    eCursorYesBlock,
    eCursorNoCell,
    eCursorNoHLine,
    eCursorNoVLine,
    eCursorNoBlock,
  };

private:
  static Decoration*           mSelf;
  PuzzleS                      mPuzzle;
  PROPERTY_GET(int,            Zoom)
  PROPERTY_GET(int,            CellWidth)
  PROPERTY_GET(int,            CellHeight)

  PROPERTY_GET(ECursor,        Cursor)
  PROPERTY_GET(QPoint,         HighlightPos)
  PROPERTY_GET(QPoint,         LastHighlightPos)

  Q_OBJECT

public:
  static Decoration* Instance() { return mSelf; }

public:
  int GetTableWidth() const;
  int GetTableHeight() const;
  bool HasZoomIn() const;
  bool HasZoomOut() const;

public:
  void SetPuzzle(const PuzzleS& _Puzzle);
  void Setup();
  int CellToPointX(int i) const
  { return (mCellWidth + 1) * i + i/5 + 2; }
  int CellToPointY(int j) const
  { return (mCellHeight + 1) * j + j/5 + 2; }
  bool TranslatePointToCellX(const QPoint& lastPos, int x, int& i);
  bool TranslatePointToCellY(const QPoint& lastPos, int y, int& j);

public slots:
  void ZoomChange(int value);
  void ZoomIn();
  void ZoomOut();
  void CursorChange(ECursor value);
  void HighlightPosChange(const QPoint& value);

signals:
  void CursorChanged();
  void HighlightPosChanged();
  void ZoomChanged();

public:
  Decoration();
};

#define qDecoration (Decoration::Instance())
