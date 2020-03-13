#include <Lib/Log/Log.h>

#include "ModuleLoaderA.h"
#include "ModuleInfo.h"
#include "ProcessManager.h"


const int kWorkPeriodMs = 100;
const int kWorkFailMs = 60 * 1000;

bool ModuleLoaderA::DoCircle()
{
  if (!UpdateModules() && !mPendingUpdate) {
    return true;
  } else if (IsStop()) {
    return false;
  }

  QMutexLocker lock(&mProcessManager->GetMainShmemMutex());
  for (auto itr = mRemoveModules.begin(); itr != mRemoveModules.end(); itr++) {
    mProcessManager->UnregisterModule(*itr);
  }
  mRemoveModules.clear();

  for (auto itr = mAddModules.begin(); itr != mAddModules.end(); itr++) {
    mProcessManager->RegisterModule(*itr);
  }

  for (auto itr = mUpdateModules.begin(); itr != mUpdateModules.end(); itr++) {
    mProcessManager->RestartModule(*itr);
  }
  mUpdateModules.clear();
  mAddModules.clear();

  mPendingUpdate = false;
  return true;
}

void ModuleLoaderA::AddModule(int id, const QString& path, const QStringList& params, const QString& uri)
{
  ModuleLoadInfoS info(new ModuleLoadInfo);
  info->Id = id;
  info->Path = path;
  info->Params = params;
  info->Params.push_front(QString("--uri=%1").arg(uri));
  info->Params.push_front(QString("--id=%1").arg(id));
  mAddModules.append(info);
  mPendingUpdate = true;
}

void ModuleLoaderA::UpdateModule(int id, const QString& path, const QStringList& params, const QString& uri)
{
  ModuleLoadInfoS info(new ModuleLoadInfo);
  info->Id = id;
  info->Path = path;
  info->Params = params;
  info->Params.push_front(QString("--uri=%1").arg(uri));
  info->Params.push_front(QString("--id=%1").arg(id));
  mUpdateModules.append(info);
  mPendingUpdate = true;
}

void ModuleLoaderA::RemoveModule(int id)
{
  mRemoveModules.append(id);
  mPendingUpdate = true;
}

void ModuleLoaderA::AddTemporaryModule(int id, const QString& path, const QStringList& params, const QString& uri)
{
  ModuleLoadInfoS info(new ModuleLoadInfo);
  info->Id = id;
  info->Path = path;
  info->Params = params;
  info->Params.push_front(QString("--uri=%1").arg(uri));
  info->Params.push_front(QString("--id=%1").arg(id));

  QMutexLocker lock(&mProcessManager->GetMainShmemMutex());
  mProcessManager->RegisterModule(info);
  mRemoveModules.append(id);
}

void ModuleLoaderA::ConnectModule(CtrlWorker *_other)
{
  if (ProcessManager* _ProcessManager = dynamic_cast<ProcessManager*>(_other)) {
    mProcessManager = _ProcessManager;
  }
}


ModuleLoaderA::ModuleLoaderA()
  : CtrlWorker(kWorkPeriodMs, true, true, kWorkFailMs)
  , mProcessManager(nullptr)
{
}

ModuleLoaderA::~ModuleLoaderA()
{
}


