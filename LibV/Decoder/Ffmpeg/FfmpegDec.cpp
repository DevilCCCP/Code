#include <Lib/Log/Log.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <libavutil/mem.h>
#include <libavutil/version.h>
#include <libavutil/avutil.h>
#include <libavutil/pixfmt.h>

#ifdef __cplusplus
}
#endif

#include "FfmpegDec.h"
#include "../Thumbnail.h"


static void FfmpegRegister()
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

inline void av_codec_context_dtor(AVCodecContext* context)
{
  if (context) {
    avcodec_close(context);
    av_free(context);
  }
}

bool FfmpegDec::InitDecoder(AVCodecID _CodecId, const char* frameData)
{
  mCodecId = _CodecId;
  Log.Trace("Ffmpeg::InitDecoder");
  mPacket = AVPacketS(new AVPacket());
  av_init_packet(mPacket.data());

//  for (AVCodec* codec = av_codec_next(nullptr); codec; codec = av_codec_next(codec)) {
//    Log.Trace(QString("Codec: %1").arg(codec->name));
//  }
//  for (mHwAccel = av_hwaccel_next(nullptr); mHwAccel; mHwAccel = av_hwaccel_next(mHwAccel)) {
//    if (mHwAccel->id == mCodecId) {
//      Log.Info(QString("Using hardware acceleration '%1'").arg(mHwAccel->name));
//      break;
//    }
//  }

  AVCodec* codec = avcodec_find_decoder(mCodecId);
  if (!codec) {
    Log.Warning(QString("ffmpeg: decoder not found (codec: %1)").arg(mCodecId));
    return false;
  }

  mContext.reset(avcodec_alloc_context3(codec), av_codec_context_dtor);
  if (!mContext) {
    Log.Warning(QString("ffmpeg: Could not allocate video codec context"));
    return false;
  }
  if (mCodecId == AV_CODEC_ID_AAC) {
    mExtraData.clear();
    mExtraData.append(frameData, 2);
    mContext->extradata = (uchar*)mExtraData.data();
    mContext->extradata_size = mExtraData.size();
  }
  //mContext->refcounted_frames = 1;
  //mContext->pix_fmt = AV_PIX_FMT_RGB32;

  if (avcodec_open2(mContext.data(), codec, nullptr) < 0) {
    Log.Warning(QString("ffmpeg: Could not open codec"));
    return false;
  }

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
  mFrame = AVFrameS(av_frame_alloc(), av_frame_dtor);
#else
  mFrame = AVFrameS(avcodec_alloc_frame(), av_frame_dtor);
#endif

  if (!mFrame) {
    Log.Warning(QString("ffmpeg: Could not allocate video frame"));
    return false;
  }

  mCodec = codec;
  av_log_set_level(AV_LOG_ERROR);
  return true;
}

void FfmpegDec::SetDestCompression(ECompression _DestCompression)
{
  switch (_DestCompression) {
  case eRawRgb:  mDestPixelFormat = AV_PIX_FMT_RGB24; mDestCompression = eRawRgb; break;
  case eRawNv12: mDestPixelFormat = AV_PIX_FMT_NV12;  mDestCompression = eRawNv12; break;
  case eRawRgba: mDestPixelFormat = AV_PIX_FMT_BGRA;  mDestCompression = eRawRgba; break;
  default:       mDestPixelFormat = AV_PIX_FMT_RGB24; mDestCompression = eRawRgb; break;
  }
}

