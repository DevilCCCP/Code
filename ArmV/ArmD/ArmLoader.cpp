#include <Lib/Log/Log.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Db/ObjectType.h>
#include <LibV/Include/ModuleNames.h>
#include <Lib/Dispatcher/ModuleDb.h>

#include "ArmLoader.h"


bool ArmLoader::DoInit()
{
  if (mStateShmem.create(sizeof(ArmState))) {
    Log.Info(QString("Create new arm state shmem (size: %1, must: %2)").arg(mStateShmem.size()).arg(sizeof(ArmState)));
    memset(mStateShmem.data(), 0, sizeof(ArmState));
  } else if (!mStateShmem.attach()) {
    Log.Fatal("Can't attach StateShmem", true);
    return false;
  } else if (mStateShmem.size() < (int)sizeof(ArmState)) {
    Log.Fatal("StateShmem has invalid size", true);
    return false;
  } else if (mStateShmem.size() > (int)sizeof(ArmState)) {
    Log.Warning(QString("StateShmem has wrong size (shmem: %1, struct: %2)").arg(mStateShmem.size()).arg(sizeof(ArmState)));
  }

  return ModuleLoaderD::DoInit();
}

bool ArmLoader::IsNeedIp()
{
  return false;
}

void ArmLoader::RegisterStatic()
{
  RegisterStaticModule(kControlExe, QStringList() << QString("-parm=%1").arg(MainObject()->Id), eNormalState);
}

bool ArmLoader::InitializeExtra()
{
  return GetObjectTypeId("srv", &mSrvTypeId) && GetObjectTypeId("cam", &mCamTypeId) && GetObjectTypeId("aca", &mCam2TypeId);
}

bool ArmLoader::CheckUpdateObject()
{
  mArmState = *static_cast<ArmState*>(mStateShmem.data());

  if (mArmState.Signal & ePowerOff) {
    GetManager()->Stop();
    return false;
  }

  if (mArmState.LayoutCounter == mLayoutCounter) {
    return ModuleLoaderD::CheckUpdateObject();
  }

  mRevision++;
  do {
    mLayoutCounter = mArmState.LayoutCounter;
    mArmState = *static_cast<ArmState*>(mStateShmem.data());
  } while (mLayoutCounter != mArmState.LayoutCounter || mLayoutCounter != mArmState.LayoutEndCounter);

  ModuleLoaderD::CheckUpdateObject();
  return true;
}

bool ArmLoader::LoadObject()
{
  bool ok;
  switch (mArmState.LayoutType) {
  case ePreloadLayout: ok = LoadDbArm(MainObject()->Id); break;
  case eNoPrime:
  case ePrimeHv:
  case ePrimeHorz:
  case ePrimeVert:     ok = LoadUserArm(); break;
  case eEmptyLayout:   ok = true; break;
  case eOneCamera:     ok = LoadUserCamera(); break;
  case eDbLayout:      ok = LoadDbArm(mArmState.CameraGroup); break;
  case eIllegalLayout:
  default:             ok = false; break;
  }

  if (!ok) {
    mLayoutCounter = 0;
  }
  return ok;
}

bool ArmLoader::LoadDbArm(int id)
{
  if (id == 0) {
    return true;
  }

  auto q = getDb()->MakeQuery();
  q->prepare(QString("SELECT l._id FROM arm_monitors m"
                     " INNER JOIN arm_monitor_layouts l ON l._amonitor = m._id"
                     " WHERE m._object = %1 AND m.used = TRUE").arg(id));
  if (!getDb()->ExecuteQuery(q)) {
    return false;
  }

  while (q->next()) {
    int id = q->value(0).toInt();
    ModuleDbS module(new ModuleDb());
    module->setId(id);
    module->setPath(kPlayerExe);
    module->setParams(QStringList());
    module->setRevision(mRevision);
    module->setState(0);
    module->setUri("");
    LoadModule(module);
  }
  return true;
}

bool ArmLoader::LoadUserArm()
{
  auto q = getDb()->MakeQuery();
  q->prepare(QString("SELECT cam._id FROM object srv"
                     " INNER JOIN object_connection c ON c._omaster = srv._id"
                     " INNER JOIN object cam ON cam._id = c._oslave AND cam._otype IN (%1, %2)"
                     " WHERE srv._otype = %3;")
             .arg(mCamTypeId).arg(mCam2TypeId).arg(mSrvTypeId));
  QList<int> cameras;
  if (!getDb()->ExecuteQuery(q)) {
    return false;
  }
  while (q->next()) {
    cameras.append(q->value(0).toInt());
  }

  auto itr = cameras.begin();
  for (int id = 1; id <= mArmState.LayoutCount; id++) {
    int camId;
    if (itr != cameras.end()) {
      camId = *itr;
      itr++;
    } else {
      camId = 0;
    }
    ModuleDbS module(new ModuleDb());
    module->setId(id);
    module->setPath(kPlayerExe);
    module->setParams(QStringList() << QString("-pcustom;id=%1;type=%2;cnt=%3;mon=%4;cam=%5;obj=%6")
                      .arg(id).arg((int)mArmState.LayoutType).arg(mArmState.LayoutCount)
                      .arg(mArmState.LayoutMonitor1).arg(camId).arg(MainObject()->Id));
    module->setRevision(mRevision);
    module->setState(0);
    module->setUri("");
    LoadModule(module);
  }
  return true;
}

bool ArmLoader::LoadUserCamera()
{
  ModuleDbS module(new ModuleDb());
  module->setId(1);
  module->setPath(kPlayerExe);
  module->setParams(QStringList() << QString("-pcustom;id=%1;type=%2;cnt=%3;mon=%4;cam=%5;obj=%6;ts=%7")
                    .arg(1).arg((int)eNoPrime).arg(mArmState.LayoutCount)
                    .arg(mArmState.LayoutMonitor1).arg(mArmState.CameraGroup)
                    .arg(MainObject()->Id).arg(mArmState.Timestamp));
  module->setRevision(mRevision);
  module->setState(0);
  module->setUri("");
  LoadModule(module);
  return true;
}


ArmLoader::ArmLoader(SettingsAS &_Settings)
  : ModuleLoaderD(_Settings, "arm", 30000)
  , mStateShmem(kArmDaemon), mLayoutCounter(0), mRevision(0)
{
  memset(&mArmState, 0, sizeof(mArmState));
}
