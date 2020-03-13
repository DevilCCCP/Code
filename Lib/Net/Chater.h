#pragma once

#include <QMutex>
#include <QWaitCondition>

#include <Lib/Include/Common.h>
#include <Lib/Common/Uri.h>

#include "Receiver.h"

DefineClassS(NetMessage);
DefineClassS(Messenger);
DefineClassS(Responder);
DefineClassS(CtrlManager);
DefineClassS(Receiver);
DefineClassS(ChaterManager);
DefineClassS(Chater);

class Chater
{
  enum ERequestState {
    eReady            = 0x0,
    eRequesting       = 0x1,
    eResponding       = 0x2,
    eMessaging        = 0x3,

    eWaitingRespond   = 0x4,
    eRespondReceived  = 0x5,

    eClosed           = 0x6,

    eStateIllegal
  };

  MessengerS        mMessenger;
  ReceiverS         mReceiver;
  ChaterManager*    mChaterManager;

  ERequestState     mRequestState;
  int               mMsgId;
  int               mRequestTimeout;

  // Send/Request something
  NetMessageS       mRequestMsg;
  NetMessageS       mRequestRspMsg;

  // Respond something
  NetMessageS       mRespondReqMsg;
  NetMessageS       mRespondMsg;

  // In eWaitingRespond state
  QMutex            mRespondMutex;
  QWaitCondition    mRespondWait;

public:
  QString Info();
private:/*internal*/
  void SetManager(ChaterManager* _ChaterManager) { mChaterManager = _ChaterManager; }

public:
  static ChaterS CreateChater(CtrlManager* ctrlManager, const Uri& _DestUri, ReceiverS receiver = ReceiverS(new Receiver()));
  void UnregisterChater();

public:
  void Close();

private:/*internal*/
  bool ReceiveRequest(NetMessageS& msg, int &rspMsgId, QByteArray &rspMsgData);
  bool ReceiveMessage(NetMessageS& msg);
  void OnDisconnected();
  void OnClosed();
private:
  void OnDisconnected(bool exit);

public:
  // send request
  template<typename MsgTypeT>
  bool PrepareRequest(int msgTypeRq, MsgTypeT*& msgRq)
  { return PrepareRequestRaw(msgTypeRq, sizeof(MsgTypeT), reinterpret_cast<char*&>(msgRq)); }
  bool PrepareRequestRaw(int msgTypeRq, int dataSize, char*& data);
  bool SendRequest(int& msgTypeRsp, int& rspDataSize, const char*& rspData);
  bool SendPingRequest();
  bool SendSimpleRequest(int msgTypeRq, NetMessageS& respondMsg, int requestTimeout = 0);
protected:
  bool PrepareRequestInit();
  bool SendReceive(bool request);

private: /*internal*/
  void SetRespond(NetMessageS& respondMsg);
  void SetDisconnect();

public:
  // send respond
  template<typename MsgTypeT>
  bool PrepareRespond(int msgTypeRp, MsgTypeT*& msgRp)
  { return PrepareRespondRaw(msgTypeRp, sizeof(MsgTypeT), reinterpret_cast<char*&>(msgRp)); }
  bool PrepareRespondRaw(int msgTypeRp, int dataSize, char*& data);
  bool SendRespond();

public:
  // send message
  template<typename MsgTypeT>
  bool PrepareMessage(int msgType, MsgTypeT*& msg)
  { return PrepareMessageRaw(msgType, sizeof(MsgTypeT), reinterpret_cast<char*&>(msg)); }
  bool PrepareMessageRaw(int msgType, int dataSize, char*& data);
  bool SendSimpleMessage(int msgType);
  bool SendMessage();
protected:
  bool PrepareMessageInit();

private:/*internal*/
  Chater(MessengerS& _Messenger, ReceiverS _Receiver);
public:
  /*new */virtual ~Chater();

  friend class ChaterManager;
  friend class Responder;
  friend class Messenger;
};