bool FfmpegDec::Decode(char *frameData, int frameSize, int width, int height, bool canSkip)
{
  mDecodedFrame.clear();

  if (!mCodec) {
    Log.Warning("ffmpeg: decode fail, init codec first");
    return false;
  }
  //if (frameSize + AV_INPUT_BUFFER_PADDING_SIZE < frameSize + AV_INPUT_BUFFER_PADDING_SIZE) {
  //  Log.Warning("ffmpeg: not enough size for AV_INPUT_BUFFER_PADDING_SIZE");
  //  return false;
  //} else {
  //  memset(frameData + frameSize, 0, AV_INPUT_BUFFER_PADDING_SIZE);
  //}

  mPacket->data = (uchar*)frameData;
  mPacket->size = frameSize;

  mFrame->width = width;
  mFrame->height = height;

  for (int size = 0; mPacket->size > 0; mPacket->data += size, mPacket->size -= size) {
    int frame;
    size = avcodec_decode_video2(mContext.data(), mFrame.data(), &frame, mPacket.data());
    if (size < 0) {
      char errText[800 + 1];
      av_strerror(size, errText, 800);
      Log.Warning(QString("ffmpeg: Error while decoding frame (code: %1, text: %2)").arg(size, 0, 16).arg(errText));
      return false;
    } else if (size == 0) {
      Log.Warning("ffmpeg: decoding 0 uchars");
      return false;
    } else if (frame) {
      return (!canSkip)? CreateFrame(): SkipFrame();
    }
    //av_frame_unref(mFrame);
  }
  return false;
}

bool FfmpegDec::DecodeAudio(char *frameData, int frameSize)
{
  mDecodedFrame.clear();

  if (!mCodec) {
    Log.Warning("ffmpeg: decode fail, init codec first");
    return false;
  }

  //if (mCodecId == AV_CODEC_ID_AAC) {
  //  frameData += 2;
  //  frameSize -= 2;
  //}
  mPacket->data = (uchar*)frameData;
  mPacket->size = frameSize;

  for (int size = 0; mPacket->size > 0; mPacket->data += size, mPacket->size -= size) {
    int frame = 0;
    size = avcodec_decode_audio4(mContext.data(), mFrame.data(), &frame, mPacket.data());
    if (size < 0) {
      char errText[800 + 1];
      av_strerror(size, errText, 800);
      Log.Warning(QString("ffmpeg: Error while decoding audio frame (code: %1, text: %2)").arg(size, 0, 16).arg(errText));
      return false;
    } else if (size == 0) {
      Log.Warning("ffmpeg: decoding audio 0 uchars");
      return false;
    } else if (frame) {
      return CreateAudioFrame();
    }
    //av_frame_unref(mFrame);
  }
  return false;
}

bool FfmpegDec::CreateFrame()
{
  //Log.Trace("ffmpeg: frame decoded");
  mDecodedFrame = FrameS(new Frame());
  if ((AVPixelFormat)mFrame->format == AV_PIX_FMT_YUV420P && mDestPixelFormat == AV_PIX_FMT_NV12) {
    if (!ConvertVideoFrameYuvDest()) {
      return false;
    }
  } else if ((AVPixelFormat)mFrame->format != mDestPixelFormat) {
    if (!ConvertVideoFrameDest()) {
      return false;
    }
  } else {
    int lineSize = mFrame->linesize[0];
    int decodedSize = lineSize * mFrame->height * 3 / 2;

    mDecodedFrame->ReserveData(decodedSize);
    Frame::Header* header = mDecodedFrame->InitHeader();
    header->Compression = mDestCompression;
    header->VideoDataSize = decodedSize;
    header->Size = decodedSize;
    memcpy(mDecodedFrame->VideoData(), (char*)mFrame->data[0], lineSize * mFrame->height);
    memcpy(mDecodedFrame->VideoData() + lineSize * mFrame->height, (char*)mFrame->data[1], lineSize * mFrame->height / 2);
  }

  Frame::Header* commonHeader = mDecodedFrame->GetHeader();
  commonHeader->Timestamp = 0;
  commonHeader->Key = true;

  commonHeader->Width = mFrame->width;
  commonHeader->Height = mFrame->height;

  if (mThumbnail && mThumbnail->IsTimeToCreate()) {
    if (EncodeJpeg()) {
      mThumbnail->Create(QByteArray((const char*)mPacketJ->data, mPacketJ->size));
      av_free_packet(mPacketJ.data());
    } else {
      mThumbnail->Create(QByteArray());
    }
  }
  return true;
}

bool FfmpegDec::SkipFrame()
{
  //Log.Trace("ffmpeg: frame skipped");
  mDecodedFrame = FrameS();
  return true;
}

