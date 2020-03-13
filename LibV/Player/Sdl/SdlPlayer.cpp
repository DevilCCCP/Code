#include <QMutexLocker>

#include <Lib/Log/Log.h>

#include "SdlPlayer.h"

const int kAudioBufferSize = 1024;
const int kSyncPacketsOnce = 8;
const int kSyncPacketsCount = 50;

void SdlCallback(void* userdata, Uint8* stream, int len)
{
  SdlPlayer* player = static_cast<SdlPlayer*>(userdata);
  player->FeedAudio(stream, len);
}

void SdlPlayer::SetFrame(FrameS &frame)
{
  QMutexLocker lock(&mStackMutex);
  if (mAudioStack.size() > kSyncPacketsOnce) {
    Log.Info("Drop audio buffer to quick sync with video");
    ResetFrames();
  } else if (mAudioStack.size() > 0) {
    mStackCount++;
    if (mStackCount > kSyncPacketsCount) {
      Log.Info("Drop audio buffer to statistics sync with video");
      ResetFrames();
    }
  } else {
    mStackCount = 0;
  }
  mStackSize += frame->AudioDataSize();
  mAudioStack.append(frame);
//  Log.Trace(QString("Buffer size: %1").arg(mStackSize));
}

void SdlPlayer::FeedAudio(Uint8 *stream, int len)
{
  while (len > 0) {
    if (mPlayingFrame) {
      int sz = qMin(mPlayingFrame->AudioDataSize() - mPlayingFramePos, len);
      memcpy(stream, mPlayingFrame->AudioData() + mPlayingFramePos, sz);
      stream += sz;
      mPlayingFramePos += sz;
      len -= sz;
      if (mPlayingFramePos >= mPlayingFrame->AudioDataSize()) {
        mPlayingFrame.clear();
      }
    } else {
      QMutexLocker lock(&mStackMutex);
      if (!mAudioStack.isEmpty()) {
        mPlayingFrame = mAudioStack.takeFirst();
        mStackSize -= mPlayingFrame->AudioDataSize();
        mPlayingFramePos = 0;
      } else {
        memset(stream, 0, len);
        return;
      }
    }
  }
}

void SdlPlayer::ResetFrames()
{
  mAudioStack.clear();
  mStackSize = 0;
  mStackCount = 0;
}


SdlPlayer::SdlPlayer(int _Frequency, int _Channels)
  : mStackSize(0), mStackCount(0)
{
  SDL_Init(SDL_INIT_AUDIO);

  SDL_AudioSpec as;
  as.freq = _Frequency;
  as.format = AUDIO_F32;
  as.channels = _Channels;
  as.silence = 0;
  as.samples = kAudioBufferSize;
  as.callback = SdlCallback;
  as.userdata = this;

  if (SDL_OpenAudio(&as, nullptr) == 0) {
    SDL_PauseAudio(0);
    mPlayOk = true;
  } else {
    Log.Error("SDL_OpenAudio fail");
    mPlayOk = false;
  }
}

SdlPlayer::~SdlPlayer()
{
  SDL_CloseAudio();
}
