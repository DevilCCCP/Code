#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "Media.h"
#include "MediaPlayer.h"
#include "Channel.h"
#include "TrFrame.h"


void Media::OnFrame(const TrFrameS& frame)
{
  QMutexLocker lock(&mMutex);
  for (auto itr = mChannels.begin(); itr != mChannels.end(); itr++) {
    ChannelS chan = *itr;
    lock.unlock();
    chan->InFrame(frame);
    lock.relock();
  }
}

void Media::OnChannelAdd(const ChannelS& channel)
{
  Q_UNUSED(channel);
}

void Media::OnChannelRemove(const ChannelS& channel)
{
  Q_UNUSED(channel);
}

bool Media::Start()
{
  QMutexLocker lock(&mMutex);
  if (mState == eNotStart) {
    mState = eStarted;
    return true;
  } else {
    Log.Warning(QString("Start media at wrong state (state: '%1')").arg(StateToString()));
    return false;
  }
}

bool Media::Test()
{
  QMutexLocker lock(&mMutex);
  if (mState == eStarted) {
    TestChannels();
    lock.unlock();
    if (const MediaPlayerS& mediaPlayer = GetMediaPlayer()) {
      return mediaPlayer->Test();
    }
  }
  return true;
}

void Media::Stop()
{
  QMutexLocker lock(&mMutex);
  if (mState == eStarted) {
    mState = eStopped;
    StopChannels();
    lock.unlock();
    if (const MediaPlayerS& mediaPlayer = GetMediaPlayer()) {
      mediaPlayer->Stop();
    }
  } else {
    Log.Warning(QString("Stop media at wrong state (state: '%1')").arg(StateToString()));
  }
}

bool Media::AddChannel(const ChannelS& channel)
{
  OnChannelAdd(channel);
  QMutexLocker lock(&mMutex);
  mChannels.append(channel);
  lock.unlock();
  Log.Info(QString("Add channel (media: '%1', ch: %2, total: %3)").arg(mId)
           .arg((quintptr)channel.data(), QT_POINTER_SIZE * 2, 16, QChar('0'))
           .arg(mChannels.size()));
  return true;
}

void Media::RemoveChannel(const ChannelS& channel)
{
  channel->StopChannel();
  OnChannelRemove(channel);
  QMutexLocker lock(&mMutex);
  for (auto itr = mChannels.begin(); itr != mChannels.end(); itr++) {
    if (itr->data() == channel.data()) {
      EraseChannel(itr);
      return;
    }
  }
  Log.Warning(QString("Remove channel fail (media: '%1', ch: %2, total: %3)").arg(mId)
              .arg((quintptr)channel.data(), QT_POINTER_SIZE * 2, 16, QChar('0'))
              .arg(mChannels.size()));
}

void Media::TestChannels()
{
  for (auto itr = mChannels.begin(); itr != mChannels.end(); ) {
    const ChannelS& channel = *itr;
    if (!channel->TestChannel()) {
      itr = EraseChannel(itr);
    } else {
      itr++;
    }
  }
}

void Media::StopChannels()
{
  QMutexLocker lock(&mMutex);
  for (auto itr = mChannels.begin(); itr != mChannels.end(); ) {
    itr = EraseChannel(itr);
  }
}

ListChannel::iterator Media::EraseChannel(const ListChannel::iterator& itr)
{
  ChannelS channel = *itr;
  auto ret = mChannels.erase(itr);
  OnChannelRemove(channel);
  channel->StopChannel();
  Log.Info(QString("Remove channel (media: '%1', ch: %2, total: %3)").arg(mId)
           .arg((quintptr)channel.data(), QT_POINTER_SIZE * 2, 16, QChar('0'))
           .arg(mChannels.size()));
  return ret;
}

const char* Media::StateToString()
{
  switch (mState) {
  case eNotStart: return "not started";
  case eStarted : return "started";
  case eStopped : return "stopped";
  case eIllegal : return "illegal";
  default       : return "error";
  }
}

Media::Media(const QString& _Id, const MediaPlayerS& _MediaPlayer)
  : Channel(nullptr, _MediaPlayer)
  , mId(_Id)
  , mState(eNotStart)
{
}

Media::~Media()
{
}

