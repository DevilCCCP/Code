#include <QTcpSocket>
#include <QMutexLocker>

#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Log/Log.h>

#include "Chater.h"
#include "ChaterManager.h"
#include "Requester.h"
#include "NetMessage.h"


const int kDefaultRequestTimeout = 30000;

QString Chater::Info()
{
  return mMessenger->GetUri().ToString();
}

ChaterS Chater::CreateChater(CtrlManager *ctrlManager, const Uri& _DestUri, ReceiverS receiver)
{
  MessengerS newRequester(new Requester(_DestUri));
  ChaterS newChater(new Chater(newRequester, receiver));
  newRequester->HoldChater(newChater);
  ctrlManager->RegisterWorker(newRequester);
  return newChater;
}

void Chater::UnregisterChater()
{
  if (mChaterManager) {
    mChaterManager->UnregisterChater(this);
    mChaterManager = nullptr;
  }
}

void Chater::Close()
{
  mMessenger->StopChat();
  QMutexLocker lock(&mRespondMutex);
  mRequestState = eClosed;
}

bool Chater::ReceiveRequest(NetMessageS& msg, int& rspMsgId, QByteArray& rspMsgData)
{
  return mReceiver->ReceiveRequest(msg, rspMsgId, rspMsgData);
}

bool Chater::ReceiveMessage(NetMessageS& msg)
{
  return mReceiver->ReceiveMessage(msg);
}

void Chater::OnDisconnected()
{
  OnDisconnected(false);
}

void Chater::OnClosed()
{
  OnDisconnected(true);
}

void Chater::OnDisconnected(bool exit)
{
  QMutexLocker lock(&mRespondMutex);
  if (mRequestState == eClosed) {
    return;
  }
  bool needWake = mRequestState == eWaitingRespond;
  mRequestState = eClosed;
  if (needWake) {
    mRequestRspMsg.clear();
    mRespondWait.wakeOne();
  }
  lock.unlock();
  if (!exit) {
    mReceiver->OnDisconnected();
  }
}

bool Chater::PrepareRequestRaw(int msgTypeRq, int requestDataSize, char*& requestData)
{
  if (!PrepareRequestInit()) {
    return false;
  }

  mRequestMsg = NetMessage::CreateForWrite(NetMessage::eRequest, msgTypeRq, ++mMsgId, requestDataSize);
  requestData = mRequestMsg->GetMessageData();
  return true;
}

bool Chater::SendRequest(int& msgTypeRsp, int& rspDataSize, const char *&rspData)
{
  if (SendReceive(true)) {
    msgTypeRsp = mRequestRspMsg->GetMessageType();
    rspDataSize = mRequestRspMsg->GetMessageDataSize();
    rspData = mRequestRspMsg->GetMessageConstData();
    return true;
  }
  return false;
}

bool Chater::SendPingRequest()
{
  if (!PrepareRequestInit()) {
    return false;
  }

  mRequestMsg = NetMessage::CreateForWrite(NetMessage::ePing, 0, ++mMsgId, 0);
  return SendReceive(true);
}

bool Chater::SendSimpleRequest(int msgTypeRq, NetMessageS& respondMsg, int requestTimeout)
{
  if (!PrepareRequestInit()) {
    return false;
  }

  if (requestTimeout > 0) {
    mRequestTimeout = requestTimeout;
  } else {
    mRequestTimeout = kDefaultRequestTimeout;
  }
  mRequestMsg = NetMessage::CreateForWrite(NetMessage::eRequest, msgTypeRq, ++mMsgId, 0);
  if (SendReceive(true)) {
    respondMsg = mRequestRspMsg;
    return true;
  }
  return false;
}

bool Chater::PrepareRequestInit()
{
  if (mRequestState != eReady) {
    if (mRequestState != eClosed) {
      Log.Warning(QString("Chater request in wrong state (state: %1)").arg(mRequestState));
    }
    return false;
  }

  mRequestState = eRequesting;
  return true;
}

bool Chater::SendReceive(bool request)
{
  QMutexLocker lock(&mRespondMutex);
  mRequestState = eWaitingRespond;
  if (!mMessenger->SendMessage(mRequestMsg, request)) {
    mRequestState = eReady;
    return false;
  }

  if (mRequestState == eWaitingRespond) {
    mRespondWait.wait(&mRespondMutex, mRequestTimeout);
  }
  if (mRequestState != eRespondReceived) {
    return false;
  }
  mRequestState = eReady;
  return true;
}

void Chater::SetRespond(NetMessageS& respondMsg)
{
  QMutexLocker lock(&mRespondMutex);
  if (mRequestState == eWaitingRespond) {
    mRequestState = eRespondReceived;
    mRequestRspMsg = respondMsg;
    mRespondWait.wakeOne();
  }
}

void Chater::SetDisconnect()
{
  QMutexLocker lock(&mRespondMutex);
  if (mRequestState == eWaitingRespond) {
    mRespondWait.wakeOne();
  }
}

bool Chater::PrepareRespondRaw(int msgTypeRp, int dataSize, char *&data)
{
  if (mRequestState != eReady) {
    if (mRequestState != eClosed) {
      Log.Warning(QString("Chater respond in wrong state (state: %1)").arg(mRequestState));
    }
    return false;
  }

  mRequestState = eResponding;

  mRespondMsg = NetMessage::CreateForWrite(NetMessage::eRespond, msgTypeRp, mRespondReqMsg->GetMessageId(), dataSize);
  data = mRespondMsg->GetMessageData();
  return true;
}

bool Chater::SendRespond()
{
  if (mRequestState != eResponding) {
    return false;
  }

  bool result = mMessenger->SendMessage(mRespondMsg);
  mRequestState = eReady;
  return result;
}

bool Chater::PrepareMessageRaw(int msgType, int dataSize, char *&data)
{
  if (!PrepareMessageInit()) {
    return false;
  }

  mRequestMsg = NetMessage::CreateForWrite(NetMessage::eMessage, msgType, ++mMsgId, dataSize);
  data = mRequestMsg->GetMessageData();
  return true;
}

bool Chater::SendSimpleMessage(int msgType)
{
  if (!PrepareMessageInit()) {
    return false;
  }

  mRequestMsg = NetMessage::CreateForWrite(NetMessage::eMessage, msgType, ++mMsgId, 0);
  return SendMessage();
}

bool Chater::SendMessage()
{
  if (mRequestState != eMessaging) {
    return false;
  }

  bool result = mMessenger->SendMessage(mRequestMsg);
  mRequestState = eReady;
  return result;
}

bool Chater::PrepareMessageInit()
{
  if (mRequestState != eReady) {
    if (mRequestState != eClosed) {
      Log.Warning(QString("Chater message in wrong state (state: %1)").arg(mRequestState));
    }
    return false;
  }

  mRequestState = eMessaging;
  return true;
}

Chater::Chater(MessengerS& _Messenger, ReceiverS _Receiver)
  : mMessenger(_Messenger), mReceiver(_Receiver), mChaterManager(nullptr)
  , mRequestState(eReady), mMsgId(0), mRequestTimeout(kDefaultRequestTimeout)
{
  mReceiver->SetChater(this);
}

Chater::~Chater()
{
  if (mRequestState != eClosed) {
    Close();
  }
}


