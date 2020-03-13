#include <QMutexLocker>

#include <LibV/Include/VideoMsg.h>
#include <Lib/Log/Log.h>
#include <Lib/Net/NetMessage.h>
#include <LibV/Storage/Storage.h>
#include <LibV/Decoder/Thumbnail.h>
#include <LibV/Source/Ptz.h>

#include "TrReceiver.h"
#include "Transmit.h"


const int kFrameAtOnceLimit = 30;
const int kSendSpeedLogMs = 2 * 60 * 1000;
const int kSlowSendWarningMs = 5000;

void TrReceiver::DoCircle()
{
  QMutexLocker lock(&mStoreMutex);
  if (mStorageConnected) {
    lock.unlock();
    if (!TransmitNextStorageFrame()) {
      GetChater()->SendSimpleMessage(eMsgPlayNoStorage);
      mTransmit->ClientDisconnected(GetChater());
    }
  }
}

bool TrReceiver::ReceiveRequest(NetMessageS &msg, int &rspMsgId, QByteArray &rspMsgData)
{
  rspMsgId = 0;
  switch (msg->GetMessageType()) {
  case eMsgLiveRequest:
    DispatchMsgLiveRequest(msg);
    return false;

  case eMsgArchRequest:
    DispatchMsgArchRequest(msg);
    return false;

  case eMsgContinue:
    mTransmit->ClientContinue(GetChater());
    return false;

  case eMsgStop:
    Log.Info(QString("Client %1 stop").arg(GetChater()->Info()));
    mTransmit->ClientDisconnected(GetChater());
    return false;

  case eMsgThumbnail:
    DispatchThumbnailRequest(rspMsgId, rspMsgData);
    return true;

  case eMsgMediaInfo:
    DispatchMediaInfoRequest(rspMsgId, rspMsgData);
    return true;

  case eMsgPtzRequest:
    DispatchPtzRequest(msg, rspMsgId, rspMsgData);
    return true;

  default:
    Log.Warning(QString("Unknown request type received (type: %1)").arg(msg->GetMessageType()));
    return false;
  }
}

bool TrReceiver::ReceiveMessage(NetMessageS &msg)
{
  switch (msg->GetMessageType()) {
  case eMsgLiveRequest:
    DispatchMsgLiveRequest(msg);
    return false;

  case eMsgArchRequest:
    DispatchMsgArchRequest(msg);
    return false;

  case eMsgContinue:
    mTransmit->ClientContinue(GetChater());
    return false;

  case eMsgStop:
    Log.Info(QString("Client %1 stop").arg(GetChater()->Info()));
    mTransmit->ClientDisconnected(GetChater());
    return false;

  case eMsgThumbnail:
    DispatchThumbnailMessage();
    return false;

  case eMsgMediaInfo:
    DispatchMediaInfoMessage();
    return false;

  default:
    Log.Warning(QString("Unknown msg type received (type: %1)").arg(msg->GetMessageType()));
    return true;
  }
}

void TrReceiver::OnDisconnected()
{
  mTransmit->ClientDisconnected(GetChater());
}

void TrReceiver::DispatchMsgLiveRequest(NetMessageS &msg)
{
  if (msg->GetMessageDataSize() == sizeof (LiveRequest)) {
    LiveRequest* req = reinterpret_cast<LiveRequest*>(msg->GetMessageData());
    if (mTransmit->ClientPlayRequest(GetChater(), req->Priority, true)) {
      GetChater()->SendSimpleMessage(eMsgPlayGranted);
    } else {
      GetChater()->SendSimpleMessage(eMsgPlayDenied);
    }
    QMutexLocker lock(&mStoreMutex);
    mStorageConnected = false;
  } else {
    Log.Warning(QString("Get live request with wrong size (size: %1)").arg(msg->GetMessageDataSize()));
  }
}

void TrReceiver::DispatchMsgArchRequest(NetMessageS &msg)
{
  if (msg->GetMessageDataSize() == sizeof (ArchRequest)) {
    ArchRequest* req = reinterpret_cast<ArchRequest*>(msg->GetMessageData());
    if (!mTransmit->HaveStorage()) {
      GetChater()->SendSimpleMessage(eMsgPlayNoStorage);
    } else if (mTransmit->ClientPlayRequest(GetChater(), req->Priority, false, req->Timestamp, req->SpeedNum, req->SpeedDenum)) {
      if (!mStorage) {
        mStorage = mTransmit->CreateStorage();
      } else {
        mStorage->Reset();
      }
      if (InitStorage(req->Timestamp, req->SpeedNum, req->SpeedDenum)) {
        GetChater()->SendSimpleMessage(eMsgPlayGranted);
      } else {
        GetChater()->SendSimpleMessage(eMsgPlayNoStorage);
      }
    } else {
      GetChater()->SendSimpleMessage(eMsgPlayDenied);
    }
  } else {
    Log.Warning(QString("Get arch request with wrong size (size: %1)").arg(msg->GetMessageDataSize()));
  }
}

