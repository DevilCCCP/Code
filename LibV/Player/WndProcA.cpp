#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Include/ModuleNames.h>

#include "WndProcA.h"
#include "Drawer.h"
#include "Render.h"


void WndProcA::UpdateTime(const QDateTime &timestamp)
{
  QMutexLocker lock(&mChangeMutex);
  mChangeTime = timestamp;
  mChangeState |= eTimeChanged;
}

void WndProcA::UpdateFps(const qreal& fps)
{
  QMutexLocker lock(&mChangeMutex);
  mChangeFps = fps;
  mChangeState |= eFpsChanged;
}

void WndProcA::UpdateInfo(const QString &cameraName)
{
  QMutexLocker lock(&mChangeMutex);
  mChangeInfo = cameraName;
  mChangeState |= eInfoChanged;
}

void WndProcA::UpdateBox(const QRect& box)
{
  QMutexLocker lock(&mChangeMutex);
  mChangeBox = box;
  mChangeState |= eBoxChanged;
}

bool WndProcA::DoInit()
{
  return true;
}

bool WndProcA::DoCircle()
{
  if (!mSingle && !mEmbedded) {
    ConnectBackWnd();
  }

  ProcessChanges();

  ProcessMsgQueue();

  return IsAlive();
}

void WndProcA::DoRelease()
{
  while (!mDrawer->WaitFinish(1)) {
    ProcessMsgQueue();
  }
}

void WndProcA::ShowRect(const QRect& rect)
{
  mSceneRect = rect;
  Show();
}

void WndProcA::OnChangedTime(const QDateTime& changeTime)
{
  Q_UNUSED(changeTime);
}

void WndProcA::OnChangedFps(const qreal& changeFps)
{
  Q_UNUSED(changeFps);
}

void WndProcA::OnChangedInfo(const QString& changeInfo)
{
  Q_UNUSED(changeInfo);
}

void WndProcA::OnChangedBox()
{
}

void WndProcA::ProcessChanges()
{
  QMutexLocker lock(&mChangeMutex);
  if (mChangeState != eNoneState) {
    int      changeState = mChangeState;
    QDateTime changeTime = mChangeTime;
    qreal      changeFps = mChangeFps;
    QString   changeInfo = mChangeInfo;
    mChangeState = eNoneState;
    lock.unlock();

    if (changeState & eTimeChanged) {
      OnChangedTime(changeTime);
    }
    if (changeState & eFpsChanged) {
      OnChangedFps(changeFps);
    }
    if (changeState & eInfoChanged) {
      OnChangedInfo(changeInfo);
    }
    if (changeState & eBoxChanged) {
      mSceneRect = mChangeBox;
      OnChangedBox();
    }
  }
}


WndProcA::WndProcA(Overseer* _Overseer, Render *_Render, Drawer *_Drawer, const QRect &_SceneRect
                 , bool _PrimeWindow, bool _ShowMouse, bool _AutoHideMouse, bool _AlwaysOnTop, bool _PlaySound, EStyleType _Style)
  : CtrlWorker(5)
  , mId(_Overseer->Id()), mSingle(_Overseer->Detached() || _Overseer->Params() == "single"), mEmbedded(_Overseer->Id() == 1), mRender(_Render), mDrawer(_Drawer)
  , mPrimeWindow(_PrimeWindow), mShowMouse(_ShowMouse), mAutoHideMouse(_AutoHideMouse), mAlwaysOnTop(_AlwaysOnTop), mPlaySound(_PlaySound), mStyle(_Style)
  , mSceneRect(_SceneRect)
  , mChangeState(eNoneState)
{
}

WndProcA::~WndProcA()
{
}


