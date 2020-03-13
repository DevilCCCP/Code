#include <Lib/Include/QtAppCon.h>
#include <Lib/Db/Db.h>
#include <Lib/Settings/FileSettings.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Updater/Updater.h>
#include <Local/ModuleNames.h>


int qmain(int argc, char* argv[])
{
  DbS db;
  OverseerS overseer;
  if (int ret = Overseer::ParseMainWithDb(kUpdateDaemon, argc, argv, overseer, db)) {
    return ret;
  }

  UpdaterS updater(new Updater(db));
  overseer->RegisterWorker(updater);

  db->MoveToThread(updater.data());
  return overseer->Run();
}

