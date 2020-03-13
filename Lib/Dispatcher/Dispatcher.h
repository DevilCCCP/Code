#pragma once

#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Settings/SettingsA.h>

#include "ModuleLoaderA.h"


DefineClassS(ProcessManager);
DefineClassS(ModuleLoaderA);
DefineClassS(Dispatcher);

// Модуль отвечает за работу процессов системы
class Dispatcher: public CtrlManager
{
  const QString   mProgramName;

  ProcessManagerS mProcessManager;
  ModuleLoaderAS  mModuleLoader;

protected:
  /*new */virtual QSharedPointer<ModuleLoaderA> GetModuleLoader() = 0;

public:
  int RunService(const QString &_Name, const QString &_Viewname, const QString &_Description, int argc, char* argv[]);
  bool Init(bool console);

public:
  Dispatcher(const QString& _ProgramName);
  /*override */virtual ~Dispatcher();
};

template<typename ModuleLoaderT>
class DispatcherDaemon: public Dispatcher
{
  SettingsAS mSettings;

private:
  /*override */virtual QSharedPointer<ModuleLoaderA> GetModuleLoader() Q_DECL_OVERRIDE
  { return QSharedPointer<ModuleLoaderA>(new ModuleLoaderT(mSettings)); }

public:
  DispatcherDaemon(const char* _kDaemonName, const SettingsAS& _Settings)
    : Dispatcher(_kDaemonName), mSettings(_Settings)
  { }
};

class ModuleLoaderB: public ModuleLoaderA
{
  SettingsAS mSettings;

protected:
  template<typename ModuleLoaderT>
  DispatcherDaemon<ModuleLoaderT>* GetDaemon() { return static_cast<DispatcherDaemon<ModuleLoaderT>*>(GetManager()); }
  SettingsA* GetSettings() { return mSettings.data(); }

  ModuleLoaderB(SettingsAS& _Settings)
    : mSettings(_Settings)
  { }
};

