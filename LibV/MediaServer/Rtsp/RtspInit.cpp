#include <QMutexLocker>

#include <Lib/Db/ObjectType.h>
#include <Lib/Net/Chater.h>
#include <Lib/Dispatcher/Overseer.h>
#include <LibV/MediaServer/Rtsp/RtspServer.h>
#include <LibV/MediaServer/VideoSysInfo.h>

#include "RtspInit.h"
#include "RtspCamReceiver.h"
#include "RtspMedia.h"
#include "Sdp.h"


const int kWorkPeriodMs = 1000;

bool RtspInit::DoCircle()
{
  if (!mInit) {
    mInit = mVideoSysInfo->LoadCameras(static_cast<Overseer*>(GetManager())->Id(), mQueryList) && InitCameras();
  }
  if (mInit) {
    return QueryCameras();
  }
  return true;
}

void RtspInit::DoRelease()
{
  QMutexLocker lock(&mMutex);
  QList<ObjectItemS> items = mQueryList.values();
  mQueryList.clear();
  lock.unlock();
  for (auto itr = items.begin(); itr != items.end(); itr++) {
    const ObjectItemS& item = *itr;
    RemoveCamera(item);
  }

  lock.relock();
  while (!mChaters.isEmpty()) {
    mWaitEnd.wait(&mMutex);
  }
}

void RtspInit::GetSpsPps(const ObjectItemS& item, const char* data, int size)
{
  SdpS sdp = Sdp::CreateFromH264Frame(data, size);
  if (sdp) {
    RtspMediaS rtspMedia(new RtspMedia(QString("/%1").arg(item->Id), mMpManager, sdp));
    if (mRtspServer->AddMedia(rtspMedia)) {
      RemoveCamera(item);
    }
  } else {
    GetFail(item);
  }
}

void RtspInit::GetFail(const ObjectItemS& item)
{
  if (IsAlive()) {
    QMutexLocker lock(&mMutex);
    if (IsAlive()) {
      mQueryList[item->Id] = item;
      return;
    }
  } else {
    RemoveCamera(item);
  }
}

void RtspInit::RemoveCamera(const ObjectItemS& item)
{
  QMutexLocker lock(&mMutex);
  auto itr = mChaters.find(item->Id);
  if (itr != mChaters.end()) {
    ChaterS chater = itr.value();
    mChaters.erase(itr);
    if (mChaters.isEmpty()) {
      mWaitEnd.wakeOne();
    }
    lock.unlock();

    chater->Close();
  }
}

bool RtspInit::InitCameras()
{
  QMutexLocker lock(&mMutex);
  for (auto itr = mQueryList.begin(); itr != mQueryList.end(); itr++) {
    InitOneCamera(itr.value());
  }
  return true;
}

void RtspInit::InitOneCamera(const ObjectItemS& item)
{
  RtspCamReceiverS reciver(new RtspCamReceiver(item, this));
  mChaters[item->Id] = Chater::CreateChater(GetManager(), Uri::FromString(item->Uri), reciver);
}

bool RtspInit::QueryCameras()
{
  QMutexLocker lock(&mMutex);
  for (auto itr = mQueryList.begin(); itr != mQueryList.end(); ) {
    ChaterS& chater = mChaters[itr.key()];
    if (chater->SendSimpleMessage(eMsgMediaInfo)) {
      itr = mQueryList.erase(itr);
    } else {
      InitOneCamera(itr.value());
      itr++;
    }
  }
  return !mChaters.isEmpty();
}


RtspInit::RtspInit(const VideoSysInfoS& _VideoSysInfo, const RtspServerS& _RtspServer, const MediaPlayerManagerS& _MpManager)
  : CtrlWorker(kWorkPeriodMs)
  , mVideoSysInfo(_VideoSysInfo), mRtspServer(_RtspServer), mMpManager(_MpManager)
  , mInit(false)
{
}

RtspInit::~RtspInit()
{
}

