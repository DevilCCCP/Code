#pragma once

#include <QTcpServer>
#include <QMutex>

#include <Lib/Include/Common.h>

DefineClassS(QTcpSocket);

class QTcpServer2 : public QTcpServer
{
  QMutex         mConnectionMutex;
  QList<qintptr> mSocketDescriptors;

private:
  /*override */virtual void incomingConnection(qintptr socketDescriptor) Q_DECL_OVERRIDE;
  ///*override */virtual QTcpSocket* nextPendingConnection() Q_DECL_OVERRIDE;

public:
  bool Listen(int port);
  bool TakeIncomingSocketDescription(qintptr& socketDescriptor);

public:
  QTcpServer2();
};

