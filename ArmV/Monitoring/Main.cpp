#include <Lib/Include/QtAppGui.h>
#include <Lib/Db/Db.h>
#include <Lib/Log/Log.h>
#include <Lib/Db/ObjectState.h>
#include <Lib/Updater/UpInfo.h>
#include <Lib/Monitoring/MonitoringWindow.h>
#include <Local/ModuleNames.h>


const QString GetProgramName()
{ return QString::fromUtf8("АРМ мониторинга"); }

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

#ifdef USE_EVENTS
  MonitoringWindow w(db, &upInfo, true);
#else
  MonitoringWindow w(db, &upInfo, false);
#endif
  w.setWindowTitle(GetProgramName());
  w.show();

  return qApp->exec();
}

