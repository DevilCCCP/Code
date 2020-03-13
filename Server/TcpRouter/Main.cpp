#include <Lib/Include/QtAppCon.h>
#include <Lib/Db/Db.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Dispatcher/OverseerThread.h>
#include <Lib/Router/RouterServer.h>
#include <Lib/Router/RouterClient.h>
#include <Lib/Log/Log.h>
#include <Local/ModuleNames.h>


int qmain(int argc, char* argv[])
{
  DbS db;
  OverseerS overseer;
  if (int ret = Overseer::ParseMainWithDb(kServerDaemon, argc, argv, overseer, db)) {
    return ret;
  }

  DbSettings settings(*db);
  if (!settings.Open(QString::number(overseer->Id()))) {
    return 1;
  }
  bool typeServer = settings.GetMandatoryValue("Server", true).toBool();
  int port = settings.GetValue("Port", 20022).toInt();
  settings.SetSilent(true);
  bool debug = settings.GetValue("Debug", false).toBool();
  settings.SetSilent(false);
  if (typeServer) {
    RouterServer* router = new RouterServer(qApp);
    router->setDebug(debug);
    if (!router->Start(port)) {
      Log.Warning(QString("Server start fail, port: %1").arg(port));
      return 3;
    }
    Log.Info(QString("Server started at port: %1").arg(port));
  } else {
    QString host = settings.GetValue("Host", "localhost").toString();
    int hostPort = settings.GetValue("HostPort", 20022).toInt();
    RouterClient* router = new RouterClient(qApp);
    router->setDebug(debug);
    if (!router->Start(port, host, hostPort)) {
      Log.Warning(QString("Client start fail, host: %1:%2").arg(host).arg(hostPort));
      return 3;
    }
    Log.Info(QString("Client started (port: %1, host: %2:%3)").arg(port).arg(host).arg(hostPort));
  }

  OverseerThreadS overseerTh(new OverseerThread(overseer));
  overseerTh->start();

  int ret = qApp->exec();
  overseerTh->wait();
  return ret;
}
