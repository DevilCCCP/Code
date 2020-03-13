#include "GameState.h"


GameState* GameState::mSelf = nullptr;

QIcon GameState::SmallIcon()
{
  return mSmallIcons.value(mState);
}

QIcon GameState::BigIcon()
{
  return mBigIcons.value(mState);
}

void GameState::SolveClear()
{
  mSolved = false;
}

void GameState::StateChange(GameState::EState value)
{
  if (mState != value) {
    mState = value;

    emit StateChanged();

    if (!mSolved && mState > eBadSolve) {
      mSolved = true;

      emit SolvedChanged();
    }
  }
}


GameState::GameState()
  : mState(eNoSolve)
{
  if (!mSelf) {
    mSelf = this;
  }

  mSmallIcons.append(QIcon(":/Icons/Game Not Solve.png"));
  mSmallIcons.append(QIcon(":/Icons/Game Solve Bad.png"));
  mSmallIcons.append(QIcon(":/Icons/Game Solve Unknown.png"));
  mSmallIcons.append(QIcon(":/Icons/Game Solve Ai.png"));
  mSmallIcons.append(QIcon(":/Icons/Game Solve Man.png"));

  mBigIcons.append(QIcon(":/Icons/Game Not Solve 128x128.png"));
  mBigIcons.append(QIcon(":/Icons/Game Solve Bad 128x128.png"));
  mBigIcons.append(QIcon(":/Icons/Game Solve Unknown 128x128.png"));
  mBigIcons.append(QIcon(":/Icons/Game Solve Ai 128x128.png"));
  mBigIcons.append(QIcon(":/Icons/Game Solve Man 128x128.png"));
  Q_ASSERT(eManSolve == 4);
}

