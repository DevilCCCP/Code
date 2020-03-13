#include <QDesktopWidget>
#include <qsystemdetection.h>

#include <Lib/Settings/FileSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Log/Log.h>
#include <LibV/Include/ModuleNames.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectType.h>
#include <Lib/Db/ArmMonitors.h>

#include "CtrlV.h"
#ifdef Q_OS_WIN32
#include "Win/BackWnd.h"
#endif


const int kLayoutCamerasMin0 = 4;
const int kLayoutCamerasMin2 = 3;
const int kLayoutCamerasMin3 = 6;
const int kLayoutCamerasDefault = 5;
const int kLayoutCamerasMax = 8;

bool CtrlV::DoInit()
{
  mDb.reset(new Db());
  if (!mDb->OpenDefault()) {
    Log.Fatal("Open Db info fail", true);
  }
  if (!GetOverseer()->Detached()) {
    if (mStateShmem.create(sizeof(ArmState))) {
      Log.Warning("Create new arm state shmem");
      memset(mStateShmem.data(), 0, sizeof(ArmState));
    } else if (!mStateShmem.attach()) {
      Log.Fatal("Can't attach StateShmem", true);
    } else if (mStateShmem.size() < (int)sizeof(ArmState)) {
      Log.Warning(QString("StateShmem has invalid size (found: %1, must: %2)").arg(mStateShmem.size()).arg(sizeof(ArmState)));
    }
    mArmState = static_cast<ArmState*>(mStateShmem.data());
  } else {
    Log.Info("Work in detached mode");
  }

  mBackWnd = BackWndS(new BackWnd(this, mScreenRect, mCtrlTools));
  if (!mBackWnd->Init()) {
    return false;
  }

  DoLayout(ePreloadLayout, 0, 0, 0);
  return true;
}

bool CtrlV::DoCircle()
{
  if (!mPrepared) {
    Prepare();
  }

  if (mBackWnd) {
    mBackWnd->DoCircle();
  }
  return true;
}

void CtrlV::DoRelease()
{
  if (mBackWnd) {
    mBackWnd->Release();
    mBackWnd.clear();
  }
}

void CtrlV::Stop()
{
  if (IsAlive()) {
    emit OnStop();
  }

  Imp::Stop();
}

void CtrlV::RegisterPlayerType(int id, bool isPrime)
{
  if (isPrime) {
    if (mPrimePlayerId && mPrimePlayerId != id) {
      Log.Warning(QString("Prime player changed %1 -> %2").arg(mPrimePlayerId).arg(id));
    }
    mPrimePlayerId = id;
  }
}

void CtrlV::RegisterPlayerTopLeft(int id, const QPoint &point)
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

void CtrlV::RegisterPlayerBottomRight(int id, const QPoint &point)
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
      if (mBackWnd) {
        mBackWnd->PlayerShowRect(id, mPlayerWindows[id]);
      }
      mPlayerWindows.remove(id);
    }
  }
}

void CtrlV::MovePlayer(int id)
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

void CtrlV::DoExit()
{
  Log.Info("Exit pushed");
  if (mArmState) {
    mArmState->Signal |= (int)ePowerOff;
  }
}

void CtrlV::DoLayout(ELayoutType id, int count, int monitor, int cameraGroup, qint64 ts)
{
  if (!mArmState) {
    return;
  }

  mArmState->LayoutType = id;
  switch (id) {
  case eNoPrime:
    mArmState->LayoutCount = (count >= 0? count: qBound(kLayoutCamerasMin0, mLayoutCameras, kLayoutCamerasMax));
    break;
  case ePrimeHv:
    mArmState->LayoutCount = (count >= 0? count: qBound(kLayoutCamerasMin3, mLayoutCameras, kLayoutCamerasMax));
    break;
  case ePrimeHorz: case ePrimeVert:
    mArmState->LayoutCount = (count >= 0? count: qBound(kLayoutCamerasMin2, mLayoutCameras, kLayoutCamerasMax));
    break;
  default:
    mArmState->LayoutCount = count;
    break;
  }
  mArmState->LayoutMonitor1 = monitor;
  mArmState->CameraGroup    = cameraGroup;
  mArmState->Timestamp      = ts;
  ApplyLayout();

  mArmMode = eNormal;
  mSelectedPlayerId = 0;
  mPrimePlayerId = 0;
  mPlayerWindows.clear();
}

