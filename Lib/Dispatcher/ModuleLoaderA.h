#pragma once

#include <QSharedPointer>
#include <QList>
#include <QString>
#include <QStringList>
#include <QMap>
#include <Lib/Ctrl/CtrlWorker.h>

DefineClassS(ProcessManager);
DefineStructS(ModuleLoadInfo);

class ModuleLoaderA : public CtrlWorker
{
  ProcessManager*        mProcessManager;
  QList<ModuleLoadInfoS> mAddModules;
  QList<ModuleLoadInfoS> mUpdateModules;
  QList<int>             mRemoveModules;
  bool                   mPendingUpdate;

public:
  /*override */virtual const char* Name() override { return "ModuleLoader"; }
  /*override */virtual const char* ShortName() override { return "L"; }
private:
  /*override */virtual bool DoCircle() override;
protected:
  /*new */virtual bool UpdateModules() = 0;

protected:
  void AddModule(int id, const QString& path, const QStringList& params, const QString& uri);
  void UpdateModule(int id, const QString& path, const QStringList& params, const QString& uri);
  void RemoveModule(int id);
  void AddTemporaryModule(int id, const QString& path, const QStringList& params, const QString& uri);

public:
  /*override */virtual void ConnectModule(CtrlWorker* _other) override;

public:
  ModuleLoaderA();
  /*override */virtual ~ModuleLoaderA();
};

