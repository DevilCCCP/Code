#include <QTcpSocket>
#include <QSslSocket>
#include <QSslConfiguration>

#include <Lib/Ctrl/WorkerStat.h>
#include <Lib/Log/Log.h>

#include "ServerResponder.h"
#include "NetServer.h"
#include "Handler.h"


const int kDefaultTimeoutMs = 30*1000;

bool ServerResponder::DoInit()
{
  QTcpSocketS socket = QTcpSocketS((mSsl)? new QSslSocket(): new QTcpSocket());
  socket->setSocketDescriptor(mSocketDescriptor);
  if (mSsl) {
    QSslSocket* sslSocket = static_cast<QSslSocket*>(socket.data());
    sslSocket->setSslConfiguration(*mSsl);
    sslSocket->startServerEncryption();
  }

  SetTcpSocket(socket);
  SetUri(Uri::FromIpv4(socket->peerAddress(), socket->peerPort()));
  mHandler = mNetServer->CreateHandler();
  mHandler->SetConnectionUri(&GetUri());
  if (mHandler->getDebug()) {
    Log.Info(QString("Respond to server connection (uri: '%1')").arg(GetUri().ToString()));
  }

  return SyncSocket::DoInit();
}

bool ServerResponder::DoCircle()
{
  if (mHandler->DoCircle()) {
    SendAvailableData();
  }
  return SendReceiveAll() && TestTimeout();
}

void ServerResponder::DoRelease()
{
  if (mHandler->getDebug()) {
    Log.Info(QString("Close server connection from (uri: '%1', abort: %2)").arg(GetUri().ToString()).arg(IsConnected()));
  }
  mHandler->OnDisconnected();
  mNetServer->FreeConnection();
  CloseSocket();
}

void ServerResponder::Stop()
{
  CtrlWorker::Stop();
}

bool ServerResponder::SendReceiveAll()
{
  while (ReadyRead()) {
    ChangeWork(1);
    DispatchData();
    mTimeoutTimer.restart();
  }

  SendAvailableData();

  if (mConnectionEnded && !mDataAvailable) {
    if (mHandler->getDebug()) {
      Log.Info(QString("Send all done"));
    }
    return false;
  }
  if (SocketError()) {
    if (mHandler->getDebug()) {
      Log.Info(QString("Socket error ('%1')").arg(SocketErrorString()));
    }
    return false;
  }
  return true;
}

bool ServerResponder::TestTimeout()
{
  if (mTimeoutTimer.elapsed() > mTimeoutMs) {
    if (mHandler->getDebug()) {
      Log.Info(QString("Connection timeout"));
    }
    return false;
  }
  return true;
}

bool ServerResponder::DispatchData()
{
  if (!mHandler->Receive(this, mConnectionEnded)) {
    Stop();
    return false;
  }
  return true;
}

void ServerResponder::SendAvailableData()
{
  mDataAvailable = false;
  QByteArray data;
  while (mHandler->TakeData(data)) {
    mDataAvailable = true;
    if (data.isEmpty()) {
      break;
    }

    WriteData(data);
    if (IsStop()) {
      break;
    }
  }
}


ServerResponder::ServerResponder(NetServer* _NetServer, int _SocketDescriptor, QSslConfiguration* _Ssl)
  : mNetServer(_NetServer), mSocketDescriptor(_SocketDescriptor), mSsl(_Ssl)
  , mTimeoutMs(kDefaultTimeoutMs), mDataAvailable(false), mConnectionEnded(false)
{
  mTimeoutTimer.start();
}

ServerResponder::~ServerResponder()
{
}

