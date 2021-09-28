#pragma once

#include <QElapsedTimer>

#include <Lib/Net/SyncSocket.h>
#include <Lib/Common/Uri.h>


DefineClassS(NetServer);
DefineClassS(Handler);
DefineClassS(ServerResponder);
DefineClassS(QSslConfiguration);

// Получение и обработка сообщений по соединению
/*internal */class ServerResponder: public SyncSocket
{
  NetServer*             mNetServer;
  int                    mSocketDescriptor;
  QSslConfiguration*     mSsl;
  HandlerS               mHandler;

  QElapsedTimer          mTimeoutTimer;
  int                    mTimeoutMs;
  bool                   mDataAvailable;
  bool                   mConnectionEnded;

protected:
  void SetTimeoutMs(int _TimeoutMs) { mTimeoutMs = _TimeoutMs; }

public:
  /*override */virtual const char* Name() override { return "ServerResponder"; }
  /*override */virtual const char* ShortName() override { return "R"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;
public:
  /*override */virtual void Stop() override;

private:
  bool SendReceiveAll();
  bool TestTimeout();

  bool DispatchData();
  void SendAvailableData();

public:
  ServerResponder(NetServer* _NetServer, int _SocketDescriptor, QSslConfiguration* _Ssl = nullptr);
  /*override */virtual ~ServerResponder();
};

