#include <qsystemdetection.h>

#include <Lib/Include/QtAppWin.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/Include/ModuleNames.h>

#include <LibV/CtrlV/CtrlV.h>


int qmain(int argc, char* argv[])
{
  OverseerS overseer;
  DbS db;
  if (int ret = Overseer::ParseMainWithDb(kArmDaemon, argc, argv, overseer, db)) {
    return ret;
  }

  CtrlVS ctrl(new CtrlV(true));
  overseer->RegisterWorker(ctrl);

  return overseer->Run();
}

