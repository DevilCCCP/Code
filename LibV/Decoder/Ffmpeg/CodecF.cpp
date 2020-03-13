#include <Lib/Log/Log.h>

#include "CodecF.h"
#include "FfmpegDec.h"


bool CodecF::IsHardware()
{
  return false;
}

bool CodecF::DecodeVideoFrame(const FrameS& frame, bool canSkip)
{
  char* frameData = frame->VideoData();
  int frameSize = frame->VideoDataSize();

  if (!mFfmpeg) {
    ECompression compression = frame->GetHeader()->Compression;

    AVCodecID codecId;
    switch (compression) {
    case eH264:  codecId = AV_CODEC_ID_H264;  Log.Info("Found H.264 compression"); break;
    case eJpeg:  codecId = AV_CODEC_ID_MJPEG; Log.Info("Found JPEG compression"); break;
    case eMpeg4: codecId = AV_CODEC_ID_MPEG4; Log.Info("Found MPEG4 compression"); break;
    default: LOG_ERROR_ONCE("Unknown compression"); return false;
    }

    Log.Info("Using ffmpeg decoder");
    mFfmpeg = FfmpegDecS(new FfmpegDec(GetThumbnail()));
    mFfmpeg->InitDecoder(codecId, nullptr);
    mFfmpeg->SetDestCompression(GetDestCompression());
  }

  if (mFfmpeg->Decode(frameData, frameSize, frame->GetHeader()->Width, frame->GetHeader()->Height, canSkip)) {
    if (mDecodeFail > 0) {
      Log.Info(QString("Decode ok (fails: %1)").arg(mDecodeFail));
      mDecodeFail = 0;
    }
    OnDecodedFrame(mFfmpeg->DecodedFrame());
    return true;
  } else {
    if (!mDecodeFail) {
      Log.Warning("Decode video fail");
    }
    mDecodeFail++;
    return false;
  }
}

bool CodecF::DecodeAudioFrame(const FrameS& frame, bool canSkip)
{
  Q_UNUSED(canSkip);

  char* frameData = frame->AudioData();
  int frameSize = frame->AudioDataSize();

  if (!mFfmpeg) {
    ECompression compression = frame->GetHeader()->CompressionAudio;

    AVCodecID codecId;
    switch (compression) {
    case eAac16b: codecId = AV_CODEC_ID_AAC; Log.Info("Found MPEG4 AAC audio compression"); break;
    default: LOG_ERROR_ONCE("Unknown audio compression"); return false;
    }

    Log.Info("Using ffmpeg audio decoder");
    mFfmpeg = FfmpegDecS(new FfmpegDec(ThumbnailS()));
    mFfmpeg->InitDecoder(codecId, frame->AudioData());
  }

  if (mFfmpeg->DecodeAudio(frameData, frameSize)) {
    if (mDecodeFail > 0) {
      if (mDecodeFail > 1) {
        Log.Info(QString("Decode audio ok (fails: %1)").arg(mDecodeFail));
      }
      mDecodeFail = 0;
    }
    OnDecodedFrame(mFfmpeg->DecodedFrame());
    return true;
  } else {
    if (!mDecodeFail) {
      Log.Warning("Decode audio fail");
    }
    mDecodeFail++;
    return false;
  }
}

bool CodecF::CanProfile()
{
  return true;
}


CodecF::CodecF(ECompression _DestCompression, int _Fps, bool _UseHardware)
  : CodecA(_DestCompression, _Fps, _UseHardware)
  , mDecodeFail(0)
{
}

CodecF::~CodecF()
{
}
