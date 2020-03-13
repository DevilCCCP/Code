#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "RtspMedia.h"
#include "RtspChannel.h"


bool RtspMedia::OnTest()
{
  return true;
}

void RtspMedia::OnChannelAdd(const ChannelS& channel)
{
  RtspChannel* rtspChannel = static_cast<RtspChannel*>(channel.data());
  QMutexLocker lock(&mChannelMutex);
  mMapChannel[rtspChannel->Id()] = channel.staticCast<RtspChannel>();
}

void RtspMedia::OnChannelRemove(const ChannelS& channel)
{
  RtspChannel* rtspChannel = static_cast<RtspChannel*>(channel.data());
  QMutexLocker lock(&mChannelMutex);
  mMapChannel.remove(rtspChannel->Id());
}

RtspChannelS RtspMedia::FindChannel(const QByteArray& id)
{
  QMutexLocker lock(&mChannelMutex);
  auto itr = mMapChannel.find(id);
  if (itr != mMapChannel.end()) {
    return itr.value();
  } else {
    return RtspChannelS();
  }
}


RtspMedia::RtspMedia(const QString& _Id, const MediaPlayerManagerS& _MpManager, const SdpS& _Sdp)
  : Media(_Id)
  , mMpManager(_MpManager), mSdp(_Sdp)
{
}

RtspMedia::~RtspMedia()
{
}
