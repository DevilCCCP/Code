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
  /*override */virtual void incomingConnection(qintptr socketDescriptor) override;
  ///*override */virtual QTcpSocket* nextPendingConnection() override;

public:
  bool Listen(int port);
  bool TakeIncomingSocketDescription(qintptr& socketDescriptor);

public:
  QTcpServer2();
};

