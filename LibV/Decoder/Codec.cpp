#include <Lib/Log/Log.h>

#include "Codec.h"
#include "Ffmpeg/FfmpegCodec.h"


bool Codec::ProcessFrame()
{
  //Log.Trace("Codec::ProcessFrame");
  FrameS currentFrame = CurrentVFrame();
  if ((currentFrame->GetHeader()->Compression & eTypeMask) == eCompessedVideo) {
    CodeVideo();
  }
  if (UseAudio() && (currentFrame->GetHeader()->CompressionAudio & eTypeMask) == eCompessedAudio) {
    CodeAudio();
  }
  return false;
}

void Codec::CodeVideo()
{
  //Log.Trace(QString("code frame (ts: %1, sz: %2, key: %3)").arg(CurrentVFrame()->GetHeader()->Timestamp).arg(CurrentVFrame()->Size()).arg(CurrentVFrame()->GetHeader()->Key));
  char* frameData  = CurrentVFrame()->VideoData();
  int frameSize    = CurrentVFrame()->VideoDataSize();
  int width        = CurrentVFrame()->GetHeader()->Width;
  int height       = CurrentVFrame()->GetHeader()->Height;
  qint64 timestamp = CurrentVFrame()->GetHeader()->Timestamp;

  if (!mFfmpeg) {
    ECompression compression = CurrentVFrame()->GetHeader()->Compression;

    AVCodecID codecId;
    switch (compression) {
    case eH264:  codecId = AV_CODEC_ID_H264; Log.Info("Found H.264 compression"); break;
    case eJpeg:  codecId = AV_CODEC_ID_MJPEG; Log.Info("Found JPEG compression"); break;
    case eMpeg4: codecId = AV_CODEC_ID_MPEG4; Log.Info("Found MPEG4 compression"); break;
    default: LOG_ERROR_ONCE("Unknown compression"); return;
    }
    AVCodecID codecId2;
    switch (mVideoCompression) {
    case eH264:  codecId2 = AV_CODEC_ID_H264; Log.Info("Encode to H.264 compression"); break;
    case eJpeg:  codecId2 = AV_CODEC_ID_MJPEG; Log.Info("Encode to JPEG compression"); break;
    case eMpeg4: codecId2 = AV_CODEC_ID_MPEG4; Log.Info("Encode to MPEG4 compression"); break;
    default: LOG_ERROR_ONCE("Encode to unknown compression"); return;
    }

    Log.Info("Using ffmpeg codec");
    mFfmpeg = FfmpegCodecS(new FfmpegCodec());
    mFfmpeg->InitCodec(codecId, codecId2);
  }

  if (mReverse) {
    if (mReverseTimestamp != -1 && timestamp < mReverseTimestamp) {
      FrameS encodedFrame;
      while (mFfmpeg->EncodeFromPack(encodedFrame)) {
        if (encodedFrame) {
          EmergeVFrame(encodedFrame);
        }
      }
    }
    mReverseTimestamp = timestamp;
    mFfmpeg->DecodeToPack(frameData, frameSize, timestamp, width, height);
    return;
  }

  if (mFfmpeg->Code(frameData, frameSize, width, height)) {
    if (mCodeFail > 0) {
      if (mCodeFail > 1) {
        Log.Info(QString("Code ok (fails: %1)").arg(mCodeFail));
      }
      mCodeFail = 0;
    }
    FrameS decodedFrame = mFfmpeg->CodedFrame();
    decodedFrame->GetHeader()->Timestamp = timestamp;
    EmergeVFrame(decodedFrame);
  } else {
    if (!mCodeFail) {
      Log.Warning("Code fail");
    }
    mCodeFail++;
  }
}

void Codec::CodeAudio()
{
  char* frameData = CurrentVFrame()->AudioData();
  int frameSize = CurrentVFrame()->AudioDataSize();

  if (!mFfmpegAudio) {
    ECompression compression = CurrentVFrame()->GetHeader()->CompressionAudio;

    AVCodecID codecId;
    switch (compression) {
    case eAac16b: codecId = AV_CODEC_ID_AAC; Log.Info("Found MPEG4 AAC audio compression"); break;
    default: LOG_ERROR_ONCE("Unknown audio compression"); return;
    }
    AVCodecID codecId2;
    switch (compression) {
    case eAac16b: codecId2 = AV_CODEC_ID_AAC; Log.Info("Encode to MPEG4 AAC audio compression"); break;
    default: LOG_ERROR_ONCE("Encode to unknown audio compression"); return;
    }

    Log.Info("Using ffmpeg audio codec");
    mFfmpegAudio = FfmpegCodecS(new FfmpegCodec());
    mFfmpegAudio->InitCodec(codecId, codecId2);
  }

  if (mReverse) {
    LOG_ERROR_ONCE("Reverse coding audio not implemented");
    return;
  }
  if (mFfmpegAudio->CodeAudio(frameData, frameSize)) {
    if (mCodeAFail > 0) {
      if (mCodeAFail > 1) {
        Log.Info(QString("Code audio ok (fails: %1)").arg(mCodeAFail));
      }
      mCodeAFail = 0;
    }
    FrameS decodedFrame = mFfmpegAudio->CodedFrame();
    decodedFrame->GetHeader()->Timestamp = CurrentVFrame()->GetHeader()->Timestamp;
    EmergeVFrame(decodedFrame);
  } else {
    if (!mCodeAFail) {
      Log.Warning("Code audio fail");
    }
    mCodeAFail++;
  }
}

void Codec::ResetCodec(bool _Reverse)
{
  mReverse = _Reverse;
  mReverseTimestamp = -1;
  ClearConveyor();
}

Codec::Codec(ECompression _VideoCompression, ECompression _AudioCompession)
  : mVideoCompression(_VideoCompression), mAudioCompression(_AudioCompession)
  , mCodeFail(0), mCodeAFail(0)
{
}

Codec::~Codec()
{
}
