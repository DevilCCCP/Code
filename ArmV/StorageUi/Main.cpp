#include <Lib/Include/QtAppGui.h>
#include <Lib/Db/Db.h>
#include <Lib/Log/Log.h>
#include <Lib/Updater/UpInfo.h>
#include <Local/ModuleNames.h>

#include "MainWindow.h"

const QString GetProgramName()
{ return QString::fromUtf8("АРМ переноса архива"); }

int qmain(int argc, char* argv[])
{
  Q_UNUSED(argc);
  Q_UNUSED(argv);

#ifdef QT_NO_DEBUG
  Log.SetFileLogging();
#endif

  UpInfo upInfo;
  Db db(true);
  if (int ret = ConnectPrimaryDb(db, &upInfo)) {
    return ret;
  }

  MainWindow w(db, &upInfo);
  w.setWindowTitle(GetProgramName());
  w.show();

  return qApp->exec();
}

