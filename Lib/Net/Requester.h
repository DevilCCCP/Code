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
  /*override */virtual const char* Name() override { return "Requester"; }
  /*override */virtual const char* ShortName() override { return "Rq"; }
protected:
  /*override */virtual bool DoInit() override;
  /*override */virtual void DoRelease() override;
public:
  /*override */virtual void Stop() override;

public:
  Requester(const Uri& _Uri);
  /*override */virtual ~Requester();
};