bool FfmpegDec::ConvertVideoFrameDest()
{
#ifndef SWS_SCALE_BUG
  mSwsContext = sws_getCachedContext(mSwsContext, mFrame->width, mFrame->height, (AVPixelFormat)mFrame->format
                                     , mFrame->width, mFrame->height, mDestPixelFormat, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);
#else
  mSwsContext = sws_getCachedContext(mSwsContext, mFrame->width, mFrame->height, (AVPixelFormat)mFrame->format
                                     , mFrame->width-1, mFrame->height, mDestPixelFormat, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);
#endif

  int dstStrides[8] = { 0 };
  av_image_fill_linesizes(dstStrides, mDestPixelFormat, mFrame->width);

  int decodedSize = avpicture_get_size(mDestPixelFormat, mFrame->width, mFrame->height);
  mDecodedFrame->ReserveData(decodedSize);

  uint8_t* dstSlices[8] = { (uint8_t*)mDecodedFrame->VideoData() };
  if (dstStrides[1] > 0 && dstStrides[0] > 0) {
    int stride = dstStrides[0];
    dstSlices[1] = (uint8_t*)mDecodedFrame->VideoData() + stride * mFrame->height;
  }

  int height = sws_scale(mSwsContext, mFrame->data, mFrame->linesize, 0, mFrame->height, dstSlices, dstStrides);
  if (height != mFrame->height) {
    Log.Warning(QString("ffmpeg: reformat fail (src: %1x%2(%3), dst height: %4").arg(mFrame->width).arg(mFrame->height).arg(mFrame->format)
                .arg(height));
    return false;
  }
  Frame::Header* header = mDecodedFrame->InitHeader();
  header->Compression = mDestCompression;
  header->VideoDataSize = decodedSize;
  header->Size = decodedSize;
  return true;
}

bool FfmpegDec::ConvertVideoFrameYuvDest()
{
  int dstStrides2[8] = { 0 };
  av_image_fill_linesizes(dstStrides2, AV_PIX_FMT_YUVJ420P, mFrame->width);

  int decodedSize2 = avpicture_get_size(AV_PIX_FMT_YUVJ420P, mFrame->width, mFrame->height);

  mSwsContextMid = sws_getCachedContext(mSwsContextMid, mFrame->width, mFrame->height, (AVPixelFormat)mFrame->format
                                     , mFrame->width, mFrame->height, AV_PIX_FMT_YUVJ420P, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);

  mBufferMid.resize(decodedSize2);

  uint8_t* dstSlices2[8] = { (uint8_t*)mBufferMid.data()
                             , (uint8_t*)mBufferMid.data() + dstStrides2[0] * mFrame->height
                             , (uint8_t*)mBufferMid.data() + dstStrides2[0] * mFrame->height + dstStrides2[1] * mFrame->height / 2 };

  int height2 = sws_scale(mSwsContextMid, mFrame->data, mFrame->linesize, 0, mFrame->height, dstSlices2, dstStrides2);
  if (height2 != mFrame->height) {
    Log.Warning(QString("ffmpeg: reformat fail (src: %1x%2(%3), dst height: %4").arg(mFrame->width).arg(mFrame->height)
                .arg(mFrame->format).arg(height2));
    return false;
  }

  mSwsContext = sws_getCachedContext(mSwsContext, mFrame->width, mFrame->height, AV_PIX_FMT_YUVJ420P
                                     , mFrame->width, mFrame->height, mDestPixelFormat, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);

  int dstStrides[8] = { 0 };
  av_image_fill_linesizes(dstStrides, mDestPixelFormat, mFrame->width);

  int stride = dstStrides[0];
  int decodedSize = avpicture_get_size(mDestPixelFormat, mFrame->width, mFrame->height);
  mDecodedFrame->ReserveData(decodedSize);

  uint8_t* dstSlices[8] = { (uint8_t*)mDecodedFrame->VideoData()
                            , (stride)? (uint8_t*)mDecodedFrame->VideoData() + stride * mFrame->height: 0 };

  int height = sws_scale(mSwsContext, dstSlices2, dstStrides2, 0, height2, dstSlices, dstStrides);
  if (height != height2) {
    Log.Warning(QString("ffmpeg: reformat fail (src: %1x%2(%3), dst height: %4").arg(mFrame->width).arg(mFrame->height).arg(mFrame->format)
                .arg(height));
    return false;
  }
  Frame::Header* header = mDecodedFrame->InitHeader();
  header->Compression = mDestCompression;
  header->VideoDataSize = decodedSize;
  header->Size = decodedSize;
  return true;
}

