#pragma once

#include <QMutex>
#include <QList>

#include "Chater.h"


DefineClassS(Listener);
DefineClassS(ChaterManager);

class ChaterManager
{
  QMutex         mRegisterMutex;
  QList<ChaterS> mRegisterChaters;

private:/*internal*/
  ChaterS CreateChater(MessengerS& messenger);
  void UnregisterChater(Chater* chater);

protected:
  /*new */virtual ReceiverS NewReceiver();
  /*new */virtual ChaterS NewChater(MessengerS& messenger);

public:
  static ChaterManagerS New();

  /*new */virtual ~ChaterManager();

  friend class Chater;
  friend class Listener;
};

template<typename ParentT, typename ReceiverT>
class ChaterManagerR: public ChaterManager
{
  ParentT* mParent;

protected:
  /*override */virtual ReceiverS NewReceiver() Q_DECL_OVERRIDE { return ReceiverS(new ReceiverT(mParent)); }
//  /*override */virtual ChaterS NewChater(MessengerS& messenger) Q_DECL_OVERRIDE { }

public:
  static ChaterManagerS New(ParentT* _Parent) { return ChaterManagerS(new ChaterManagerR(_Parent)); }

  ChaterManagerR(ParentT* _Parent)
    : mParent(_Parent)
  { }
};

