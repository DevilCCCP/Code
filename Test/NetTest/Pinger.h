#pragma once

#include <QSharedPointer>

#include <Lib/Ctrl/CtrlWorker.h>
#include <Lib/Include/Uri.h>

DefineClassS(Chater);
DefineClassS(Sender);
DefineClassS(Pinger);

class Pinger : public CtrlWorker
{
  Uri mDestUri;
  ChaterS mChater;

public:
  /*new */virtual const char* Name() { return "Test pinger"; }
  /*new */virtual const char* ShortName() { return "P"; }
protected:
  /*new */virtual bool DoInit();
  /*new */virtual bool DoCircle();

public:
  Pinger(const Uri& _DestUri);
};

