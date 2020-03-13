#include <QMutexLocker>

#include <Lib/Log/Log.h>
#include <Lib/Ctrl/CtrlManager.h>
#include <Lib/Dispatcher/Overseer.h>
#include <Lib/Dispatcher/ListenSvc.h>
#include <LibV/Include/VideoMsg.h>
#include <LibV/Storage/Storage.h>
#include <Lib/Include/License.h>
#include <LibV/Decoder/Thumbnail.h>
#include <LibV/MediaServer/H264/H264NalUnit.h>
#include <LibV/MediaServer/H264/H264Sprop.h>

#include "Transmit.h"
#include "TrReceiver.h"


const int kStoreTimeMs = 5000;

bool Transmit::DoInit()
{
  GetManager()->RegisterWorker<ListenSvc>(mListenSvc);
  return Conveyor::DoInit();
}

bool Transmit::ProcessFrame()
{
  LICENSE_CIRCLE(0x403A48);
  bool isDataFrame = CurrentVFrame()->GetHeader()->HeaderSize == sizeof(Frame::Header);
  if (isDataFrame) {
    if (CurrentVFrame()->GetHeader()->Key) {
      mFramePool.clear();
      if (!mMediaInfo) {
        InitMediaInfo();
      }
      mKeyFrame = CurrentVFrame();
    }
    if (mStoreSizeLimit) {
      UpdateStoreFrames();
    }
    mIsStatus = false;
  } else if (!mIsStatus && CurrentVFrame()->GetHeader()->HeaderSize == sizeof(Frame::StatusHeader)) {
    mIsStatus = true;
    mFramePool.clear();
    if (mKeyFrame) {
      mFramePool.append(mKeyFrame);
      mFramePool.append(CurrentVFrame());
    }
  }

  QMutexLocker lock(&mPoolMutex);
  for (auto itr = mClientPool.begin(); itr != mClientPool.end(); ) {
    ClentInfo& info = *itr;
    if (info.Live && !SendFrames(info)) {
      itr = ClientRemove(itr, "send fail", true);
    } else {
      itr++;
    }
  }
  lock.unlock();

  if (isDataFrame) {
    mFramePool.append(CurrentVFrame());
  }
  return true;
}

bool Transmit::GetMediaInfo(int& rspMsgId, QByteArray& rspMsgData)
{
  QMutexLocker lock(&mMediaMutex);
  if (!mSpsPpsInfo.isEmpty()) {
    rspMsgId = eMsgSpsPps;
    rspMsgData = mSpsPpsInfo;
    return true;
  }
  return false;
}

bool Transmit::HaveStorage()
{
  return mStorageSettings;
}

StorageS Transmit::CreateStorage()
{
  StorageS storage(new Storage(*mStorageSettings));
  if (!storage->Open(GetOverseer()->Id())) {
    Log.Error("Transmit: open storage fail");
    storage.clear();
  }
  return storage;
}

bool Transmit::ClientPlayRequest(Chater* chater, int priority, bool live, const qint64 timestamp, int speed, int denum)
{
  ClentInfo* existedClient = nullptr;
  int newWeight = CalcWeight(live, speed / denum);
  int canDropWeight = 0;
  QMutexLocker lock(&mPoolMutex);
  auto insertClient = mClientPool.begin();
  for (auto itr = mClientPool.begin(); itr != mClientPool.end(); itr++) {
    ClentInfo& info = *itr;
    if (info.Chat == chater) {
      existedClient = &info;
      canDropWeight += CalcWeight(info.Live, info.Speed / info.Denum);
    } else if (info.Priority < priority) {
      insertClient = itr;
      canDropWeight += CalcWeight(info.Live, info.Speed / info.Denum);
    }
  }

  int needDropWeight = mPoolWeight + newWeight - mPoolLimit;
  if (canDropWeight < needDropWeight) {
    int size = mClientPool.size();
    int weight = mPoolWeight;
    lock.unlock();
    Log.Info(QString("Client '%1' deny %2(%6) (priority: %3, pool: %4(%5))")
             .arg(chater->Info()).arg((live)? "live": "arch").arg(priority).arg(size).arg(weight).arg(speed / denum));
    return false;
  }

  if (existedClient) {
    int oldWeight = CalcWeight(existedClient->Live, existedClient->Speed / existedClient->Denum);
    *existedClient = ClentInfo(chater, live, priority, speed, denum);
    mPoolWeight += newWeight - oldWeight;
  } else {
    ClentInfo newInfo(chater, live, priority, speed, denum);
    mClientPool.insert(insertClient, newInfo);
    mPoolWeight += newWeight;
  }

  if (needDropWeight > 0) {
    for (auto itr = mClientPool.begin(); itr != mClientPool.end(); ) {
      ClentInfo& info = *itr;
      needDropWeight -= CalcWeight(info.Live, info.Speed / info.Denum);
      itr = ClientRemove(itr, "dropped", true);
      if (needDropWeight <= 0) {
        break;
      }
    }
  }
  int size = mClientPool.size();
  int weight = mPoolWeight;
  lock.unlock();
  QString action = (live)? "live": QString("arch '%1'").arg(QDateTime::fromMSecsSinceEpoch(timestamp).toString(Qt::ISODate));
  if (existedClient) {
    Log.Info(QString("Change client '%1' %2(%6) (priority: %3, pool: %4(%5))").arg(chater->Info()).arg(action).arg(priority).arg(size).arg(weight).arg(speed));
  } else {
    Log.Info(QString("New client '%1' %2(%6) (priority: %3, pool: %4(%5))").arg(chater->Info()).arg(action).arg(priority).arg(size).arg(weight).arg(speed));
  }
  return true;
}

