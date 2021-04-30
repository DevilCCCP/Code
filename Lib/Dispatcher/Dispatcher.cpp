#include <qsystemdetection.h>
#include <QFile>
#include <QDir>

#include <Lib/Log/Log.h>
#include <Lib/Include/License.h>
#include <Lib/Common/Version.h>
#include <Local/ModuleNames.h>

#include "Dispatcher.h"
#include "ProcessManager.h"

#ifdef Q_OS_WIN32
#include "Win/WinService.h"
#else
#include "Linux/LinuxService.h"
#endif


const int kProcessManagerStopMs = 7000;

int Dispatcher::RunService(const QString& _Name, const QString& _Viewname, const QString& _Description, int argc, char *argv[])
{
  Version ver;
  if (ver.LoadFromThis()) {
    Log.Info(QString("Version %1").arg(ver.ToString()));
  } else {
    Log.Warning(QString("Version unknown"));
  }

  QString param = (argc > 1)? argv[1]: "";

  bool skipLicenceTest = false;
  for (int i = 0; i < argc; i++) {
    if (QString(argv[i]) == "--skip-license") {
      skipLicenceTest = true;
    }
  }

#ifdef Q_OS_WIN32
  WinService service(_Name, _Viewname, _Description, this);
#else
  LinuxService service(_Name, _Viewname, _Description, this);
#endif
  if (param == "run" || param == "") {
    if (service.Run()) {
      return 0;
    } else {
      return -1;
    }
  }

  if (!skipLicenceTest) {
    LICENSE_MAIN(false);
  }

  if (param == "install") {
    if (service.Install()) {
      return 0;
    } else {
      return -2;
    }
  } else if (param == "uninstall") {
    if (service.Uninstall()) {
      return 0;
    } else {
      return -3;
    }
  } else if (param == "start") {
    if (service.Start()) {
      return 0;
    } else {
      return -4;
    }
  } else if (param == "stop") {
    if (service.Stop()) {
      return 0;
    } else {
      return -5;
    }
  } else if (param == "restart") {
    if (service.Restart()) {
      return 0;
    } else {
      return -5;
    }
  }
  return -1;
}

bool Dispatcher::Init(bool console)
{
  if (QFile::exists(kLicenseExe)) {
    if (QFile::remove(kLicenseExe)) {
      Log.Warning(QString("Remove license generator"));
    } else {
      Log.Error(QString("Remove license generator fail"));
    }
  }

  if (console) {
    SetConsoleBreak();
  }

  mProcessManager = QSharedPointer<ProcessManager>(new ProcessManager(mProgramName));
  if (!mProcessManager->Init()) {
    return false;
  }
  RegisterWorker(mProcessManager);
  mModuleLoader = GetModuleLoader();
  mModuleLoader->ConnectModule(mProcessManager.data());
  RegisterWorker(mModuleLoader);
  return true;
}

Dispatcher::Dispatcher(const QString& _ProgramName)
  : CtrlManager(false, kProcessManagerStopMs)
  , mProgramName(_ProgramName)
{
}

Dispatcher::~Dispatcher()
{
}

