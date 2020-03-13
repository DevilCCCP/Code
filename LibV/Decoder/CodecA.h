#pragma once

#include <QMutex>

#include <LibV/Include/Frame.h>


DefineClassS(CodecA);
DefineClassS(Thumbnail);
DefineClassS(FpsDown);
DefineClassS(Profiler);

enum ECodec {
  eFfmpeg,
  eOmx
};

enum ECapability {
  eCapNone  = 0,
  eVideo    = 1 << 0,
  eAudio    = 1 << 1,
  eEncode   = 1 << 2,
  eDecode   = 1 << 3,
  eHardware = 1 << 4,
  eSoftware = 1 << 5
};

class CodecA
{
  ECompression   mDestCompression;
  bool           mUseHardware;

  ThumbnailS     mThumbnail;
  ProfilerS      mProfiler;
  FpsDownS       mFpsDown;

  qint64         mLastTimestamp;
  QList<FrameS>  mFramesIn;
  QMutex         mFramesMutex;
  QList<FrameS>  mFramesOut;

public:
  ECompression      GetDestCompression() const { return mDestCompression; }
  bool              GetUseHardware()     const { return mUseHardware; }
  const ThumbnailS& GetThumbnail()       const { return mThumbnail; }

  void SetThumbnail(const ThumbnailS& _Thumbnail) { mThumbnail = _Thumbnail; }

public:
  /*new */virtual bool IsHardware() = 0;

protected:
  /*new */virtual bool DecodeVideoFrame(const FrameS& frame, bool canSkip) = 0;
  /*new */virtual bool DecodeAudioFrame(const FrameS& frame, bool canSkip) = 0;
  /*new */virtual bool CanProfile() = 0;

protected:
  void OnDecodedFrame(const FrameS& frame);

public:
  void DecodeFrame(const FrameS& frame, bool audioVideo);
  bool GetDecodedFrame(FrameS& destFrame);

public:
  CodecA(ECompression _DestCompression = eRawNv12, int _Fps = 0, bool _UseHardware = true);
  virtual ~CodecA() { }
};