void Transmit::ClientContinue(Chater* chater)
{
//  Log.Debug(QString("Clent '%1' continue").arg(chater->Info()));
  QMutexLocker lock(&mPoolMutex);
  for (auto itr = mClientPool.begin(); itr != mClientPool.end(); itr++) {
    ClentInfo& info = *itr;
    if (info.Chat == chater) {
      info.ConfirmTimer.restart();
    }
  }
}

void Transmit::ClientDisconnected(Chater* chater)
{
  QMutexLocker lock(&mPoolMutex);
  for (auto itr = mClientPool.begin(); itr != mClientPool.end(); itr++) {
    ClentInfo& info = *itr;
    if (info.Chat == chater) {
      ClientRemove(itr, "disconnected");
      return;
    }
  }
}

void Transmit::InitMediaInfo()
{
#ifdef MEDIA_INFO
  FrameS frame = CurrentVFrame();
  H264NalUnit nalu(frame->VideoData(), frame->VideoDataSize());
  H264Sprop sprop;
  QByteArray sps;
  QByteArray pps;
  while (nalu.FindNext()) {
    if (sprop.Test(nalu.CurrentUnit(), nalu.CurrentUnitSize())) {
      if (sprop.HasSps()) {
        sps = QByteArray(nalu.CurrentMarkedUnit(), nalu.CurrentMarkedUnitSize());
        sprop.ClearSps();
      } else if (sprop.HasPps()) {
        pps = QByteArray(nalu.CurrentMarkedUnit(), nalu.CurrentMarkedUnitSize());
        sprop.ClearPps();
      }
    }
  }

  if (!sps.isEmpty() && !pps.isEmpty()) {
    Log.Info(QString("Read sps-pps info"));
    QMutexLocker lock(&mMediaMutex);
    mSpsPpsInfo = sps + pps;
  } else {
    Log.Info(QString("Not found sps-pps info"));
  }
#endif
  mMediaInfo = true;
}

void Transmit::UpdateStoreFrames()
{
  qint64 tsNow = CurrentVFrame()->GetHeader()->Timestamp;

  QMutexLocker lock(&mStoreMutex);
  RemoveOldStoreFrames(tsNow);

  FrameS frame = CurrentVFrame();
  mStoreSize += frame->Size();
  mStorePool.append(frame);
}

void Transmit::RemoveOldStoreFrames(const qint64& timestamp)
{
  while (mStoreSize > mStoreSizeLimit
         && timestamp - mStorePool.first()->GetHeader()->Timestamp > kStoreTimeMs) {
    if (!mStoreFirstPartSize) {
      if (!CalcStoreFirstPartSize()) {
        return;
      }
    }
    if (mStoreSize - mStoreFirstPartSize <= mStoreSizeLimit) {
      return;
    }

    mStoreFirstPartSize = 0;
    FrameS frame = mStorePool.takeFirst();
    mStoreSize -= frame->Size();
    while (!mStorePool.isEmpty() && !mStorePool.first()->GetHeader()->Key) {
      frame = mStorePool.takeFirst();
      mStoreSize -= frame->Size();
    }
  }
}

bool Transmit::CalcStoreFirstPartSize()
{
  auto itr = mStorePool.begin();
  const FrameS& frame = *itr;
  mStoreFirstPartSize = frame->Size();
  for (itr++; itr != mStorePool.end(); itr++) {
    const FrameS& frame = *itr;
    if (frame->GetHeader()->Key) {
      return true;
    }
    mStoreFirstPartSize += frame->Size();
  }
  mStoreFirstPartSize = 0;
  return false;
}

