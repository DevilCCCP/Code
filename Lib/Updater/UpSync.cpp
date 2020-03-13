#include <Lib/Log/Log.h>

#include "UpSync.h"
#include "Updater.h"


const int kWorkPeriodMs = 100;

bool UpSync::DoCircle()
{
  if (QDateTime::currentMSecsSinceEpoch() > mNextCheck) {
    return !SyncUpdate();
  }
  return true;
}

bool UpSync::SyncUpdate()
{
  if (!mUserWait) {
    if (mUpInfo->HasUserUpStart()) {
      mUserWait = true;
    }
  }

  if (mUserWait) {
    if (mUpInfo->GetUserUpStart() < 0) {
      return StartUpdate();
    }
  }
  int waitMs = mUserWait? 500: 5000;

  int upStart = mUpInfo->GetReadyToUpdate();
  if (upStart <= 0) {
    return StartUpdate();
  }
  waitMs = qMin(upStart, waitMs);
  mNextCheck = QDateTime::currentMSecsSinceEpoch() + waitMs;
  return false;
}

bool UpSync::StartUpdate()
{
  mUpdater->OnUpdateSync();
  return true;
}


UpSync::UpSync(Updater* _Updater, const UpInfoS& _UpInfo)
  : Imp(kWorkPeriodMs)
  , mUpdater(_Updater), mUpInfo(_UpInfo)
  , mNextCheck(0), mUserWait(false)
{
}

