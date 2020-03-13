#pragma once

#include <Lib/Ctrl/CtrlWorker.h>

#include <Lib/Common/Uri.h>


DefineClassS(QTcpSocket);

/// Sync socket from Qt async
class SyncSocket: public CtrlWorker
{
  QTcpSocketS            mTcpSocket;

  int                    mReadCircleMs;
  Uri                    mUri;
  bool                   mError;

protected:
  void SetReadCircleMs(int _ReadCircleMs) { mReadCircleMs = _ReadCircleMs; }
public:
  void SetTcpSocket(QTcpSocketS _TcpSocket) { mTcpSocket = _TcpSocket; }
  void SetUri(const Uri& _Uri) { mUri = _Uri; }
  bool SocketError() { return mError; }
  QString SocketErrorString();
  const Uri& GetUri() { return mUri; }
  QString GetInfo() { return mUri.ToString(); }

public:
  QHostAddress PeerAddress();

  bool SslWait();
  bool ReadyRead(int size = 1);
  bool ReadData(QByteArray& buffer, int maxSize = 0x20000000);
  bool ReadData(char* buffer, int size);
  bool WriteData(const QByteArray &buffer, bool sync = true);
  bool WriteSync();
  bool DisconnectSync();

  bool IsConnected();
  void CloseSocket();

public:
  SyncSocket();
  ~SyncSocket();
};