bool Transmit::TakeStoreFrames(const qint64& timestamp, QList<FrameS>& frames, bool needKey)
{
  frames.clear();
  QMutexLocker lock(&mStoreMutex);
  if (mStorePool.isEmpty() || mStorePool.first()->GetHeader()->Timestamp > timestamp) {
    return false;
  }

  auto itrKey = mStorePool.begin();
  for (auto itr = mStorePool.begin(); itr != mStorePool.end(); itr++) {
    FrameS& stFr = *itr;
    if (stFr->GetHeader()->Timestamp >= timestamp) {
      if (needKey) {
        itr = itrKey;
      }
      for (; itr != mStorePool.end(); itr++) {
        frames.append(*itr);
      }
      return true;
    } else if (stFr->GetHeader()->Key) {
      itrKey = itr;
    }
  }
  return false;
}

bool Transmit::TakeStoreBackFrames(const qint64& timestamp, QList<FrameS>& frames)
{
  frames.clear();
  QMutexLocker lock(&mStoreMutex);
  if (mStorePool.isEmpty() || mStorePool.first()->GetHeader()->Timestamp > timestamp) {
    return false;
  }

  for (auto itr = mStorePool.begin(); itr != mStorePool.end(); itr++) {
    FrameS& stFr = *itr;
    if (stFr->GetHeader()->Key && stFr->GetHeader()->Timestamp <= timestamp) {
      frames.append(*itr);
    }
  }
  return !frames.isEmpty();
}

QDateTime Transmit::GetStoreFirstFrameTs()
{
  QMutexLocker lock(&mStoreMutex);
  return (!mStorePool.isEmpty())? QDateTime::fromMSecsSinceEpoch(mStorePool.first()->GetHeader()->Timestamp): QDateTime();
}

QList<ClentInfo>::iterator Transmit::ClientRemove(QList<ClentInfo>::iterator &where, const char *reason, bool stop)
{
  ClentInfo& info = *where;
  if (stop) {
    info.Chat->Close();
  }
  info.Chat->UnregisterChater();
  mPoolWeight -= CalcWeight(info.Live, info.Speed / info.Denum);
  Log.Info(QString("Client '%1' %2 (pool: %3(%4))").arg(info.Chat->Info()).arg(reason).arg(mClientPool.size()).arg(mPoolWeight));
  return mClientPool.erase(where);
}

bool Transmit::SendFrames(ClentInfo &info)
{
  if (info.ConfirmTimer.elapsed() > 2 * kConfirmFramesMs) {
    Log.Info(QString("Client '%1' timeout").arg(info.Chat->Info()));
    return false;
  }

  Chater* chater = info.Chat;
  if (info.PreFrames) {
    if (mKeyFrame) {
      SendInfo(chater);
    }
    if (!mFramePool.empty()) {
      for (auto itr_ = mFramePool.begin(); itr_ != mFramePool.end(); itr_++) {
        if (!SendFrame(chater, *itr_)) {
          return false;
        }
      }
    }
    info.PreFrames = false;
  }

  FrameS currentFrame = CurrentVFrame();
  return SendFrame(chater, currentFrame);
}

bool Transmit::SendInfo(Chater* chater)
{
  VideoInfo* info;
  if (chater->PrepareMessage(eMsgVideoInfo, info)) {
    info->StartTimestamp = mKeyFrame->GetHeader()->Timestamp;
    return chater->SendMessage();
  }
  return false;
}

bool Transmit::SendFrame(Chater* chater, FrameS &frame)
{
  char* data;
  if (chater->PrepareMessageRaw(eMsgVideoFrame, frame->Size(), data)) {
    memcpy(data, frame->Data(), frame->Size());
    return chater->SendMessage();
  }
  return false;
}

int Transmit::CalcWeight(bool live, int speed)
{
  return live? mLiveWeight: mArchWeight + qMax(speed / 8, 0);
}


Transmit::Transmit(SettingsAS& _StorageSettings, int _Port, int _PoolLimit, int _LiveWeight, int _ArchWeight)
  : mStorageSettings(_StorageSettings)
  , mPoolLimit(_PoolLimit), mLiveWeight(_LiveWeight), mArchWeight(_ArchWeight)
  , mPoolWeight(0)
  , mStoreSize(0), mStoreFirstPartSize(0), mStoreSizeLimit(0)
  , mMediaInfo(false)
{
  if (mStorageSettings) {
    mStoreSizeLimit = mStorageSettings->GetValue("CellSize").toInt();
    Log.Info(QString("Store size limit: %1").arg(mStoreSizeLimit));
  }
  mListenSvc = ListenSvcS(new ListenSvc(_Port, ChaterManagerR<Transmit, TrReceiver>::New(this)));
}

Transmit::~Transmit()
{
}


