#include <QFile>

#include <Lib/Include/License.h>
#undef LICENSE_MAIN
#define LICENSE_MAIN(XXX)
#include <Lib/Include/QtAppCon.h>
#include <Lib/Db/Db.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Local/ModuleNames.h>

#include "LiLoader.h"


int qmain(int argc, char* argv[])
{
  OverseerS overseer;
  if (int ret = Overseer::ParseMain(kServerDaemon, argc, argv, overseer)) {
    return ret;
  }

  LiLoaderS lilo(new LiLoader());
  overseer->RegisterWorker(lilo);

  return overseer->Run();
}

