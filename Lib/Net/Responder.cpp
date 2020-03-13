#include <QTcpSocket>

#include <Lib/Log/Log.h>

#include "Responder.h"
#include "Listener.h"

const int kWorkCircleMs = 500;

bool Responder::DoInit()
{
  QTcpSocketS connection(new QTcpSocket());
  connection->setSocketDescriptor(mSocketDescriptor);
  SetTcpSocket(connection);
  SetUri(Uri::FromIpv4(connection->peerAddress(), connection->peerPort()));
  Log.Info(QString("Respond to connection from '%1'").arg(GetUri().ToString()));
  return true;
}

void Responder::DoRelease()
{
  Messenger::DoRelease();
  mListener->FreeConnection();
}

Responder::Responder(Listener* _Listener, int _SocketDescriptor)
  : mListener(_Listener), mSocketDescriptor(_SocketDescriptor)
{
}

Responder::~Responder()
{
}


