#ifndef QT_NO_DEBUG
#include <QDir>
#include <QImage>
#endif

#include <Lib/Log/Log.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <libavutil/mem.h>

#ifdef __cplusplus
}
#endif

#include "FfmpegCodec.h"


void FfmpegRegister()
{
  static volatile bool gInit = false;
  if (!gInit) {
    gInit = true;
    avcodec_register_all();
    Log.Info("avcodec_register_all");
  }
}

inline void av_frame_dtor(AVFrame* fr)
{
  if (fr) {
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
    av_frame_free(&fr);
#else
    avcodec_free_frame(&fr);
#endif
  }
}

inline void av_context_dtor(AVCodecContext* context)
{
  if (context) {
    avcodec_close(context);
    av_free(context);
  }
}

bool FfmpegCodec::InitCodec(AVCodecID _SrcCodecId, AVCodecID _DestCodecId)
{
  mPacketDec = AVPacketS(new AVPacket());
  av_init_packet(mPacketDec.data());
  mPacketEnc = AVPacketS(new AVPacket());
  av_init_packet(mPacketEnc.data());

  if (!InitFrame()) {
    return false;
  }

  av_log_set_level(AV_LOG_ERROR);

  Log.Trace("Ffmpeg::InitCodec");
  return InitDecoder(_SrcCodecId) && InitEncoder(_DestCodecId);
}

bool FfmpegCodec::Code(char *frameData, int frameSize, int width, int height)
{
  mCodedFrame.clear();

  if (!mCodecDec) {
    LOG_ERROR_ONCE("ffmpeg: decode fail, init codec first");
    return false;
  } else if (!mCodecEnc) {
    LOG_ERROR_ONCE("ffmpeg: encode fail, init codec first");
    return false;
  }

  mPacketDec->data = (byte*)frameData;
  mPacketDec->size = frameSize;

  mFrame->width = width;
  mFrame->height = height;

  for (int size = 0; mPacketDec->size > 0; mPacketDec->data += size, mPacketDec->size -= size) {
    int frame;
    size = avcodec_decode_video2(mContextDec.data(), mFrame.data(), &frame, mPacketDec.data());
    if (size < 0) {
      char errText[800 + 1];
      av_strerror(size, errText, 800);
      Log.Warning(QString("ffmpeg: Error while decoding frame (code: %1, text: %2)").arg(size, 0, 16).arg(errText));
      return false;
    } else if (size == 0) {
      Log.Warning("ffmpeg: decoding 0 bytes");
      return false;
    } else if (frame) {
      return EncodeFrame(mFrame);
    }
    //av_frame_unref(mFrame);
  }
  return true;
}

bool FfmpegCodec::DecodeToPack(char* frameData, int frameSize, qint64 timestamp, int width, int height)
{
  mPackSynced = true;
  mCodedFrame.clear();

  if (!mCodecDec) {
    LOG_ERROR_ONCE("ffmpeg: decode fail, init codec first");
    return false;
  } else if (!mCodecEnc) {
    LOG_ERROR_ONCE("ffmpeg: encode fail, init codec first");
    return false;
  }

  mPacketDec->data = (byte*)frameData;
  mPacketDec->size = frameSize;

  mFrame->width = width;
  mFrame->height = height;

  mDecodedTimestamps.append(timestamp);

  for (int size = 0; mPacketDec->size > 0; mPacketDec->data += size, mPacketDec->size -= size) {
    int frame;
    mContextDec->refcounted_frames = 1;
    size = avcodec_decode_video2(mContextDec.data(), mFrame.data(), &frame, mPacketDec.data());
    if (size < 0) {
      char errText[800 + 1];
      av_strerror(size, errText, 800);
      Log.Warning(QString("ffmpeg: Error while decoding frame (code: %1, text: %2)").arg(size, 0, 16).arg(errText));
      return false;
    } else if (size == 0) {
      Log.Warning("ffmpeg: decoding 0 bytes");
      return false;
    } else if (frame) {
      //Log.Trace(QString("decoded frames: %1").arg(frame));
      mDecodedFrames.append(mFrame);
      return InitFrame();
    } else {
      //Log.Trace(QString("ffmpeg: No frame decoded"));
    }
  }
  return true;
}

bool FfmpegCodec::EncodeFromPack(FrameS& frame)
{
  frame.clear();
  if (!mDecodedFrames.isEmpty()) {
    mPackSynced = false;
    AVFrameS fr = mDecodedFrames.takeLast();
    if (EncodeFrame(fr)) {
      mCodedFrame->GetHeader()->Timestamp = mDecodedTimestamps.takeLast();
      frame = mCodedFrame;
    }
    return true;
  } else if (!mPackSynced) {
    //Log.Trace("encode sync");
    AVFrameS avfr;
    if (EncodeFrame(avfr)) {
      mCodedFrame->GetHeader()->Timestamp = mDecodedTimestamps.takeLast();
      frame = mCodedFrame;
    } else {
      mDecodedTimestamps.clear();
      mContextEnc.clear();
      mPackSynced = true;
    }
    return true;
  }

  return false;
}

bool FfmpegCodec::CodeAudio(char *frameData, int frameSize)
{
  Q_UNUSED(frameData);
  Q_UNUSED(frameSize);
  return false;
}

bool FfmpegCodec::InitFrame()
{
#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
  mFrame = AVFrameS(av_frame_alloc(), av_frame_dtor);
#else
  mFrame = AVFrameS(avcodec_alloc_frame(), av_frame_dtor);
#endif

  if (!mFrame) {
    Log.Error(QString("ffmpeg: Could not allocate video frame"));
    return false;
  }

  return true;
}

