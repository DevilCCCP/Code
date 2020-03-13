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


#ifdef QT_NO_DEBUG
const int kMinPeriodMs = 60 * 1000;
#else
const int kMinPeriodMs = 5 * 1000;
#endif
const int kMaxPeriodMs = 24 * 60 * 60 * 1000;
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
  NetServerS server;
  bool primaryUsed = false;
  if (settings.GetValue("Master", false).toBool()) {
    un.reset(new UniteInfo(overseer.data(), *db));
    if (!un->Init()) {
      return -2200;
    }
    int port = settings.GetValue("Port", 8084).toInt();
    server.reset(new NetServer(port, UniteManagerS(new UniteManager(un.data()))));
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
    int periodMs = qBound(kMinPeriodMs, settings.GetValue("Period", 60 * 1000).toInt(), kMaxPeriodMs);
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
  if (server && db) {
    db->MoveToThread(server.data());
  }
  return overseer->Run();
}

