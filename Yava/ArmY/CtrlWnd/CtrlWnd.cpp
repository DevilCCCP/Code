#include "stdafx.h"

#include <QDesktopWidget>
#include <qsystemdetection.h>

#include <Lib/Settings/FileSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>
#include <LibV/Include/ModuleNames.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ArmMonitors.h>

#include "CtrlWnd.h"
#ifdef Q_OS_WIN32
#include "Win/BackWnd.h"
#endif


void CtrlWnd::Init(Overseer* overseer)
{
  overseer->RegisterWorker(mBackWnd);

  UpdateMonitorsInfo();
  if (!overseer->Detached()) {
    if (mStateShmem.create(sizeof(ArmState))) {
      Log.Warning("Create new arm state shmem");
      memset(mStateShmem.data(), 0, sizeof(ArmState));
    } else if (!mStateShmem.attach()) {
      Log.Fatal("Can't attach StateShmem", true);
      return;
    } else if (mStateShmem.size() != sizeof(ArmState)) {
      Log.Warning(QString("StateShmem has invalid size (found: %1, must: %2)").arg(mStateShmem.size()).arg(sizeof(ArmState)));
    }
    mArmState = static_cast<ArmState*>(mStateShmem.data());
  }
}

void CtrlWnd::RegisterPlayerType(int id, bool isPrime)
{
  if (isPrime) {
    if (mPrimePlayerId && mPrimePlayerId != id) {
      Log.Warning(QString("Prime player changed %1 -> %2").arg(mPrimePlayerId).arg(id));
    }
    mPrimePlayerId = id;
  }
}

void CtrlWnd::RegisterPlayerTopLeft(int id, const QPoint &point)
{
  auto itr = mPlayerWindows.find(id);
  if (itr == mPlayerWindows.end()) {
    mPlayerWindows.insert(id, QRect(point, point));
    return;
  } else {
    QRect& wnd = itr.value();
    wnd.setTopLeft(point);
  }
}

void CtrlWnd::RegisterPlayerBottomRight(int id, const QPoint &point)
{
  auto itr = mPlayerWindows.find(id);
  if (itr == mPlayerWindows.end()) {
    mPlayerWindows.insert(id, QRect(point, point));
    Log.Warning(QString("Receive BottomRight point before TopLeft (id: %1)"));
  } else {
    QRect& wnd = itr.value();
    wnd.setBottomRight(point);

    bool use = wnd.intersects(mScreenRect);
    Log.Info(QString("Registered player %6 (id: %1, rect: (%2, %3, %4, %5))")
             .arg(id).arg(wnd.left()).arg(wnd.top()).arg(wnd.right()).arg(wnd.bottom())
             .arg((use)? "accepted": "denied"));
    if (use) {
      UpdatePlayer(id);
    } else {
      mBackWnd->PlayerShowRect(id, mPlayerWindows[id]);
      mPlayerWindows.remove(id);
    }
  }
}

void CtrlWnd::MovePlayer(int id)
{
  Log.Trace(QString("Move player to top (id: %1)").arg(id));
  switch (mArmMode) {
  case eNormal:
    if (id == mPrimePlayerId || mPrimePlayerId == 0) {
      mArmMode = eFullscreen;
      mSelectedPlayerId = id;
      UpdatePlayerAll();
    } else {
      mArmMode = ePrime;
      mSelectedPlayerId = id;
      UpdatePlayer(mSelectedPlayerId);
      UpdatePlayer(mPrimePlayerId);
    }
    break;

  case ePrime:
    if (id == mSelectedPlayerId) {
      mArmMode = eFullscreen;
      UpdatePlayerAll();
    } else if (id == mPrimePlayerId) {
      mArmMode = eNormal;
      int selId = mSelectedPlayerId;
      mSelectedPlayerId = 0;
      UpdatePlayer(selId);
      UpdatePlayer(mPrimePlayerId);
    } else {
      mArmMode = ePrime;
      int selId = mSelectedPlayerId;
      mSelectedPlayerId = id;
      UpdatePlayer(selId);
      UpdatePlayer(mSelectedPlayerId);
      UpdatePlayer(mPrimePlayerId);
    }
    break;

  case eFullscreen:
    if (id == mSelectedPlayerId) {
      if (id == mPrimePlayerId || mPrimePlayerId == 0) {
        mArmMode = eNormal;
        mSelectedPlayerId = 0;
      } else {
        mArmMode = ePrime;
      }
      UpdatePlayerAll();
    }
    break;

  case eModeIllegal:
    break;
  }
  Log.Info(QString("Mode change to '%1' (sel: %2, prime: %3)")
           .arg(EArmState_ToString(mArmMode)).arg(mSelectedPlayerId).arg(mPrimePlayerId));
}

void CtrlWnd::DoExit()
{
  Log.Info("Exit pushed");
  mArmState->Signal |= (int)ePowerOff;
}

void CtrlWnd::DoLayout(int id, int count)
{
  Log.Info(QString("Layout pushed (id: %1, count: %2)").arg(id).arg(count));
  mArmState->LayoutType = id;
  mArmState->LayoutCount = count;

  mArmMode = eNormal;
  mSelectedPlayerId = 0;
  mPrimePlayerId = 0;
  mPlayerWindows.clear();
}

