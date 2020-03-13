#pragma once

#include "Handler.h"


DefineClassS(HandlerManager);
DefineClassS(NetServer);

class HandlerManager
{
private:/*internal*/
  HandlerS CreateHandler();

protected:
  /*new */virtual HandlerS NewHandler();

public:
  static HandlerManagerS New();

  /*new */virtual ~HandlerManager();

  friend class NetServer;
};

template<typename HandlerT>
class HandlerManagerB: public HandlerManager
{
protected:
  /*override */virtual HandlerS NewHandler() Q_DECL_OVERRIDE { return HandlerS(new HandlerT()); }

public:
  static HandlerManagerS New() { return HandlerManagerS(new HandlerManagerB()); }

  HandlerManagerB()
  { }
  /*override */virtual ~HandlerManagerB()
  { }
};

template<typename HandlerT, typename HandlerParentT>
class HandlerManagerC: public HandlerManager
{
  HandlerParentT* mParent;

protected:
  /*override */virtual HandlerS NewHandler() Q_DECL_OVERRIDE { return HandlerS(new HandlerT(mParent)); }

public:
  static HandlerManagerS New() { return HandlerManagerS(new HandlerManagerC()); }

  HandlerManagerC(HandlerParentT* _Parent)
    : mParent(_Parent)
  { }
  /*override */virtual ~HandlerManagerC()
  { }
};

