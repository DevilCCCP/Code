#include "Pinger.h"

#include <QElapsedTimer>

#include <Lib/Net/Chater.h>
#include <Lib/Log/Log.h>

bool Pinger::DoInit()
{
  return true;
}

bool Pinger::DoCircle()
{
  QElapsedTimer timer;
  timer.start();
  if (!mChater) {
    mChater = Chater::CreateChater(GetManager(), mDestUri);
  }
  if (mChater->SendPingRequest()) {
    Log.Info(QString("Ping done in %1 ms").arg(timer.restart()));
  } else if (IsAlive()) {
    Log.Info(QString("Ping fail %1 ms").arg(timer.restart()));
    mChater.clear();
  } else {
    return false;
  }
  return IsAlive();
}

Pinger::Pinger(const Uri &_DestUri)
  : CtrlWorker(1000)
  , mDestUri(_DestUri)
{
}



