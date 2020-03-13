#pragma once

#include <QtNetwork/QTcpSocket>

#include <Lib/Include/Common.h>


DefineClassS(QTcpSocket2);

class QTcpSocket2: public QTcpSocket
{
public:
  void Stop() { setSocketState(QTcpSocket::ClosingState); }
};

