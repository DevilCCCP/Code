#pragma once

#include <Lib/Ctrl/CtrlWorker.h>

#include "Messenger.h"


DefineClassS(Listener);

// Поток обработки для соединений, поступающих от Listner
class Responder: public Messenger
{
  Listener*  mListener;
  int        mSocketDescriptor;

public:
  /*override */virtual const char* Name() override { return "Responder"; }
  /*override */virtual const char* ShortName() override { return "Rp"; }
protected:
  /*override */virtual bool DoInit() override;
//  /*override */virtual bool DoCircle() override;
  /*override */virtual void DoRelease() override;
public:
//  /*override */virtual void Stop() override;

public:
  Responder(Listener* _Listener, int _SocketDescriptor);
  /*override */virtual ~Responder();
};

