#include <Lib/Log/Log.h>

#include "CodecP.h"
#include "VdpDec.h"


bool CodecP::IsHardware()
{
  return Init();
}

bool CodecP::DecodeVideoFrame(const FrameS& frame, bool canSkip)
{
  if (frame->GetHeader()->Compression != eH264) {
    Log.Error(QString("Not H264 compression with vdpau not implemented"));
    return false;
  }

  char* frameData = frame->VideoData();
  int frameSize = frame->VideoDataSize();

  if (!Init()) {
    return false;
  }

  bool ok = mVdp->DecodeIn(frameData, frameSize, frame->Timestamp());
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
      if (!mVdp->DecodeOut(nextSkip, destFrame)) {
        break;
      }
      mSkipList.removeFirst();
      OnDecodedFrame(destFrame);
    }
  }

  return ok;
}

bool CodecP::DecodeAudioFrame(const FrameS& frame, bool canSkip)
{
  Q_UNUSED(frame);
  Q_UNUSED(canSkip);
  return false;
}

bool CodecP::CanProfile()
{
    return false;
}

bool CodecP::Init()
{
  if (!mVdp) {
    Log.Info("Using vdpau decoder");
    mVdp = VdpDecS(new VdpDec(GetThumbnail()));
    mInitOk = mVdp->InitDecoder();
  }
  return mInitOk;
}


CodecP::CodecP(ECompression _DestCompression, int _Fps, bool _UseHardware)
  : CodecA(_DestCompression, _Fps, _UseHardware)
  , mInitOk(false), mInOk(false), mDecodeFail(0)
{
}

CodecP::~CodecP()
{
//  if (mVdp) {
//    mVdp->CloseDecoder();
//  }
}
