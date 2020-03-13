#include <Lib/Include/QtAppGui.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Ctrl/ManagerThread.h>
#include <Lib/Updater/UpInfo.h>
#include <Local/ModuleNames.h>

#include "MainWindowZ.h"


const QString GetProgramName() { return GetAdminName(); }

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

#ifdef QT_NO_DEBUG
  Log.SetFileLogging();
#endif

  ManagerThread manager;
  manager.start();

  UpInfo upInfo;
  Db db(true);
  if (int ret = ConnectPrimaryDb(db, &upInfo)) {
    return ret;
  }

  MainWindowZ w(db, &upInfo, manager.GetCtrlManager());
  w.setWindowTitle(GetAdminName());
  w.show();

  int ret = qApp->exec();
  manager.Stop();
  manager.wait(3000);
  return ret;
}

