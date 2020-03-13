#include <Lib/Log/Log.h>

#include "CodecO.h"
#include "OmxDec.h"


bool CodecO::IsHardware()
{
  return true;
}

bool CodecO::DecodeVideoFrame(const FrameS& frame, bool canSkip)
{
  char* frameData = frame->VideoData();
  int frameSize = frame->VideoDataSize();

  if (!mOmx) {
    Log.Info("Using omx decoder");
    mOmx = OmxDecS(new OmxDec(GetThumbnail()));
    mInitOk = mOmx->InitDecoder();
  }
  if (!mInitOk) {
    return false;
  }

  bool ok = mOmx->DecodeIn(frameData, frameSize);
  if (ok) {
    if (mDecodeFail > 0) {
      Log.Info(QString("Decode ok (fails: %1)").arg(mDecodeFail));
      mDecodeFail = 0;
    }
    mSkipList.append(canSkip);
    mInOk = true;
  } else {
    if (!mDecodeFail) {
      Log.Warning("Decode fail");
    }
    mDecodeFail++;
  }

  if (mInOk) {
    while (!mSkipList.isEmpty()) {
      bool nextSkip = mSkipList.first();
      FrameS destFrame;
      if (!mOmx->DecodeOut(nextSkip, destFrame)) {
        break;
      }
      mSkipList.removeFirst();
      OnDecodedFrame(destFrame);
    }
  }

  return ok;
}

bool CodecO::DecodeAudioFrame(const FrameS& frame, bool canSkip)
{
  Q_UNUSED(frame);
  Q_UNUSED(canSkip);
  return false;
}

bool CodecO::CanProfile()
{
  return false;
}

CodecO::CodecO(ECompression _DestCompression, int _Fps, bool _UseHardware)
  : CodecA(_DestCompression, _Fps, _UseHardware)
  , mInitOk(false), mInOk(false), mDecodeFail(0)
{
}

CodecO::~CodecO()
{
  if (mOmx) {
    mOmx->CloseDecoder();
  }
}
