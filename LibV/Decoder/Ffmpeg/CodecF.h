#pragma once

#include <LibV/Include/Frame.h>

#include "../CodecA.h"


DefineClassS(CodecF);
DefineClassS(FfmpegDec);

class CodecF: public CodecA
{
  FfmpegDecS     mFfmpeg;
  int            mDecodeFail;

public:
  /*override */virtual bool IsHardware() override;

protected:
  /*override */virtual bool DecodeVideoFrame(const FrameS& frame, bool canSkip) override;
  /*override */virtual bool DecodeAudioFrame(const FrameS& frame, bool canSkip) override;
  /*override */virtual bool CanProfile() override;

public:
  CodecF(ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  /*override */virtual ~CodecF();
};

