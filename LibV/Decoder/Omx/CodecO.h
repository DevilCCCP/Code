#pragma once

#include <LibV/Include/Frame.h>

#include "../CodecA.h"


DefineClassS(CodecO);
DefineClassS(OmxDec);

class CodecO: public CodecA
{
  OmxDecS        mOmx;
  bool           mInitOk;
  bool           mInOk;
  int            mDecodeFail;
  QList<bool>    mSkipList;

public:
  /*override */virtual bool IsHardware() Q_DECL_OVERRIDE;

public:
  /*override */virtual bool DecodeVideoFrame(const FrameS& frame, bool canSkip) Q_DECL_OVERRIDE;
  /*override */virtual bool DecodeAudioFrame(const FrameS& frame, bool canSkip) Q_DECL_OVERRIDE;
  /*override */virtual bool CanProfile() Q_DECL_OVERRIDE;

public:
  CodecO(ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  /*override */virtual ~CodecO();
};

