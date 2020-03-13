#pragma once

#include <QThread>

#include "CtrlManager.h"


DefineClassS(CtrlManager);

class ManagerThread: public QThread
{
  CtrlManagerS mCtrlManager;

public:
  const CtrlManagerS& GetCtrlManager() { return mCtrlManager; }

protected:
  /*override */virtual void run() Q_DECL_OVERRIDE
  {
    mCtrlManager->Run();
    mCtrlManager.clear();
  }

public:
  void Stop()
  {
    if (CtrlManagerS cm = mCtrlManager) {
      cm->Stop();
    }
  }

public:
  ManagerThread()
    : mCtrlManager(new CtrlManager(false))
  {
  }
};
