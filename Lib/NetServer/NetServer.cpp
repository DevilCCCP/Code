#include <QTcpServer>
#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Net/QTcpServer2.h>
#include <Lib/Include/StateNotifier.h>
#include <Lib/Include/License.h>

#include "NetServer.h"
#include "ServerResponder.h"


const int kWorkPeriod = 200;

bool NetServer::DoInit()
{
  Log.Info(QString("Starting net server (port: %1, ssl: %2)").arg(mPort).arg(mSsl? "yes": "no"));
  mNetServer = QTcpServer2S(new QTcpServer2);
  if (!mNetServer->listen(QHostAddress::AnyIPv4, mPort)) {
    Log.Fatal(QString("Listen port fail (port: %1)").arg(mPort), true);
    return false;
  }
  return true;
}

bool NetServer::DoCircle()
{
  LICENSE_CIRCLE(0x66ED8E48);
  bool timeout;
  if (mNetServer->waitForNewConnection(kWorkPeriod, &timeout)) {
    mConnectionError = false;
    ProcessNewConnections();
    NotifyGood();
  } else if (IsStop()) {
    return false;
  } else if (!timeout) {
    NotifyError();
    if (!mConnectionError) {
      Log.Error(QString("Connection error %1 (code: %2)").arg(mNetServer->errorString()).arg(mNetServer->serverError()));
      mConnectionError = true;
    }
  } else {
    mConnectionError = false;
    NotifyGood();
  }
  return IsAlive();
}

void NetServer::DoRelease()
{
  mNetServer->close();
  mNetServer.clear();
}

void NetServer::ProcessNewConnections()
{
  qintptr socketDescriptor;
  while (mNetServer->TakeIncomingSocketDescription(socketDescriptor)) {
    if (mConnectionsCount < mConnectionsLimit) {
      mConnectionsCount++;
      //Log.Info(QString("New server connection available (count: %1, descr: %2)").arg(mConnectionsCount).arg(socketDescriptor));
      ServerResponderS newResponder(new ServerResponder(this, socketDescriptor, mSsl));
      if (mHandlerWorkerStat) {
        newResponder->SetAggregateWorkerStat(mHandlerWorkerStat);
      }
      GetManager()->RegisterWorker(newResponder);
    } else {
      Log.Info("New server connection skipped due to limit");
    }
  }
}

HandlerS NetServer::CreateHandler()
{
  return mHandlerManager->CreateHandler();
}

void NetServer::FreeConnection()
{
  mConnectionsCount--;
}

void NetServer::NotifyGood()
{
  if (mNotifier) {
    if (mConnectionsCount < mConnectionsLimit) {
      mNotifier->NotifyGood();
    } else {
      mNotifier->NotifyWarning();
    }
  }
}

void NetServer::NotifyError()
{
  if (mNotifier) {
    mNotifier->NotifyError();
  }
}

NetServer::NetServer(int _Port, HandlerManagerS _HandlerManager, int _Timeout, int _ConnectionsLimit)
  : CtrlWorker(0)
  , mHandlerManager(_HandlerManager), mPort(_Port), mTimeout(_Timeout), mConnectionsLimit(_ConnectionsLimit)
  , mSsl(nullptr), mConnectionsCount(0), mConnectionError(false)
{
}

NetServer::~NetServer()
{
}