void TrReceiver::DispatchThumbnailMessage()
{
  int rspMsgId = 0;
  QByteArray rspMsgData;
  DispatchThumbnailRequest(rspMsgId, rspMsgData);

  char* data;
  GetChater()->PrepareMessageRaw(rspMsgId, rspMsgData.size(), data);
  memcpy(data, rspMsgData.constData(), rspMsgData.size());
  GetChater()->SendMessage();

  if (mTransmit->IsStatus()) {
    FrameS frame(new Frame());
    frame->ReserveData(sizeof(Frame::StatusHeader));
    Frame::StatusHeader* header = frame->GetStatusHeader();
    header->Size = sizeof(Frame::StatusHeader);
    header->HeaderSize = sizeof(Frame::StatusHeader);
    header->Status = Connection::eNoFrames;
    rspMsgData = QByteArray(frame->Data(), frame->Size());
    Log.Info(QString("Return status thumbnail"));

    GetChater()->PrepareMessageRaw(eMsgOneFrame, rspMsgData.size(), data);
    memcpy(data, rspMsgData.constData(), rspMsgData.size());
    GetChater()->SendMessage();
  }
}

void TrReceiver::DispatchThumbnailRequest(int &rspMsgId, QByteArray &rspMsgData)
{
  if (TakeThumbnail(rspMsgData)) {
    rspMsgId = eMsgThumbnailOk;
    Log.Info(QString("Return thumbnail (size: %1)").arg(rspMsgData.size()));
  } else if (FrameS frame = mTransmit->KeyFrame()) {
    rspMsgData = QByteArray(frame->Data(), frame->Size());
    rspMsgId = eMsgOneFrame;
    Log.Info(QString("Return key frame thumbnail (size: %1)").arg(rspMsgData.size()));
  } else {
    rspMsgId = eMsgThumbnailNo;
    Log.Info(QString("Return no thumbnail"));
  }
}

void TrReceiver::DispatchMediaInfoMessage()
{
  int rspMsgId;
  QByteArray rspMsgData;
  if (mTransmit->GetMediaInfo(rspMsgId, rspMsgData)) {
    char* data;
    GetChater()->PrepareMessageRaw(rspMsgId, rspMsgData.size(), data);
    memcpy(data, rspMsgData.constData(), rspMsgData.size());
    Log.Info(QString("Send media info (type: %1, size: %2)").arg(rspMsgId).arg(rspMsgData.size()));
    GetChater()->SendMessage();
  } else {
    Log.Info(QString("Respond no media info"));
    GetChater()->SendSimpleMessage(eMsgNoMediaInfo);
  }
}

void TrReceiver::DispatchMediaInfoRequest(int& rspMsgId, QByteArray& rspMsgData)
{
  if (mTransmit->GetMediaInfo(rspMsgId, rspMsgData)) {
    Log.Info(QString("Respond media info (type: %1, size: %2)").arg(rspMsgId).arg(rspMsgData.size()));
  } else {
    rspMsgId = eMsgNoMediaInfo;
    Log.Info(QString("Respond no media info"));
  }
}

void TrReceiver::DispatchPtzRequest(NetMessageS& msg, int& rspMsgId, QByteArray& rspMsgData)
{
  rspMsgId = eMsgPtzRespond;
  rspMsgData.resize(sizeof(PtzRespond));
  PtzRespond* resp = (PtzRespond*)rspMsgData.data();
  const PtzRequest* req = reinterpret_cast<const PtzRequest*>(msg->GetMessageConstData());
  if (msg->GetMessageDataSize() == sizeof(PtzRequest)) {
    if (DoPtz(*req, *resp)) {
      return;
    }
  }

  memset(resp, 0, sizeof(PtzRespond));
}

