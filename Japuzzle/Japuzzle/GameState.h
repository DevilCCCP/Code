#pragma once

#include <QObject>
#include <QVector>
#include <QIcon>

#include <Lib/Include/Common.h>


DefineClassS(GameState);

class GameState: public QObject
{
public:
  enum EState {
    eNoSolve = 0,
    eBadSolve,
    eUnknownSolve,
    eAiSolve,
    eManSolve,
  };

private:
  static GameState*    mSelf;

  PROPERTY_GET(EState, State)
  PROPERTY_GET(bool,   Solved)
  QVector<QIcon>       mSmallIcons;
  QVector<QIcon>       mBigIcons;

  Q_OBJECT

public:
  static GameState* Instance() { return mSelf; }
  bool IsStateSolved() { return mState > GameState::eBadSolve; }
  QIcon SmallIcon();
  QIcon BigIcon();

public slots:
  void SolveClear();
  void StateChange(EState value);

signals:
  void StateChanged();
  void SolvedChanged();

public:
  GameState();
};

#define qGameState (GameState::Instance())
