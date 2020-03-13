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
  /*override */virtual const char* Name() Q_DECL_OVERRIDE { return "Responder"; }
  /*override */virtual const char* ShortName() Q_DECL_OVERRIDE { return "Rp"; }
protected:
  /*override */virtual bool DoInit() Q_DECL_OVERRIDE;
//  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
//  /*override */virtual void Stop() Q_DECL_OVERRIDE;

public:
  Responder(Listener* _Listener, int _SocketDescriptor);
  /*override */virtual ~Responder();
};

