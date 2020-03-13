#pragma once

#include <QtNetwork/QSslSocket>

#include <Lib/Include/Common.h>


DefineClassS(QSslSocket2);

class QSslSocket2: public QSslSocket
{
public:
  void Stop() { setSocketState(QSslSocket::ClosingState); }
};

