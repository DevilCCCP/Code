#include <QSettings>

#include <Lib/Include/QtAppCon.h>
#include <Lib/Db/Db.h>
#include <Lib/Settings/DbSettings.h>
#include <Lib/Dispatcher/OverseerState.h>
#include <Lib/NetServer/NetServer.h>
#include <Local/ModuleNames.h>

#include "UniteHandler.h"
#include "UniteInfo.h"
#include "UniteAgent.h"


typedef HandlerManagerC<UniteHandler, UniteInfo> UniteManager;
typedef QSharedPointer<UniteManager> UniteManagerS;

int qmain(int argc, char* argv[])
{
  DbS db;
  OverseerStateS overseer;
  if (int ret = OverseerState::ParseMainWithDb(kServerDaemon, argc, argv, overseer, db)) {
    return ret;
  }
  DbStateNotifierS notifier = overseer->GetNotifier();

  DbSettings settings(*db);
  if (!settings.Open(QString::number(overseer->Id()))) {
    return -2100;
  }

//  INSERT INTO object_settings(_object, key, value) VALUES (14, 'Slave', '0');
//  INSERT INTO object_settings(_object, key, value) VALUES (14, 'Uri', 'http://<server>:<port>');
//  INSERT INTO object_settings(_object, key, value) VALUES (14, 'Master', '0');
//  INSERT INTO object_settings(_object, key, value) VALUES (14, 'Port', '8084');

  UniteInfoS un;
  bool primaryUsed = false;
  if (settings.GetValue("Master", false).toBool()) {
    un.reset(new UniteInfo(overseer.data(), *db));
    if (!un->Init()) {
      return -2200;
    }
    int port = settings.GetValue("Port", 8084).toInt();
    NetServerS server = NetServerS(new NetServer(port, UniteManagerS(new UniteManager(un.data()))));
    server->SetNotifier(notifier);
    primaryUsed = true;
    overseer->RegisterWorker(server);
  }

  UniteAgentS agent;
  if (settings.GetValue("Slave", false).toBool()) {
//    INSERT INTO object_settings(_object, key, value) VALUES (14, 'Slave', '0');
//    INSERT INTO object_settings(_object, key, value) VALUES (14, 'Uri', 'http://<server>:<port>');
//    INSERT INTO object_settings(_object, key, value) VALUES (14, 'Period', '60');
    QString uri = settings.GetValue("Uri", "bad uri").toString();
    int periodMs = qMax(24 * 60 * 60, settings.GetValue("Period", 60).toInt()) * 1000;
    agent.reset(new UniteAgent(uri, periodMs));
    if (!primaryUsed) {
      agent->SetPrimary(db, notifier);
      primaryUsed = true;
    }
    overseer->RegisterWorker(agent);
  }

  if (!primaryUsed) {
    notifier->NotifyNothing();
    db.clear();
  }

  return overseer->Run();
}

