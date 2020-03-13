#pragma once

#include <LibV/Include/Frame.h>


DefineClassS(OmxSound);
DefineClassS(IlComponents);
DefineClassS(IlTunnels);
DefineStructS(_ILCLIENT_T);
DefineStructS(_COMPONENT_T);

class OmxSound
{
  _ILCLIENT_TS  mIlClient;
  IlComponentsS mIlSound;
  QList<FrameS> mAudioStack;
  QMutex        mStackMutex;
  FrameS        mPlayingFrame;
  int           mPlayingFramePos;
  bool          mInit;

public:
  bool InitSound(int bufferSize);
  void CloseSound();
  bool PlayFrame(const char* audioData, int audioDataSize);

private:
  void ResetFrames();

public:
  OmxSound();
};
