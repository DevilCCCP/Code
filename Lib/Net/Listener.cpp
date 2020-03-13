#include <QtNetwork/QTcpServer>
#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Ctrl/CtrlManager.h>

#include "Listener.h"
#include "QTcpServer2.h"
#include "Responder.h"


const int kWorkPeriod = 200;

bool Listener::DoInit()
{
  mNetServer = QTcpServer2S(new QTcpServer2);
  if (mNetServer->listen(QHostAddress::AnyIPv4, mPort)) {
    Log.Info(QString("Listen ok (port: %1)").arg(mPort));
    return true;
  } else {
    Log.Error(QString("Listen fail (port: %1)").arg(mPort));
    mListenFail = true;
    return false;
  }
}

bool Listener::DoCircle()
{
  bool timeout;
  if (mNetServer->waitForNewConnection(kWorkPeriod, &timeout)) {
    mConnectionError = false;
    ProcessNewConnections();
  } else if (IsStop()) {
    return false;
  } else if (!timeout) {
    if (!mConnectionError) {
      Log.Error(QString("Connection error %1 (code: %2)").arg(mNetServer->errorString()).arg(mNetServer->serverError()));
      mConnectionError = true;
    }
  } else {
    mConnectionError = false;
  }

  return true;
}

void Listener::DoRelease()
{
  Log.Info(QString("Listen done"));
  mNetServer->close();
  mNetServer.clear();
}

void Listener::ProcessNewConnections()
{
  qintptr socketDescriptor;
  while (mNetServer->TakeIncomingSocketDescription(socketDescriptor)) {
    if (mConnectionsCount < mConnectionsLimit) {
      mConnectionsCount++;
      //Log.Info(QString("New connection available (count: %1, descr: %2)").arg(mConnectionsCount).arg(socketDescriptor));
      MessengerS newResponder(new Responder(this, socketDescriptor));
      ChaterS chater = mChaterManager->CreateChater(newResponder);
      newResponder->HoldChater(chater);
      GetManager()->RegisterWorker(newResponder);
    } else {
      Log.Info("New connection skipped due to limit");
    }
  }
}

void Listener::FreeConnection()
{
  mConnectionsCount--;
}

Listener::Listener(int _Port, ChaterManagerS _ChaterManager, int _ConnectionsLimit)
  : CtrlWorker(0)
  , mChaterManager(_ChaterManager), mPort(_Port), mConnectionsLimit(_ConnectionsLimit)
  , mConnectionsCount(0), mConnectionError(false)
  , mListenFail(false)
{
}

Listener::~Listener()
{
}



