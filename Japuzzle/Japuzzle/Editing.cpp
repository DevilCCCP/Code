#include "Editing.h"
#include "Core.h"
#include "Puzzle.h"


Editing* Editing::mSelf = nullptr;

void Editing::ModeChange(Editing::EMode value)
{
  if (mMode != value) {
    mMode = value;

    emit ModeChanged();
  }
}

void Editing::UndoChanged(bool hasUndo, bool hasRedo)
{
  if (mHasUndo != hasUndo) {
    mHasUndo = hasUndo;

    emit HasUndoChanged();
  }
  if (mHasRedo != hasRedo) {
    mHasRedo = hasRedo;

    emit HasRedoChanged();
  }
}


Editing::Editing()
  : mMode(eModeNormal)
  , mCurrentPropLevel(0)
{
  if (!mSelf) {
    mSelf = this;
  }
}

