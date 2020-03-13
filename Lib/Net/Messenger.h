#pragma once

#include <QElapsedTimer>
#include <QMutex>
#include <QWaitCondition>

#include <Lib/Include/Common.h>

#include "SyncSocket.h"
#include "NetMessage.h"


DefineClassS(NetMessage);
DefineClassS(Chater);

// Посылка/получение сообщений по соединению
/*internal */class Messenger: public SyncSocket
{
  ChaterS            mChater;
  QMutex             mChaterMutex;
  bool               mChaterClosed;

  QMutex             mMessagesMutex;
  NetMessageS        mRequestMsg;
  QList<NetMessageS> mMessagesOut;
  NetMessageS        mMessageIn;
  NetMessageS        mMessageOut;

  QElapsedTimer      mWorkTimer;

protected:
  /*override */virtual bool DoCircle() Q_DECL_OVERRIDE;
  /*override */virtual void DoRelease() Q_DECL_OVERRIDE;
public:
  /*override */virtual void Stop() Q_DECL_OVERRIDE;

public:
  void StopChat();

public:/*internal*/
  void HoldChater(ChaterS& _Chater) { mChater = _Chater; }

public:
  bool SendRequest(const NetMessageS& requestMsg);
  bool SendMessage(const NetMessageS& requestMsg, bool request = false);

private:
  bool DispatchNewMessage();
  bool DispatchNewPing();
  bool DispatchNewRequest();
  bool DispatchNewResponse();

private:
  bool ReceiveAll();
  bool SendAll();
  bool ExtraProcess();

private:
  bool ReadNewMessage();
  bool GetOutMessage();
  bool WriteMessage(const NetMessageS &message);

protected:
  Messenger();
public:
  /*override */virtual ~Messenger();
};

