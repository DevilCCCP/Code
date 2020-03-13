#pragma once

#include <QVector>
#include <QObject>
#include <QPoint>

#include <Lib/Include/Common.h>


DefineClassS(Ai);
DefineClassS(Puzzle);
class Cell;
typedef QVector<Cell> Cells;

class Ai: public QObject
{
  static Ai*           mSelf;

  struct LineDigit {
    int Left;
    int Right;
  };
  typedef QVector<LineDigit> LineDigits;

  const Cells*         mCurrentLine;
  const QVector<int>*  mCurrentDigits;
  LineDigits*          mCurrentLineDigits;
  int                  mCurrentInc;

  Puzzle*              mCurrentPuzzle;
  int                  mCurrentLevel;
  int                  mCurrentPuzzleSolved;
  int                  mPuzzleSolved;

  Q_OBJECT

public:
  static Ai* Instance() { return mSelf; }

public:
  void CalcAllDigits(Puzzle* puzzle);
  void CalcHorzDigits(Puzzle* puzzle, int j);
  void CalcVertDigits(Puzzle* puzzle, int i);

  bool Test(Puzzle* puzzle);
  bool Hint(Puzzle* puzzle, int level, bool& hasSolve);

  void Solve(Puzzle* puzzle);
  bool SolveResult();

private:
  void CalcDigitsSimple(const Cells& line, const QVector<int>& digits, QVector<int>& colors);
  void CalcDigitsSmart(const Cells& line, const QVector<int>& digits, QVector<int>& colors);

  bool LineSolve(Cells& line, const QVector<int>& digits);
  bool LineMakeLeft(const Cells& line, const QVector<int>& digits, LineDigits& lineDigits);
  bool LineMakeRight(const Cells& line, const QVector<int>& digits, LineDigits& lineDigits);
  bool LineMakeFull(int lineBegin, int lineEnd, int digitBegin, int digitEnd);
  bool LineMakeSimple(int lineBegin, int lineEnd, int digitBegin, int digitEnd);
  bool LineMakeFindAndFixYes(int lineBegin, int lineEnd, int digitBegin, int digitEnd);

  int HintCalcCellPrice(int i, int j, bool isYes);

  bool PuzzleSolve();
  bool PuzzleSolveProp();

signals:
  void SolveChanged(int count);

public:
  Ai();
};

#define qAi (Ai::Instance())
