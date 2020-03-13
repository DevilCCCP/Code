#include <QMutexLocker>
#include <QTcpSocket>

#include <Lib/Log/Log.h>

#include "Messenger.h"
#include "Chater.h"


const int kMaxWorkPeriodMs = 200;

bool Messenger::DoCircle()
{
  if (SendAll() && ExtraProcess() && ReceiveAll()) {
    return IsAlive();
  } else if (IsAlive()) {
    QMutexLocker lock(&mChaterMutex);
    if (mChater) {
      if (!mChaterClosed) {
        mChater->OnDisconnected();
      }
      mChater->UnregisterChater();
      mChater.clear();
    }
  }
  return false;
}

void Messenger::DoRelease()
{
  QMutexLocker lock(&mChaterMutex);
  if (mChater) {
    if (mChaterClosed) {
      mChater->OnClosed();
    } else {
      mChater->OnDisconnected();
    }
    mChater->UnregisterChater();
    mChater.clear();
    lock.unlock();
  }
  CloseSocket();
}

void Messenger::Stop()
{
  CtrlWorker::Stop();
}

void Messenger::StopChat()
{
  QMutexLocker lock(&mChaterMutex);
  mChaterClosed = true;
  lock.unlock();
  CtrlWorker::Stop();
}

bool Messenger::SendMessage(const NetMessageS &requestMsg, bool request)
{
  QMutexLocker lock(&mMessagesMutex);
  if (request) {
    mRequestMsg = requestMsg;
  }
  mMessagesOut.append(requestMsg);
  return true;
}

bool Messenger::DispatchNewMessage()
{
  switch (mMessageIn->GetRequestType()) {
  case NetMessage::ePing:             return DispatchNewPing();
  case NetMessage::eRequest:          return DispatchNewRequest();
  case NetMessage::eRespond:          return DispatchNewResponse();
  case NetMessage::eMessage:          mChater->ReceiveMessage(mMessageIn); return true;
  case NetMessage::eIllegal:
  default:
    Log.Warning(QString("Receive illegal message type (code: %1)").arg(mMessageIn->GetRequestType()));
    return false;
  }
}

bool Messenger::DispatchNewPing()
{
  NetMessageS respondMsg = NetMessage::CreateForWrite(NetMessage::eRespond, 0, mMessageIn->GetMessageId(), 0);
  return WriteMessage(respondMsg);
}

bool Messenger::DispatchNewRequest()
{
  int rspMsgId;
  QByteArray rspMsgData;
  if (mChater->ReceiveRequest(mMessageIn, rspMsgId, rspMsgData)) {
    NetMessageS respondMsg = NetMessage::CreateForWrite(NetMessage::eRespond, rspMsgId, mMessageIn->GetMessageId(), rspMsgData.size());
    if (rspMsgData.size() > 0) {
      memcpy(respondMsg->GetMessageData(), rspMsgData.constData(), rspMsgData.size());
    }
    return WriteMessage(respondMsg);
  }
  return true;
}

bool Messenger::DispatchNewResponse()
{
  QMutexLocker lock(&mMessagesMutex);
  if (mRequestMsg) {
    if (mMessageIn->GetMessageId() == mRequestMsg->GetMessageId()) {
      mChater->SetRespond(mMessageIn);
      return true;
    } else if (mMessageIn->GetMessageId() > mRequestMsg->GetMessageId()) {
      Log.Warning("Get respond for future message");
    }
  }
  return false;
}

bool Messenger::ReceiveAll()
{
//  int count = 0;
  mWorkTimer.start();
  while (ReadNewMessage()) {
    DispatchNewMessage();
    mMessageIn.clear();
//    count++;
    if (mWorkTimer.elapsed() > kMaxWorkPeriodMs) {
//      Log.Warning(QString("Receive too long, break (count: %1, time: %2)").arg(count).arg(mWorkTimer.elapsed()));
      break;
    }
  }

  if (IsStop()) {
    return false;
  } else if (SocketError()) {
    return false;
  }
  return true;
}

bool Messenger::SendAll()
{
//  int count = 0;
  mWorkTimer.start();
  while (GetOutMessage()) {
    if (!WriteMessage(mMessageOut)) {
      return false;
    }
    mMessageOut.clear();

//    count++;
    if (mWorkTimer.elapsed() > kMaxWorkPeriodMs) {
//      Log.Warning(QString("Send too long, break (count: %1, time: %2)").arg(count).arg(mWorkTimer.elapsed()));
      break;
    }
  }

//  mWorkTimer.start();
  bool sync = WriteSync();
//  if (mWorkTimer.elapsed() > kMaxWorkPeriodMs) {
//    Log.Warning(QString("Send sync too long (count: %1, time: %2)").arg(count).arg(mWorkTimer.elapsed()));
//  }
  return sync;
}

bool Messenger::ExtraProcess()
{
  mWorkTimer.start();
  mChater->mReceiver->DoCircle();
  if (mWorkTimer.elapsed() > kMaxWorkPeriodMs) {
    Log.Warning(QString("Extra process too long (time: %2)").arg(mWorkTimer.elapsed()));
  }
  return true;
}

bool Messenger::ReadNewMessage()
{
  if (!mMessageIn) {
    mMessageIn = NetMessage::CreateForRead();
  }

  char* buffer;
  int size;
  if (!mMessageIn->ValidateHeader()) {
    mMessageIn->GetReadHeaderBuffer(buffer, size);
    if (!ReadData(buffer, size) || !mMessageIn->ReadAndValidateHeader()) {
      return false;
    }
  }

  mMessageIn->GetReadDataBuffer(buffer, size);
  if (ReadData(buffer, size)) {
    //Log.Trace("Read message ok");
    return true;
  }
  return false;
}

bool Messenger::GetOutMessage()
{
  QMutexLocker lock(&mMessagesMutex);
  if (!mMessagesOut.empty()) {
    mMessageOut = mMessagesOut.takeFirst();
    return true;
  }
  return false;
}

bool Messenger::WriteMessage(const NetMessageS& message)
{
  if (WriteData(message->GetRawData(), false)) {
    //Log.Trace("Write message ok");
    return true;
  } else {
    Log.Trace("Write message fail");
    return false;
  }
}


Messenger::Messenger()
  : mChaterClosed(false)
{
}

Messenger::~Messenger()
{
}

