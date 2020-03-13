#pragma once

#include <LibV/Include/Frame.h>

#include "../CodecA.h"


DefineClassS(CodecM);
DefineClassS(MfxDec);

class CodecM: public CodecA
{
  MfxDecS        mMfx;
  int            mDecodeFail;

public:
  /*override */virtual bool IsHardware() Q_DECL_OVERRIDE;

protected:
  /*override */virtual bool DecodeVideoFrame(const FrameS& frame, bool canSkip) Q_DECL_OVERRIDE;
  /*override */virtual bool DecodeAudioFrame(const FrameS& frame, bool canSkip) Q_DECL_OVERRIDE;
  /*override */virtual bool CanProfile() Q_DECL_OVERRIDE;

public:
  CodecM(ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  /*override */virtual ~CodecM();
};

