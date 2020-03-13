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
  /*override */virtual bool IsHardware() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool DecodeVideoFrame(const FrameS& frame, bool canSkip) Q_DECL_OVERRIDE;
  /*override */virtual bool DecodeAudioFrame(const FrameS& frame, bool canSkip) Q_DECL_OVERRIDE;
  /*override */virtual bool CanProfile() Q_DECL_OVERRIDE;

public:
  CodecF(ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  /*override */virtual ~CodecF();
};