bool TrReceiver::InitStorage(const qint64& timestamp, int speedNum, int speedDenum)
{
  QMutexLocker lock(&mStoreMutex);
  if (speedNum > 0) {
    mMemoryStoreConnected = mTransmit->TakeStoreFrames(timestamp, mMemoryStoreFrames, true);
  } else {
    mMemoryStoreConnected = mTransmit->TakeStoreBackFrames(timestamp, mMemoryStoreFrames);
  }
  if (mMemoryStoreConnected) {
    mStorageFrame = (speedNum > 0)? mMemoryStoreFrames.takeFirst(): mMemoryStoreFrames.takeLast();
    mStorageConnected = true;
  } else {
    bool found;
    if (mStorage->SeekFrame(timestamp, found)) {
      if (found && mStorage->ReadFrame(mStorageFrame, speedNum >= 0)) {
        mStorageConnected = true;
      } else {
        mStorageConnected = false;
      }
    } else {
      Log.Error(QString("Storage seek fail (ts: %1, mem 1st: %2, mem size: %3)")
                .arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate))
                .arg(mTransmit->GetStoreFirstFrameTs().toString(Qt::ISODate))
                .arg(mTransmit->GetStoreSize()));
      mStorageConnected = false;
    }
  }

  if (mStorageConnected) {
    mStartTimestamp = mLastTimestamp = mStorageFrame->GetHeader()->Timestamp;
    mNextLogTimestamp = kSendSpeedLogMs;
    mStorageSpeedNum = speedNum;
    mStorageSpeedDenum = speedDenum;
    if (!mStorageSpeedNum) {
      mStorageSpeedNum = mStorageSpeedDenum = 1;
    }
    mTimerFromStart.start();

    qint64 fromTs = mStartTimestamp;
    bool placeMemory = mMemoryStoreConnected;
    lock.unlock();

    Log.Info(QString("Start transmit storage (ts: %1, from: %2, place: '%3')")
             .arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate))
             .arg(QDateTime::fromMSecsSinceEpoch(fromTs).toString(Qt::ISODate))
             .arg((placeMemory)? "memory": "disk"));
    return true;
  } else {
    lock.unlock();
    Log.Info(QString("Storage records not found (ts: %1)")
             .arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate)));
    return false;
  }
}

bool TrReceiver::TransmitNextStorageFrame()
{
  QMutexLocker lock(&mStoreMutex);
  if (!mStorageFrame) {
    if (!GetNextStorageFrame()) {
      lock.unlock();
      Log.Info(QString("Can't read first frame, disconnect from storage (client: '%1')").arg(GetChater()->Info()));
      return false;
    }
  }

  for (int frameAtOnce = 0; frameAtOnce < kFrameAtOnceLimit; frameAtOnce++) {
    if (!mStorageFrame) {
      break;
    }
    qint64 nowTimeRelative = mTimerFromStart.elapsed();
    qint64 nextTime = mStorageFrame->GetHeader()->Timestamp - mStartTimestamp;
    qint64 nextRelativeTime = nextTime * mStorageSpeedDenum/ mStorageSpeedNum;
    if (nowTimeRelative < nextRelativeTime) {
      break;
    }
    if (nextRelativeTime > mNextLogTimestamp) {
      Log.Info(QString("Sending info (speed ask: %1/%2, speed done: %3)")
               .arg(mStorageSpeedNum).arg(mStorageSpeedDenum).arg((qreal)nextTime / nowTimeRelative, 0, 'f', 1));
      mNextLogTimestamp += kSendSpeedLogMs;
    }
    if (!mSlowSendWarning && nowTimeRelative > nextRelativeTime + kSlowSendWarningMs) {
      Log.Warning(QString("Too slow sending (speed: %1/%2, time: %3, frame: %4)")
                  .arg(mStorageSpeedNum).arg(mStorageSpeedDenum).arg(nowTimeRelative).arg(nextRelativeTime));
      mSlowSendWarning = true;
    }
    FrameS sendFrame = mStorageFrame;
    mStorageFrame.clear();
    lock.unlock();

    char* data;
    GetChater()->PrepareMessageRaw(eMsgVideoFrame, sendFrame->Size(), data);
    memcpy(data, sendFrame->Data(), sendFrame->Size());

    bool sendResult = GetChater()->SendMessage();
    lock.relock();
    if (!sendResult) {
      mStorageConnected = false;
      lock.unlock();
      Log.Info(QString("Can't send frame, disconnect from storage (client: '%1')").arg(GetChater()->Info()));
      return false;
    }
    if (!GetNextStorageFrame()) {
      mStorageConnected = false;
      lock.unlock();
      Log.Error(QString("Storage get next fail for '%1' (ts: %2, mem 1st: %3, mem size: %4)").arg(GetChater()->Info())
                .arg(QDateTime::fromMSecsSinceEpoch(mLastTimestamp).toString(Qt::ISODate))
                .arg(mTransmit->GetStoreFirstFrameTs().toString(Qt::ISODate))
                .arg(mTransmit->GetStoreSize()));
      return false;
    }
  }
  return true;
}

