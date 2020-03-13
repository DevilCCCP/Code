#include "stdafx.h"

#include <Lib/Include/QtAppGui.h>
#include <Lib/Db/Db.h>
#include <Lib/Db/ObjectState.h>
#include <LibV/Include/ModuleNames.h>
#include <Lib/Ctrl/ManagerThread.h>

#include "MainWindowY.h"


const QString GetProgramName()
{ return QString::fromUtf8("АРМ администратора СНА"); }

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

#ifdef QT_NO_DEBUG
  Log.SetFile(QString("./Log/!%1.log").arg(kAdminUi));
#endif

  ManagerThread manager;
  manager.start();

  Db db(true);
  if (int ret = ConnectPrimaryDb(db)) {
    return ret;
  }

  MainWindowY w(db, manager.GetCtrlManager());
  w.setWindowTitle(GetProgramName());
  w.show();

  return qApp->exec();
}