bool FfmpegCodec::InitDecoder(AVCodecID _CodecId)
{
  mCodecDec = avcodec_find_decoder(_CodecId);
  if (!mCodecDec) {
    Log.Error(QString("ffmpeg: decoder not found (codec: %1)").arg(_CodecId));
    return false;
  }

  mContextDec = AVCodecContextS(avcodec_alloc_context3(mCodecDec), av_context_dtor);
  if (!mContextDec) {
    Log.Error(QString("ffmpeg: Could not allocate video codec context"));
    return false;
  }

  if (avcodec_open2(mContextDec.data(), mCodecDec, nullptr) < 0) {
    Log.Error(QString("ffmpeg: Could not open codec"));
    return false;
  }
  return true;
}

bool FfmpegCodec::InitEncoder(AVCodecID _CodecId)
{
  mCodecEnc = avcodec_find_encoder(_CodecId);
  if (!mCodecEnc) {
    Log.Error(QString("ffmpeg: encoder not found (codec: %1)").arg(_CodecId));
    return false;
  }
  return true;
}

bool FfmpegCodec::InitEncodeContext(AVFrameS& avframe)
{
  mContextEnc = AVCodecContextS(avcodec_alloc_context3(mCodecEnc), av_context_dtor);
  if (!mContextEnc) {
    Log.Error(QString("ffmpeg: Could not allocate video codec context"));
    return false;
  }

  mContextEnc->pix_fmt = (AVPixelFormat)avframe->format;
  mContextEnc->width = avframe->width;
  mContextEnc->height = avframe->height;

  if (int err = avcodec_open2(mContextEnc.data(), mCodecEnc, nullptr) < 0) {
    char errText[800 + 1];
    av_strerror(err, errText, 800);
    Log.Error(QString("ffmpeg: Could not open codec (code: %1, error: '%2')").arg(err).arg(errText));
    mContextEnc.clear();
    return false;
  }
  return true;
}

bool FfmpegCodec::EncodeFrame(AVFrameS& avframe)
{
  mCodedFrame.clear();

  if (!mContextEnc) {
    if (!avframe) {
      return false;
    }
    InitEncodeContext(avframe);
  }

  mPacketEnc->data = nullptr;
  mPacketEnc->size = 0;

  int frame;
  //Log.Trace(QString("Begin encode data: %1").arg(reinterpret_cast<int>(avframe.data())));
  int err = avcodec_encode_video2(mContextEnc.data(), mPacketEnc.data(), avframe.data(), &frame);
  if (err < 0) {
    char errText[800 + 1];
    av_strerror(err, errText, 800);
    Log.Warning(QString("ffmpeg: Error while encoding frame (code: %1, text: %2)").arg(err, 0, 16).arg(errText));
    return false;
  } else if (frame) {
    //Log.Trace(QString("encoded frames: %1").arg(frame));
    return CreateFrame();
  } else {
    //Log.Trace(QString("ffmpeg: No frame encoded"));
    return false;
  }
}

bool FfmpegCodec::CreateFrame()
{
  //Log.Trace("ffmpeg: frame decoded");
  mCodedFrame = FrameS(new Frame());

  mCodedFrame->ReserveData(mPacketEnc->size);
  Frame::Header* header = mCodedFrame->InitHeader();
  switch (mCodecEnc->id) {
  case AV_CODEC_ID_H264:  header->Compression = eH264; break;
  case AV_CODEC_ID_MJPEG: header->Compression = eJpeg; break;
  case AV_CODEC_ID_MPEG4: header->Compression = eMpeg4; break;
  default:
    LOG_ERROR_ONCE(QString("ffmpeg: CreateFrame unimplemented codec Id (id: %1)").arg(mCodecEnc->id));
    return false;
  }
  header->VideoDataSize = mPacketEnc->size;
  memcpy(mCodedFrame->VideoData(), mPacketEnc->data, mPacketEnc->size);

  Frame::Header* commonHeader = mCodedFrame->GetHeader();
  commonHeader->Timestamp = 0;
  commonHeader->Key = (mPacketEnc->flags & AV_PKT_FLAG_KEY);

  commonHeader->Width = mFrame->width;
  commonHeader->Height = mFrame->height;
  return true;
}

void FfmpegCodec::DebugOutFrame(const QString& prefix, int counter, const AVFrameS& frame)
{
#ifndef QT_NO_DEBUG
  if (counter == 0) {
    QDir dir(QDir::current());
    dir.mkdir("codec");
  }
  QString filename = QString("./codec/%1_%2.jpg").arg(prefix).arg(counter, 6, 10, QChar('0'));
  QImage shot(frame->width, frame->height, QImage::Format_RGB888);

  int srcStride = frame->linesize[0];
  for (int j = 0; j < frame->height; j++) {
    char* dst = (char*)shot.scanLine(j);
    const char* src = (const char*)frame->data[0] + j*srcStride;
    for (int i = 0; i < frame->width; i++) {
      *dst++ = *src;
      *dst++ = *src;
      *dst++ = *src++;
    }
  }
  shot.save(filename, "JPG", 95);
#else
  Q_UNUSED(prefix);
  Q_UNUSED(counter);
  Q_UNUSED(frame);
#endif
}


FfmpegCodec::FfmpegCodec()
  : mCodecDec(nullptr), mCodecEnc(nullptr), mFrame(nullptr)
{
  FfmpegRegister();
}

FfmpegCodec::~FfmpegCodec()
{
}
