#include <Lib/Include/QtAppCon.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Local/ModuleNames.h>

#include "Creator.h"


int qmain(int argc, char* argv[])
{
  OverseerS overseer;
  if (int ret = Overseer::ParseMain(kServerDaemon, argc, argv, overseer)) {
    return ret;
  }

  CreatorS creator(new Creator());
  overseer->RegisterWorker(creator);

  return overseer->Run();
}