void CtrlWnd::DoSwitchDesktop()
{
  Log.Info("Desktop show/hide switch");
  mArmState->Signal = mArmState->Signal ^ eHideDesktop;
}

void CtrlWnd::DoSwitchOther()
{
  Log.Info("Other show/hide switch");
  mArmState->Signal = mArmState->Signal ^ eHideOther;
}

void CtrlWnd::UpdatePlayerAll()
{
  for (auto itr = mPlayerWindows.begin(); itr != mPlayerWindows.end(); itr++) {
    UpdatePlayer(itr.key());
  }
}

void CtrlWnd::UpdatePlayer(int id)
{
  switch (mArmMode) {
  case eNormal:
    if (mPlayerWindows.contains(id)) {
      mBackWnd->PlayerShowRect(id, mPlayerWindows[id]);
    } else {
      mBackWnd->PlayerHide(id);
    }
    break;

  case ePrime:
    if (id == mSelectedPlayerId) {
      mBackWnd->PlayerShowRect(id, mPlayerWindows[mPrimePlayerId]);
    } else if (id == mPrimePlayerId) {
      mBackWnd->PlayerShowRect(id, mPlayerWindows[mSelectedPlayerId]);
    } else if (mPlayerWindows.contains(id)) {
      mBackWnd->PlayerShowRect(id, mPlayerWindows[id]);
    } else {
      mBackWnd->PlayerHide(id);
    }
    break;

  case eFullscreen:
    if (id == mSelectedPlayerId) {
      mBackWnd->PlayerShowRect(id, mScreenRect);
    } else {
      mBackWnd->PlayerHide(id);
    }
    break;

  case eModeIllegal:
    break;
  }
}

void CtrlWnd::UpdateMonitorsInfo()
{
  FileSettings settings;
  if (!settings.Open("./.arm")) {
    Log.Warning(QString("Open arm settings fail"));
    return;
  }
  QString guid = settings.GetValue("GUID").toString();

  ObjectItemS item;
  if (!mObjectTable->GetObjectByGuid(guid, item)) {
    Log.Warning(QString("Read arm Id fail (guid: '%1')").arg(guid));
    return;
  }
  int id = item->Id;

  QList<ArmMonitorsS> monitors;
  if (!mArmMonitorsTable->Select(QString("WHERE _object=%1").arg(id), monitors)) {
    Log.Warning(QString("Read arm monitors fail (id: %1)").arg(id));
    return;
  }

  QDesktopWidget* desktop = QApplication::desktop();
  int screenCount = desktop->screenCount();
  for (int i = 0; i < screenCount; i++) {
    int monNumber = i + 1;
    QRect screenRect = desktop->screenGeometry(i);
    QString monitorName = QString("Monitor %1 (%2, %3), (%4, %5)")
        .arg(monNumber).arg(screenRect.left()).arg(screenRect.top()).arg(screenRect.width()).arg(screenRect.height());
    Log.Info(monitorName);

    ArmMonitorsS newMonitor(new ArmMonitors());
    newMonitor->Object = id;
    newMonitor->Name = monitorName;
    newMonitor->Descr = "";
    newMonitor->Num = monNumber;
    newMonitor->Width = screenRect.width();
    newMonitor->Height = screenRect.height();
    newMonitor->Size = QPoint(0, 0);
    newMonitor->Used = false;

    for (auto itr = monitors.begin(); itr != monitors.end(); itr++) {
      const ArmMonitorsS& monitor = *itr;
      if (monitor->Num == newMonitor->Num) {
        if (monitor->Name == newMonitor->Name && monitor->Width == newMonitor->Width && monitor->Height == newMonitor->Height) {
          newMonitor.clear();
        } else {
          monitor->Name = newMonitor->Name;
          monitor->Width = newMonitor->Width;
          monitor->Height = newMonitor->Height;
          newMonitor = monitor;
        }
        break;
      }
    }

    if (newMonitor) {
      if (newMonitor->Id) {
        mArmMonitorsTable->Update(newMonitor);
      } else {
        mArmMonitorsTable->Insert(newMonitor);
      }
    }
  }

  for (auto itr = monitors.begin(); itr != monitors.end(); itr++) {
    const ArmMonitorsS& monitor = *itr;
    if (monitor->Num > screenCount) {
      if (monitor->Width != 0 || monitor->Height != 0) {
        monitor->Width = 0;
        monitor->Height = 0;
        mArmMonitorsTable->Update(monitor);
      }
    }
  }
}


CtrlWnd::CtrlWnd(const Db& _Db)
  : mDb(_Db), mObjectTable(new ObjectTable(_Db)), mArmMonitorsTable(new ArmMonitorsTable(_Db))
  , mArmMode(eNormal), mSelectedPlayerId(0), mPrimePlayerId(0)
  , mStateShmem(kArmDaemon), mArmState(nullptr)
{
  QDesktopWidget* desktop = qApp->desktop();
  int primaryMonitor = desktop->primaryScreen();
  mScreenRect = desktop->screenGeometry(primaryMonitor);
  mBackWnd = BackWndS(new BackWnd(this, mScreenRect));
}

CtrlWnd::~CtrlWnd()
{
}
