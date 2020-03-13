#pragma once

#include <Lib/Common/Uri.h>

#include "Messenger.h"


DefineClassS(QTcpSocket2);

// Поток обработки для исходящих соединений
class Requester: public Messenger
{
  bool              mConnecting;
  QTcpSocket2S      mConnection2;

public:
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Requester"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "Rq"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
  /*override */virtual void Stop() Q_DECL_OVERRIDE;

public:
  Requester(const Uri& _Uri);
  /*override */virtual ~Requester();
};