bool TrReceiver::GetNextStorageFrame()
{
  if (mMemoryStoreConnected) {
    if (!mMemoryStoreFrames.isEmpty()) {
      mStorageFrame = (mStorageSpeedNum > 0)? mMemoryStoreFrames.takeFirst(): mMemoryStoreFrames.takeLast();
      mLastTimestamp = mStorageFrame->GetHeader()->Timestamp;
      if (mStorageSpeedNum > 0 && mMemoryStoreFrames.isEmpty()) {
        mMemoryStoreConnected = mTransmit->TakeStoreFrames(mLastTimestamp + 1, mMemoryStoreFrames, false);
      }
      return true;
    } else {
      mMemoryStoreConnected = false;
      bool found;
      mLastTimestamp += (mStorageSpeedNum >= 0)? 1: -1;
      if (!mStorage->SeekFrame(mLastTimestamp, found)) {
        if (!found) {
          Log.Info(QString("Storage switch mem -> no disk (ts: %1)")
                    .arg(QDateTime::fromMSecsSinceEpoch(mLastTimestamp).toString(Qt::ISODate)));
        }
        return false;
      } else {
        Log.Error(QString("Storage switch mem -> disk seek fail (ts: %1)")
                  .arg(QDateTime::fromMSecsSinceEpoch(mLastTimestamp).toString(Qt::ISODate)));
      }
    }
  }

  if (mStorage->ReadFrame(mStorageFrame, mStorageSpeedNum >= 0)) {
    mLastTimestamp = mStorageFrame->GetHeader()->Timestamp;
    return true;
  } else if (mStorageSpeedNum >= 0) {
    return mMemoryStoreConnected = mTransmit->TakeStoreFrames(mLastTimestamp + 1, mMemoryStoreFrames, false);
  }
  return false;
}

bool TrReceiver::TakeThumbnail(QByteArray& rspMsgData)
{
  if (ThumbnailS thumbnail = mTransmit->Thumbnail()) {
    QMutexLocker lock(&thumbnail->GetMutex());
    rspMsgData = QByteArray(thumbnail->GetThumbnail(&lock).constData(), thumbnail->GetThumbnail(&lock).size());
    return !rspMsgData.isEmpty();
  }
  return false;
}

bool TrReceiver::DoPtz(const PtzRequest& request, PtzRespond& respond)
{
  PtzS ptz = mTransmit->getPtz();
  if (!ptz) {
    return false;
  }
  QMutexLocker lock(mTransmit->PtzMutex());

  memset(&respond, 0, sizeof(respond));
  switch (request.Command) {
  case eGetAbilities:   return ReturnPtz(respond, eGetAbilities, ptz->GetAbilities(respond.AbilityFlag));
  case eGetPosition:    return ReturnPtz(respond, eGetPosition, ptz->GetPosition(respond.Position));
  case eGetRange:       return ReturnPtz(respond, eGetRange, ptz->GetRange(respond.MinPosition, respond.MaxPosition, respond.MinSpeed, respond.MaxSpeed));
  case eGetHome:        return ReturnPtz(respond, eGetHome, ptz->GetHome(respond.HomePosition));

  case eSetPosition:    return ReturnPtz(respond, eSetPosition, (request.Command & eUseSpeed)
                                         ? ptz->SetPosition(request.Position, request.Speed)
                                         : ptz->SetPosition(request.Position));
  case eRelativeMove:   return ReturnPtz(respond, eRelativeMove, (request.Command & eUseSpeed)
                                         ? ptz->RelativeMove(request.Position, request.Speed)
                                         : ptz->RelativeMove(request.Position));
  case eContinuousMove: return ReturnPtz(respond, eContinuousMove, ptz->ContinuousMove(request.Speed, request.Timeout));
  case eStopMove:       return ReturnPtz(respond, eStopMove, ptz->StopMove());
  case eMoveHome:       return ReturnPtz(respond, eMoveHome, (request.Command & eUseSpeed)
                                         ? ptz->MoveHome(request.Speed): ptz->MoveHome());
  case eSetHome:        return ReturnPtz(respond, eSetHome, ptz->SetHome(request.Position));
  default:              Log.Warning(QString("PTZ bad request (cmd: 0x%1)").arg(request.Command, 0, 16)); return false;
  }
  return false;
}

bool TrReceiver::ReturnPtz(PtzRespond& respond, int flag, bool result)
{
  if (result) {
    respond.AbilityFlag |= flag;
    return true;
  }
  respond.AbilityFlag = 0;
  return false;
}


TrReceiver::TrReceiver(Transmit *_Transmit)
  : mTransmit(_Transmit)
  , mStorageConnected(false), mSlowSendWarning(false)
  , mMemoryStoreConnected(false)
{
}

TrReceiver::~TrReceiver()
{
}
