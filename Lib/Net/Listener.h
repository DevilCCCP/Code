#pragma once

#include <QList>

#include <Lib/Ctrl/CtrlWorker.h>

#include "Chater.h"
#include "ChaterManager.h"


DefineClassS(QTcpServer2);
DefineClassS(Responder);
DefineClassS(Chater);
DefineClassS(ChaterManager);
DefineClassS(Listener);

// Класс, слушащий порт и распределяющий задания (как бы сервер)
class Listener: public CtrlWorker
{
  ChaterManagerS    mChaterManager;
  const int         mPort;
  const int         mConnectionsLimit;

  QTcpServer2S      mNetServer;
  int               mConnectionsCount;
  bool              mConnectionError;

  bool              mListenFail;

public:
  bool ListenFail() { return mListenFail; }

public:
  /*override */virtual const char* Name() override { return "Listener"; }
  /*override */virtual const char* ShortName() override { return "L"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;
public:
//  /*override */virtual void Stop() override;

private:
  void ProcessNewConnections();
private: /*internal*/
  void FreeConnection();

public:
  explicit Listener(int _Port, ChaterManagerS _ChaterManager = ChaterManagerS(new ChaterManager()), int _ConnectionsLimit = 100);
  /*override */virtual ~Listener();

  friend class Responder;
};

