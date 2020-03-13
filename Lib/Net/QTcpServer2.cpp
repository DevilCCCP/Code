#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "QTcpServer2.h"


void QTcpServer2::incomingConnection(qintptr socketDescriptor)
{
  //Log.Info(QString("Incoming connection (descriptor: %1)").arg(socketDescriptor));
  QMutexLocker lock(&mConnectionMutex);
  mSocketDescriptors.append(socketDescriptor);
}

bool QTcpServer2::Listen(int port)
{
  return listen(QHostAddress::Any, port);
}

bool QTcpServer2::TakeIncomingSocketDescription(qintptr& socketDescriptor)
{
  QMutexLocker lock(&mConnectionMutex);
  if (mSocketDescriptors.empty()) {
    return false;
  }
  socketDescriptor = mSocketDescriptors.takeFirst();
  return true;
}

QTcpServer2::QTcpServer2()
{
}
