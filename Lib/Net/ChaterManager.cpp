#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "ChaterManager.h"


ChaterS ChaterManager::CreateChater(MessengerS &messenger)
{
  ChaterS chater = NewChater(messenger);
  chater->SetManager(this);
  QMutexLocker lock(&mRegisterMutex);
  mRegisterChaters.append(chater);
  return chater;
}

void ChaterManager::UnregisterChater(Chater *chater)
{
  QMutexLocker lock(&mRegisterMutex);

  for (auto itr = mRegisterChaters.begin(); itr != mRegisterChaters.end(); itr++) {
    ChaterS& c = *itr;
    if (c.data() == chater) {
      mRegisterChaters.erase(itr);
      return;
    }
  }
  lock.unlock();
  Log.Warning("Unregister unregistered chater");
}

ReceiverS ChaterManager::NewReceiver()
{
  return ReceiverS(new Receiver());
}

ChaterS ChaterManager::NewChater(MessengerS &messenger)
{
  return ChaterS(new Chater(messenger, NewReceiver()));
}


ChaterManagerS ChaterManager::New()
{
  return ChaterManagerS(new ChaterManager());
}

ChaterManager::~ChaterManager()
{
}
