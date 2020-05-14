#pragma once

#include <QElapsedTimer>
#include <QVector>
#include <QMap>
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
  int                  mPuzzleSolved;
  int                  mPropCommited;
  int                  mSolveResult;
  volatile bool        mStop;

  struct Prop {
    int Value;
    int Miss;

    int EffectiveValue() { return Miss > 0? qMax(1, Value / Miss): Value; }
    Prop(): Value(0), Miss(0) { }
  };
  Puzzle*              mPropPuzzle;
  QVector<Prop>        mPropMap;
  QList<Prop*>         mPropValueList;
  int                  mPropValueSum;
  QElapsedTimer        mPropTimer;

  Q_OBJECT

public:
  static Ai* Instance() { return mSelf; }

public:
  void CalcAllDigits(Puzzle* puzzle);
  void CalcHorzDigits(Puzzle* puzzle, int j);
  void CalcVertDigits(Puzzle* puzzle, int i);

  bool Test(Puzzle* puzzle);
  bool Hint(Puzzle* puzzle, int level, bool& hasSolve);

  void Solve(Puzzle* puzzle, int level, int maxProp);
  void Stars(Puzzle* puzzle);

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

  int SolveDoAll();
  void SolvePrepareProp();
  void SolveUpdatePropInfo();
  void SolveCreateProp();
  bool SolveDoProp();
  int SolveAllLines();

signals:
  void SolveChanged(int solved, int prop);
  void SolveDone(int result, int prop);
  void SolveInfo(QByteArray data);

public slots:
  void StopSolve();

public:
  Ai();
};

#define qAi (Ai::Instance())
