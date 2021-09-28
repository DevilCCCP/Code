#pragma once

#include <QMutex>
#include <SDL.h>

#include "../Drawer.h"


class SdlPlayer: public DevicePlayer
{
  bool          mPlayOk;
  QList<FrameS> mAudioStack;
  int           mStackSize;
  int           mStackCount;
  QMutex        mStackMutex;
  FrameS        mPlayingFrame;
  int           mPlayingFramePos;

public:
  /*override */virtual void SetFrame(const FrameS& frame) override;

private:
  void FeedAudio(Uint8* stream, int len);
  void ResetFrames();

public:
  SdlPlayer(int _Frequency, int _Channels);
  /*override */virtual ~SdlPlayer();

  friend void SdlCallback(void* userdata, Uint8* stream, int len);
};

