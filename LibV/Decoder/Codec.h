#pragma once

#include <Lib/Dispatcher/Conveyor.h>
#include <LibV/Include/Frame.h>
#include <LibV/Include/ConveyorV.h>


DefineClassS(Codec);
DefineClassS(FfmpegCodec);

class Codec: public ConveyorV
{
  ECompression   mVideoCompression;
  ECompression   mAudioCompression;
  bool           mReverse;

  FfmpegCodecS   mFfmpeg;
  FfmpegCodecS   mFfmpegAudio;
  int            mCodeFail;
  int            mCodeAFail;
  qint64         mReverseTimestamp;

public:
  bool UseAudio() { return (mAudioCompression & eAnyAudio) != 0; }

public:
  /*override */virtual const char* Name() override { return "Codec"; }
  /*override */virtual const char* ShortName() override { return "C"; }

protected:
//  /*override */virtual bool DoInit() override;
//  /*override */virtual void DoRelease() override;

protected:
  /*override */virtual bool ProcessFrame() override;

private:
  void CodeVideo();
  void CodeAudio();

public:
  void ResetCodec(bool _Reverse);

public:
  Codec(ECompression _VideoCompression, ECompression _AudioCompession = eCmprNone);
  /*override */virtual ~Codec();
};

