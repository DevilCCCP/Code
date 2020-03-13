#pragma once

#include <QList>

#include <Lib/Ctrl/CtrlWorker.h>
#include <Lib/Include/License_h.h>

#include "HandlerManager.h"


DefineClassS(NetServer);
DefineClassS(QTcpServer2);
DefineClassS(ServerResponder);
DefineClassS(HandlerManager);
DefineClassS(StateNotifier);
DefineClassS(WorkerStat);
DefineClassS(QSslConfiguration);

// Класс, слушающий порт и распределяющий задания (как бы сервер)
class NetServer: public CtrlWorker
{
  HandlerManagerS    mHandlerManager;
  const int          mPort;
  const int          mTimeout;
  const int          mConnectionsLimit;

  QSslConfiguration* mSsl;
  QTcpServer2S       mNetServer;
  int                mConnectionsCount;
  bool               mConnectionError;

  StateNotifierS     mNotifier;
  WorkerStatS        mHandlerWorkerStat;

  LICENSE_HEADER

public:
  void SetNotifier(const StateNotifierS& _Notifier) { mNotifier = _Notifier; }
  void SetSsl(QSslConfiguration* _Ssl) { mSsl = _Ssl; }
  void SetHandlerWorkerStat(const WorkerStatS& _WorkerStat) { mHandlerWorkerStat = _WorkerStat; }

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "NetServer"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "N"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

private:
  void ProcessNewConnections();
private: /*internal*/
  HandlerS CreateHandler();
  void FreeConnection();

private:
  void NotifyGood();
  void NotifyError();

public:
  explicit NetServer(int _Port, HandlerManagerS _HandlerManager, int _Timeout = 30000, int _ConnectionsLimit = 100);
  /*override */virtual ~NetServer();

  friend class ServerResponder;
};

