#pragma once

#include <QVector>

#include <Lib/Include/Common.h>

#include "Cell.h"
#include "Propotion.h"
#include "UndoInfo.h"


DefineClassS(Editing);
class QDataStream;
class QTextStream;

class Puzzle
{
  QVector<Cell>         mTable;
  PROPERTY_GET(QString, SourceName)
  PROPERTY_GET(QString, ViewName)
  PROPERTY_GET(int,     Width)
  PROPERTY_GET(int,     Height)

  PROPERTY_GET(QVector<Line>, DigitsHorz)
  PROPERTY_GET(QVector<Line>, DigitsVert)
  PROPERTY_GET(int          , DigitsHorzMax)
  PROPERTY_GET(int          , DigitsVertMax)
  PROPERTY_GET(QVector<Line>, DigitsMarkHorz)
  PROPERTY_GET(QVector<Line>, DigitsMarkVert)

  EditingS              mEditing;
  QVector<Propotion>    mProp;
  int                   mStars;
  bool                  mDigits;

  QDataStream*          mStream;
  QTextStream*          mTxtStream;
  int                   mOneUndoSize;
  QList<UndoInfo>       mUndoStack;
  UndoInfo*             mCurrentUndo;
  int                   mUndoIndex;

public:
  bool IsEmpty() const { return mWidth <= 0 || mHeight <= 0; }
  const Cell& At(const QPoint& p) const { return mTable.at(p.x() + p.y() * mWidth); }
  const Cell& At(int i, int j) const { return mTable.at(i + j * mWidth); }
  Cell& Value(const QPoint& p) { return mTable[p.x() + p.y() * mWidth]; }
  Cell& Value(int i, int j) { return mTable[i + j * mWidth]; }
  bool IsValid(const QPoint& p) const { return p.x() >= 0 && p.x() < mWidth && p.y() >= 0 && p.y() < mHeight; }
  void SetName(const QString& _SourceName);
  int Stars() const { return mStars; }
  void SetStars(int stars) { mStars = stars; }
  QString StarsText();

public:
  void New(int width, int height);
  void Reset();
  bool IsBlank();
  void Clear();
  void ClearPropMark();
  void Copy(const Puzzle& puzzle);
  void Apply(const Puzzle& puzzle);
  bool Load(const QString& filename, bool compact = true);
  bool Save(const QString& filename);
  int Count() const;
  int Size() const;
  void SetEditing(const EditingS& _Editing);

  void SetCells(const QPoint& p1, const QPoint& p2, int mark, int level);
  void ClearProp(int level);
  void ApplyProp(int level);
  int GetAutoPropLevel();
  bool SolveTest(bool isAi = false);
  bool CalcSolve(bool& realSolved);

  void SetDigit(Qt::Orientation type, const QPoint& p1, const QPoint& p2, int value);
  void SetDigit(Qt::Orientation type, const QPoint& p, int value);

  void ClearUndo();
  void MakeUndo();
  bool HasUndo();
  bool HasRedo();
  bool DoUndo();
  bool DoRedo();

  void Resize(const QRect& realRect);
  void ToByteArray(QByteArray& data);
  void FromByteArray(const QByteArray& data);

private:
  void ApplyUndo();

private:
  bool LoadYpp(bool compact);
  bool LoadTxt();
  bool SaveYpp();

  bool ValidateSize();
  void CalcDigits();
  void LoadDigits();
  bool SaveDigits();
  void CalcMax();

  void TryCompact();

public:
  class PuzzleRef
  {
    Puzzle* mPuzzle;
    int     mI;

  public:
    Cell& operator[] (int j) { return mPuzzle->Value(mI, j); }

  public:
    PuzzleRef(Puzzle* _Puzzle, int _I)
      : mPuzzle(_Puzzle), mI(_I)
    { }
  };
  PuzzleRef operator[](int i) { return PuzzleRef(this, i); }

  class PuzzleRefConst
  {
    const Puzzle* mPuzzle;
    int           mI;

  public:
    const Cell& operator[] (int j) const { return mPuzzle->At(mI, j); }

  public:
    PuzzleRefConst(const Puzzle* _Puzzle, int _I)
      : mPuzzle(_Puzzle), mI(_I)
    { }
  };
  PuzzleRefConst operator[](int i) const { return PuzzleRefConst(this, i); }

public:
  Puzzle();

  friend class Ai;
};