void CtrlV::DoSwitchDesktop()
{
  Log.Info("Desktop show/hide switch");
  mArmState->Signal = mArmState->Signal ^ eHideDesktop;
}

void CtrlV::DoSwitchOther()
{
  Log.Info("Other show/hide switch");
  mArmState->Signal = mArmState->Signal ^ eHideOther;
}

bool CtrlV::Prepare()
{
  if (!mDb->Connect()) {
    return false;
  }

  if (InitLayoutsInfo() && InitMonitorsInfo()) {
    mPrepared = true;
    mDb.clear();
  } else {
    return false;
  }
  return true;
}

bool CtrlV::InitLayoutsInfo()
{
  auto q = mDb->MakeQuery();
  q->prepare("SELECT COUNT(o._id) FROM object o"
             " JOIN object_type t ON o._otype = t._id"
             " JOIN object_connection c ON c._oslave = o._id"
             " JOIN object o2 ON o2._id = c._omaster"
             " JOIN object_type t2 ON o2._otype = t2._id"
             " WHERE t.name = 'cam' AND t2.name = 'srv';");
  if (!mDb->ExecuteQuery(q) || !q->next()) {
    return false;
  }
  mLayoutCameras = q->value(0).toInt();
  return true;
}

bool CtrlV::InitMonitorsInfo()
{
  ObjectTable      objectTable(*mDb);
  ArmMonitorsTable armMonitorsTable(*mDb);

  FileSettings settings;
  if (!settings.OpenLocal(kArmDaemon)) {
    Log.Error(QString("Open arm settings fail"));
    DoExit();
    return false;
  }
  QString guid = settings.GetValue("GUID").toString();

  ObjectItemS item;
  if (!objectTable.GetObjectByGuid(guid, item) || !item) {
    Log.Error(QString("Read arm Id fail (guid: '%1')").arg(guid));
    DoExit();
    return false;
  }
  int id = item->Id;

  QList<ArmMonitorsS> monitors;
  if (!armMonitorsTable.Select(QString("WHERE _object=%1").arg(id), monitors)) {
    Log.Warning(QString("Read arm monitors fail (id: %1)").arg(id));
    return false;
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
        armMonitorsTable.Update(newMonitor);
      } else {
        armMonitorsTable.Insert(newMonitor);
      }
    }
  }

  for (auto itr = monitors.begin(); itr != monitors.end(); itr++) {
    const ArmMonitorsS& monitor = *itr;
    if (monitor->Num > screenCount) {
      if (monitor->Width != 0 || monitor->Height != 0) {
        monitor->Width = 0;
        monitor->Height = 0;
        armMonitorsTable.Update(monitor);
      }
    }
  }
  return true;
}

void CtrlV::ApplyLayout()
{
  Log.Info(QString("Layout pushed (id: %1, count: %2, monitor: %3, group: %4, ts: %5(%6))")
           .arg(mArmState->LayoutType).arg(mArmState->LayoutCount)
           .arg(mArmState->LayoutMonitor1).arg(mArmState->CameraGroup)
           .arg(mArmState->Timestamp).arg(QDateTime::fromMSecsSinceEpoch(mArmState->Timestamp).toString()));
  volatile int counter = mArmState->LayoutCounter;
  counter++;
  mArmState->LayoutCounter = counter;
  mArmState->LayoutEndCounter = counter;
}

void CtrlV::UpdatePlayerAll()
{
  if (!mBackWnd) {
    return;
  }

  for (auto itr = mPlayerWindows.begin(); itr != mPlayerWindows.end(); itr++) {
    UpdatePlayer(itr.key());
  }
}

void CtrlV::UpdatePlayer(int id)
{
  if (!mBackWnd) {
    return;
  }

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


CtrlV::CtrlV(bool _CtrlTools, int _Monitor)
  : Imp(2)
  , mDb(new Db()), mCtrlTools(_CtrlTools)
  , mArmMode(eNormal), mSelectedPlayerId(0), mPrimePlayerId(0), mLayoutCameras(kLayoutCamerasDefault), mPrepared(false)
  , mStateShmem(kArmDaemon), mArmState(nullptr)
{
  if (!mDb->OpenDefault()) {
    Log.Fatal("Open Db info fail", true);
  }
  QDesktopWidget* desktop = qApp->desktop();
  if (_Monitor) {
    mScreenRect = desktop->screenGeometry(_Monitor - 1);
  } else {
    mScreenRect = desktop->screenGeometry(desktop->primaryScreen());
  }
}

CtrlV::~CtrlV()
{
}