bool FfmpegDec::CreateAudioFrame()
{
  //Log.Trace("ffmpeg: frame decoded audio");
  mDecodedFrame = FrameS(new Frame());
  if ((AVSampleFormat)mFrame->format != AV_SAMPLE_FMT_FLTP) {
    LOG_ERROR_ONCE(QString("Audio need to be reconverted (format: %1)").arg(mFrame->format));
    return false;
  }

  int decodedSize = av_samples_get_buffer_size(nullptr, mContext->channels, mFrame->nb_samples, mContext->sample_fmt, 1);
  if (mContext->channels > 1) {
    decodedSize /= mContext->channels;
  }

  mDecodedFrame->ReserveData(decodedSize);
  Frame::Header* header = mDecodedFrame->InitHeader();
  header->CompressionAudio = eRawAuF16;
  header->VideoDataSize = 0;
  header->AudioDataSize = decodedSize;
  header->Size = decodedSize;
  memcpy(mDecodedFrame->AudioData(), (char*)mFrame->data[0], decodedSize);

  header->Timestamp = 0;
  header->Key = true;

  header->Width = 0;
  header->Height = 0;
  return true;
}

bool FfmpegDec::InitJ()
{
  mPacketJ = AVPacketS(new AVPacket());
  av_init_packet(mPacketJ.data());

  AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (!mContext) {
    Log.Warning(QString("ffmpeg: main context not initialized"));
  }
  if (!codec) {
    Log.Warning(QString("ffmpeg: jpeg encoder not found"));
    return false;
  }

  mContextJ.reset(avcodec_alloc_context3(codec), av_codec_context_dtor);
  if (!mContextJ) {
    Log.Warning(QString("ffmpeg: Could not allocate video codec context (jpeg)"));
    return false;
  }

  mContextJ->bit_rate      = mContext->bit_rate;
  mContextJ->width         = mContext->width;
  mContextJ->height        = mContext->height;
  mContextJ->pix_fmt       = AV_PIX_FMT_YUVJ420P;
  mContextJ->codec_id      = AV_CODEC_ID_MJPEG;
  mContextJ->codec_type    = AVMEDIA_TYPE_VIDEO;
  mContextJ->time_base.num = 1;
  mContextJ->time_base.den = 30;

  mContextJ->mb_lmin        = mContextJ->qmin * FF_QP2LAMBDA;
  mContextJ->mb_lmax        = mContextJ->qmax * FF_QP2LAMBDA;
  mContextJ->flags          = AV_CODEC_FLAG_QSCALE;
  mContextJ->global_quality = mContextJ->qmin * FF_QP2LAMBDA;

  if (avcodec_open2(mContextJ.data(), codec, nullptr) < 0) {
    Log.Warning(QString("ffmpeg: Could not open codec (jpeg)"));
    return false;
  }

  mCodecJ = codec;
  return true;
}

bool FfmpegDec::EncodeJpeg()
{
  if (!mCodecJ || mContextJ->width != mContext->width || mContextJ->height != mContext->height) {
    if (!InitJ()) {
      return false;
    }
  }

  int got = 0;
  int ret = avcodec_encode_video2(mContextJ.data(), mPacketJ.data(), mFrame.data(), &got);
  if (ret < 0 || got <= 0) {
    Log.Warning(QString("Jpeg encode fail (err: %1)").arg(ret));
    return false;
  }
  return true;
}

FfmpegDec::FfmpegDec(const ThumbnailS& _Thumbnail)
  : mCodec(nullptr), mHwAccel(nullptr), mFrame(nullptr), mSwsContext(nullptr), mSwsContextMid(nullptr), mDestCompression(eRawNv12)
  , mThumbnail(_Thumbnail), mCodecJ(nullptr)
{
  FfmpegRegister();
}

FfmpegDec::~FfmpegDec()
{
}
