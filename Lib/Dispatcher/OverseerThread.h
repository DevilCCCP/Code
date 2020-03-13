#pragma once

#include <QThread>
#include <QCoreApplication>

#include "Overseer.h"


DefineClassS(OverseerThread);
DefineClassS(Overseer);

//// Common usage:
////OverseerThreadS overseerTh(new OverseerThread(overseer));
////overseerTh->start();
////int ret = qApp->exec();
////overseerTh->wait();
////return ret;

class OverseerThread: public QThread
{
  OverseerS mOverseer;

public:
  const OverseerS& GetOverseer() { return mOverseer; }

protected:
  /*override */virtual void run() Q_DECL_OVERRIDE
  {
    mOverseer->Run();
    mOverseer.clear();

    QCoreApplication::quit();
  }

public:
  void Stop()
  {
    if (OverseerS ov = mOverseer) {
      ov->Stop();
    }
  }

public:
  OverseerThread(const OverseerS& _Overseer)
    : mOverseer(_Overseer)
  {
  }
};
