#include <QtGlobal>

#include <Lib/Log/Log.h>

#include "Decoder.h"
#include "Thumbnail.h"
#include "Ffmpeg/CodecF.h"
#if defined(USE_MFX)
#include "Mfx/CodecM.h"
#elif defined(USE_VDPAU)
#include "Vdp/CodecP.h"
#elif defined(USE_OMX)
#include "Omx/CodecO.h"
#endif


const int kWorkPeriodMs = 200;

bool Decoder::DoCircle()
{
  RetriveDecodedFrames();

  return ConveyorV::DoCircle();
}

void Decoder::DoRelease()
{
  mCodecVideo.clear();
  mCodecAudio.clear();

  return ConveyorV::DoRelease();
}

bool Decoder::ProcessFrame()
{
  FrameS srcFrame = CurrentVFrame();
  if (srcFrame->GetHeader()->HeaderSize != sizeof(Frame::Header)) {
    EmergeVFrame(srcFrame);
  } else if ((srcFrame->GetHeader()->Compression & eTypeMask) == eCompessedVideo) {
    InitCodecVideo();
    mCodecVideo->DecodeFrame(srcFrame, true);
  } else if (mUseAudio && (srcFrame->GetHeader()->CompressionAudio & eTypeMask) == eCompessedAudio) {
    InitCodecAudio();
    mCodecAudio->DecodeFrame(srcFrame, false);
  }

  RetriveDecodedFrames();
  return false;
}

void Decoder::InitCodecVideo()
{
  if (!mCodecVideo) {
#if defined(USE_MFX)
    if (mUseHardware) {
      mCodecVideo.reset(new CodecM(mDestCompression, mFps, mUseHardware));
      if (!mCodecVideo->IsHardware()) {
        mCodecVideo.reset(new CodecF(mDestCompression, mFps, mUseHardware));
      }
#elif defined(USE_VDPAU)
    if (mUseHardware) {
      mCodecVideo.reset(new CodecP(mDestCompression, mFps, mUseHardware));
      if (!mCodecVideo->IsHardware()) {
        mCodecVideo.reset(new CodecF(mDestCompression, mFps, mUseHardware));
      }
#elif defined(USE_OMX)
    if (mUseHardware) {
      mCodecVideo.reset(new CodecO(mDestCompression, mFps, mUseHardware));
#else
    if (false) {
#endif
    } else {
      mCodecVideo.reset(new CodecF(mDestCompression, mFps, mUseHardware));
    }

    if (mThumbnail) {
      mCodecVideo->SetThumbnail(mThumbnail);
    }
  }
}

void Decoder::InitCodecAudio()
{
  if (!mCodecAudio) {
    mCodecAudio.reset(new CodecF(eRawAuF16, 0, mUseHardware));
  }
}

void Decoder::RetriveDecodedFrames()
{
  RetriveDecodedFramesForCodec(mCodecVideo);
  RetriveDecodedFramesForCodec(mCodecAudio);
}

void Decoder::RetriveDecodedFramesForCodec(const CodecAS& codec)
{
  FrameS destFrame;
  if (codec) {
    while (codec->GetDecodedFrame(destFrame)) {
      EmergeVFrame(destFrame);
    }
  }
}


Decoder::Decoder(bool _UseThumbnail, bool _UseAudio, ECompression _DestCompression, int _Fps, bool _UseHardware)
  : ConveyorV(kWorkPeriodMs)
  , mDestCompression(_DestCompression), mUseAudio(_UseAudio), mFps(_Fps), mUseHardware(_UseHardware)
{
  if (_UseThumbnail) {
    mThumbnail.reset(new Thumbnail());
  }
}

Decoder::~Decoder()
{
}
