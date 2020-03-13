#include <Lib/Log/Log.h>

#include "CodecM.h"
#include "MfxDec.h"


bool CodecM::IsHardware()
{
  if (mMfx) {
    return mMfx->IsHardware();
  }
  mMfx.reset(new MfxDec(ThumbnailS()));
  bool hw = mMfx->QueryHardware();
  mMfx.clear();
  return hw;
}

bool CodecM::DecodeVideoFrame(const FrameS& frame, bool canSkip)
{
  char* frameData = frame->VideoData();
  int frameSize = frame->VideoDataSize();

  if (!mMfx) {
    ECompression compression = frame->GetHeader()->Compression;

    int codecId;
    switch (compression) {
    case eH264:  codecId = MFX_CODEC_AVC;  Log.Info("Found H.264 compression"); break;
    case eJpeg:  Log.Warning("Found JPEG compression, incompatible with MFX"); return false;
    case eMpeg4: Log.Warning("Found MPEG4 compression, incompatible with MFX"); return false;
    default: LOG_ERROR_ONCE("Unknown compression"); return false;
    }

    Log.Info("Using mfx decoder");
    mMfx.reset(new MfxDec(GetThumbnail()));
    mMfx->InitDecoder(codecId, frameData, frameSize);
  }

  if (mMfx->Decode(frameData, frameSize, canSkip)) {
    if (mDecodeFail > 0) {
      Log.Info(QString("Decode ok (fails: %1)").arg(mDecodeFail));
      mDecodeFail = 0;
    }
    FrameS nextFrame;
    while (mMfx->TakeDecodedFrame(nextFrame)) {
      OnDecodedFrame(nextFrame);
    }
    return true;
  } else {
    if (!mDecodeFail) {
      Log.Warning("Decode video fail");
    }
    mDecodeFail++;
    return false;
  }
}

bool CodecM::DecodeAudioFrame(const FrameS& frame, bool canSkip)
{
  Q_UNUSED(frame);
  Q_UNUSED(canSkip);
  return false;
}

bool CodecM::CanProfile()
{
  return false;
}


CodecM::CodecM(ECompression _DestCompression, int _Fps, bool _UseHardware)
  : CodecA(_DestCompression, _Fps, _UseHardware)
  , mDecodeFail(0)
{
}

CodecM::~CodecM()
{
}
