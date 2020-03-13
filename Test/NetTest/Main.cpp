#include <QSettings>

#include <Lib/Include/QtAppCon.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Net/Listener.h>
#include <Lib/Net/Chater.h>

#include "Pinger.h"


int qmain(int argc, char* argv[])
{
  QString path = (argc == 2)? QString(argv[1]): qApp->applicationDirPath() + "/.NetSettings";
  QSettings settingFile(path, QSettings::IniFormat);

  QVariant devil = settingFile.value("Devil");
  bool listen = settingFile.value("Listen", true).toBool();
  int port = settingFile.value("Port", 0).toInt();
  QString host = settingFile.value("Host", "localhost").toString();
  Uri uri(Uri::eTcp, host, port);
  if (devil.isNull()) {
    Log.Info("Create default settings");
    settingFile.setValue("Devil", "exists");
    settingFile.setValue("Listen", true);
    settingFile.setValue("Port", 0);
    settingFile.setValue("Host", "localhost");
    settingFile.sync();
    return 0;
  }

  CtrlManager manager(true);
  if (listen) {
    Log.Info(QString("Listen mode (port: %1)").arg(port));
    ListenerS listener = ListenerS(new Listener(port));
    manager.RegisterWorker(listener);
  } else {
    Log.Info(QString("Send mode (host: %1; port: %2)").arg(host).arg(port));
    PingerS pinger = PingerS(new Pinger(uri));
    manager.RegisterWorker(pinger);
  }

  return manager.Run();
}

