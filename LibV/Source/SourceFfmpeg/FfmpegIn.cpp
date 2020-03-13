#include <qsystemdetection.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif

#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>
#include <LibV/Decoder/Thumbnail.h>

#include "FfmpegIn.h"


static void FfmpegRegister()
{
  static volatile bool gInit = false;
  if (!gInit) {
    gInit = true;
    av_register_all();
    Log.Info("av_register_all");
  }
}

int InterruptCallback(void* _FfmpegIn)
{
  FfmpegIn* ffmpegIn = static_cast<FfmpegIn*>(_FfmpegIn);
  return ffmpegIn->OpenCallback();
}

void FreeAvformatContext(AVFormatContext* fcontext)
{
  avformat_close_input(&fcontext);
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

class AVPacketCloser {
  AVPacket* mData;

public:
  AVPacketCloser(AVPacket* _Data): mData(_Data) { }
  ~AVPacketCloser() { av_free_packet(mData); }
};

inline void av_codec_context_dtor(AVCodecContext* context)
{
  if (context) {
    avcodec_close(context);
    av_free(context);
  }
}

bool FfmpegIn::Open(const QString &filename, St::EType type, const QString& extraOptions)
{
  mPath = filename;
  mType = type;

  mFileContext.clear();
  if (mNeedRegister) {
    if (mType & St::eRtsp) {
      avformat_network_init();
    } else if (mType & St::eUsb) {
      avdevice_register_all();
    }
    mNeedRegister = false;
  }

  AVFormatContext* fcontext = avformat_alloc_context();
  if (!fcontext) {
    if (!mWarning) {
      Log.Warning(QString("Ffmpeg: avformat_alloc_context fail"));
      mWarning = true;
    }
    return false;
  }
  AVDictionary *opts = 0;
  if (mType == St::eRtspTcp) {
    av_dict_set(&opts, "rtsp_transport", "tcp", 0);
  } else if (mType == St::eUsb && !extraOptions.isEmpty()) {
    QStringList settings = extraOptions.split('|', QString::KeepEmptyParts);
    if (settings.size() > 0) {
      av_dict_set(&opts, "video_size", settings[0].toLatin1().constData(), 0);
    }
    if (settings.size() > 1) {
      av_dict_set(&opts, "framerate", settings[1].toLatin1().constData(), 0);
    }
  }

  fcontext->interrupt_callback.callback = InterruptCallback;
  fcontext->interrupt_callback.opaque = this;

  //avio_open2(..);
  mStreamRead = false;
  mStreamOpenTimer.start();
#ifdef Q_OS_WIN32
  const char* kUsbLib = "vfwcap";
#elif defined(Q_OS_UNIX)
  const char* kUsbLib = "video4linux2";
#endif
  AVInputFormat* fformat = (mType & St::eUsb)? av_find_input_format(kUsbLib): nullptr;
  int ret = avformat_open_input(&fcontext, mPath.toUtf8().constData(), fformat, mType? &opts: nullptr);
  if (ret < 0) {
    if (mStreamOpened) {
      if (!mWarning) {
        Log.Warning(QString("Ffmpeg: open file fail (mPath: '%1', code: %2(0x%3))").arg(mPath).arg(ret).arg(ret, 0, 16));
        mWarning = true;
      }
    } else {
      Log.Info("Stream closed");
      mStreamOpened = true;
    }
    return false;
  }

  mFileContext = AVFormatContextS(fcontext, FreeAvformatContext);
  ret = avformat_find_stream_info(mFileContext.data(), nullptr);
  if (ret < 0) {
    if (!mWarning) {
      Log.Warning(QString("Ffmpeg: couldn't find stream information (mPath: '%1', code: %2)").arg(mPath).arg(ret, 8, 16));
      mWarning = true;
    }
    return false;
  }

  mStreamIndex = -1;
  mStreamAIndex = -1;
  AVCodecContext* videoCodec = nullptr;
  AVCodecContext* audioCodec = nullptr;
  for (int i = 0; i < (int)mFileContext->nb_streams; i++) {
    AVStream* stream = mFileContext->streams[i];
    AVCodecContext* avc = mFileContext->streams[i]->codec;
    if (avc->codec_type == AVMEDIA_TYPE_VIDEO) {
      mTimeBase = stream->time_base;
      if (videoCodec) {
        Log.Warning("Found extra video stream, not supported, skipped");
      } else {
        videoCodec = avc;
        mStreamIndex = i;
      }
    } else if (avc->codec_type == AVMEDIA_TYPE_AUDIO) {
      mTimeBaseA = stream->time_base;
      if (audioCodec) {
        Log.Warning("Found extra audio stream, not supported, skipped");
      } else {
        audioCodec = avc;
        mStreamAIndex = i;
      }
    }
  }
  if (!videoCodec) {
    if (!mWarning) {
      Log.Warning(QString("Ffmpeg: couldn't find video stream (path: '%1', streams: %2)").arg(mPath).arg(mFileContext->nb_streams));
      mWarning = true;
    }
    return false;
  }

  mChangeColorspace = false;
  switch (videoCodec->codec_id) {
  case AV_CODEC_ID_RAWVIDEO:
    switch (videoCodec->pix_fmt) {
    case AV_PIX_FMT_NV12:    mCompression = eRawNv12; break;
    case AV_PIX_FMT_YUYV422: mCompression = eRawYuvP; break;
    default:                 mCompression = eRawNv12; mChangeColorspace = true; break;
    }
    break;
  case AV_CODEC_ID_H264:     mCompression = eH264; break;
  case AV_CODEC_ID_MPEG4:    mCompression = eMpeg4; break;
  case AV_CODEC_ID_MJPEG:    mCompression = eJpeg; break;
  default:
    if (!mWarning) {
      Log.Warning(QString("Ffmpeg: unsupported video stream compression (path: '%1', compression: %2)")
                  .arg(mPath).arg(videoCodec->codec_id));
      mWarning = true;
    }
    return false;
  }

  mWidth = (videoCodec->width)? videoCodec->width: videoCodec->coded_width;
  mHeight = (videoCodec->height)? videoCodec->height: videoCodec->coded_height;
  mBitrate = mWidth * mHeight / 10;

  if (audioCodec) {
    switch (audioCodec->codec_id) {
    case AV_CODEC_ID_AAC: mCompressionA = eAac16b; break;
    default:
      Log.Warning(QString("Ffmpeg: unsupported audio stream compression (path: '%1', compression: %2)")
                  .arg(mPath).arg(audioCodec->codec_id));
      mStreamAIndex = -1;
    }
  }

  mPacket = AVPacketS(new AVPacket());
  av_init_packet(mPacket.data());
  mStreamRead = true;
  mFirstFrame = true;
  return true;
}

bool FfmpegIn::SeekTime(const qint64& posMs)
{
  AVStream* stream = mFileContext->streams[mStreamIndex];
  int seekTs = (int)(posMs * stream->time_base.num / stream->time_base.den);
  int ret = av_seek_frame(mFileContext.data(), mStreamIndex, seekTs, 0/*AVSEEK_FLAG_ANY*/);
  if (ret < 0) {
    Log.Warning(QString("Seek file '%1' fail (err: 0x%2)").arg(FormatTimeDelta(posMs)).arg(ret, 0, 16));
  }
  return ret >= 0;
}

bool FfmpegIn::ReadNext(FrameS& frame)
{
  while (av_read_frame(mFileContext.data(), mPacket.data()) >= 0) {
    QScopedPointer<AVPacketCloser> packet(new AVPacketCloser(mPacket.data()));
    bool key = (mPacket->flags & AV_PKT_FLAG_KEY)? true: false;
    uint8_t* extraData = nullptr;
    int extraSize = 0;
    if ((mPacket->stream_index == mStreamIndex && mCompression == eH264 && key)
        /*|| (mPacket->stream_index == mStreamAIndex && mCompressionA == eAac16b)*/) {
      extraData = mFileContext->streams[mPacket->stream_index]->codec->extradata;
      extraSize = mFileContext->streams[mPacket->stream_index]->codec->extradata_size;
    }
    int frameFullSize = mPacket->size + extraSize;

    //Log.Trace(QString("Frame (stream: %1, pts: %2, dts: %3, ts: %4)").arg(mPacket->stream_index).arg(mPacket->pts).arg(mPacket->dts)
    //          .arg(mPacket->dts * 1000 * mTimeBase.num / mTimeBase.den));
    if (mPacket->stream_index == mStreamIndex) {
      mPixFormat = mFileContext->streams[mPacket->stream_index]->codec->pix_fmt;
      if (mChangeColorspace) {
        if (!ConvertToNv12(frame)) {
          return false;
        }
      } else {
        Frame::Header* header = InitFrame(frame, frameFullSize, key);
        header->Compression = mCompression;

        if (extraData) {
          memcpy(frame->VideoData(), extraData, extraSize);
        }
        memcpy(frame->VideoData() + extraSize, mPacket->data, mPacket->size);
        header->VideoDataSize = frameFullSize;
      }

      Frame::Header* header = frame->GetHeader();
      header->Width = mWidth;
      header->Height = mHeight;

      if (mFirstFrame) {
        mLastTimestamp = 0;
        if (mTimeBase.num > 0 && mPacket->dts > 0 && mPacket->dts < mTimeBase.den / mTimeBase.num) {
          Log.Info(QString("ffmpeg: clear first frame dts (dts: %1, pts: %2, timebase: %3/%4)")
                     .arg(mPacket->dts).arg(mPacket->pts).arg(mTimeBase.num).arg(mTimeBase.den));
          mPacket->dts = 0;
        }
        mFirstFrame = false;
      }

      if ((quint64)mPacket->dts != (quint64)AV_NOPTS_VALUE) {
        header->Timestamp = 1000 * mPacket->dts * mTimeBase.num / mTimeBase.den;
      } else if (mType == St::eFile) {
        header->Timestamp = mLastTimestamp + 40;
      } else {
        header->Timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
      }

//      Log.Trace(QString("Video frame (size: %1, ts: %2)").arg(frameFullSize).arg(header->Timestamp));
      if ((mCompression & eTypeMask) == eRawVideo && mThumbnail && mThumbnail->IsTimeToCreate()) {
        if (EncodeJpeg()) {
          mThumbnail->Create(QByteArray((const char*)mPacketJ->data, mPacketJ->size));
          av_free_packet(mPacketJ.data());
        } else {
          mThumbnail->Create(QByteArray());
        }
      }

      mLastTimestamp = header->Timestamp;
      return true;
    } else if (mPacket->stream_index == mStreamAIndex) {
      Frame::Header* header = InitFrame(frame, frameFullSize, key);
      header->CompressionAudio = mCompressionA;

      header->AudioDataSize = frameFullSize;
      if (extraData) {
        memcpy(frame->AudioData(), extraData, extraSize);
      }
      memcpy(frame->AudioData() + extraSize, mPacket->data, mPacket->size);

      if ((quint64)mPacket->dts != (quint64)AV_NOPTS_VALUE) {
        header->Timestamp = 1000 * mPacket->dts * mTimeBaseA.num / mTimeBaseA.den;
      } else if (mType == St::eFile) {
        LOG_WARNING_ONCE(QString("No way to sync video and audio, audio skipping"));
        continue;
      } else {
        header->Timestamp = QDateTime::currentDateTime().toMSecsSinceEpoch();
      }

//      Log.Trace(QString("Audio frame (size: %1, ts: %2)").arg(frameFullSize).arg(header->Timestamp));
      return true;
    }
  }
  Log.Trace("av_read_frame fail");
  return false;
}

void FfmpegIn::CloseStream()
{
  mStreamOpened = false;
}

int FfmpegIn::OpenCallback()
{
  if (mSource && mStreamOpened) {
    if (mStreamRead) {
      mSource->OnStatus(Connection::eConnected);
    } else {
      mSource->OnStatus(Connection::eConnecting);
    }
  }
  return mStreamOpened? 0: 1;
}

Frame::Header* FfmpegIn::InitFrame(FrameS& frame, int fullSize, int key)
{
  frame = FrameS(new Frame());
  frame->ReserveData(fullSize);
  Frame::Header* header = frame->InitHeader();
  header->Key = key;
  header->Size = sizeof(Frame::Header) + fullSize;
  return header;
}

bool FfmpegIn::ConvertToNv12(FrameS& frame)
{
  mSwsContext = sws_getCachedContext(mSwsContext, mWidth, mHeight, mPixFormat
                                     , mWidth, mHeight, AV_PIX_FMT_NV12, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);

  int dstStrides[8] = { 0 };
  av_image_fill_linesizes(dstStrides, AV_PIX_FMT_NV12, mWidth);

  int stride = dstStrides[0];
  int decodedSize = avpicture_get_size(AV_PIX_FMT_NV12, mWidth, mHeight);
  Frame::Header* header = InitFrame(frame, decodedSize, true);

  uint8_t* dstSlices[8] = { (uint8_t*)frame->VideoData(), (uint8_t*)frame->VideoData() + stride * mHeight };

  int srcStrides[8] = { 0 };
  av_image_fill_linesizes(srcStrides, mPixFormat, mWidth);
  uint8_t* srcSlices[8] = { (uint8_t*)mPacket->data };
  int disp = srcStrides[0];
  for (int i = 1; i < 8 && srcStrides[i]; i++) {
    srcSlices[i] = (uint8_t*)mPacket->data + srcStrides[i];
    disp += srcStrides[i];
  }
  int height = sws_scale(mSwsContext, srcSlices, srcStrides, 0, mHeight, dstSlices, dstStrides);
  if (height != mHeight) {
    Log.Warning(QString("ffmpeg: reformat NV12 fail (src: %1x%2(%3), dst height: %4").arg(mWidth).arg(mHeight)
                .arg(mPixFormat).arg(mHeight));
    return false;
  }
  header->Compression = eRawNv12;
  header->VideoDataSize = decodedSize;
  return true;
}

bool FfmpegIn::ConvertToJpegYuv()
{
  AVPixelFormat pixFormat = mContextJ->pix_fmt;
  mFrame->format = pixFormat;
  mFrame->width = mWidth;
  mFrame->height = mHeight;
  av_image_fill_linesizes(mFrame->linesize, pixFormat, mWidth);
  if (av_image_alloc(mFrame->data, mFrame->linesize, mFrame->width, mFrame->height, pixFormat, 32) < 0) {
    LOG_WARNING_ONCE(QString("av_image_alloc failed"));
    return false;
  }

  if (mPixFormat != pixFormat) {
    mSwsContextJ = sws_getCachedContext(mSwsContextJ, mWidth, mHeight, mPixFormat
                                        , mWidth, mHeight, pixFormat, SWS_GAUSS
                                        , nullptr, nullptr, nullptr);

    int srcStrides[8] = { 0 };
    av_image_fill_linesizes(srcStrides, mPixFormat, mWidth);
    uint8_t* srcSlices[8] = { (uint8_t*)mPacket->data };
    int disp = srcStrides[0];
    for (int i = 1; i < 8 && srcStrides[i]; i++) {
      srcSlices[i] = (uint8_t*)mPacket->data + srcStrides[i];
      disp += srcStrides[i];
    }
    int height = sws_scale(mSwsContextJ, srcSlices, srcStrides, 0, mHeight, mFrame->data, mFrame->linesize);
    if (height != mHeight) {
      Log.Warning(QString("ffmpeg: reformat jpeg fail (src: %1x%2(%3), dst height: %4").arg(mWidth).arg(mHeight)
                  .arg(mPixFormat).arg(mHeight));
      return false;
    }
  } else {
    av_image_fill_pointers(mFrame->data, mPixFormat, mHeight, mPacket->data, mFrame->linesize);
  }
  return true;
}

bool FfmpegIn::InitJ()
{
  mPacketJ = AVPacketS(new AVPacket());
  av_init_packet(mPacketJ.data());

  AVCodec* codec = avcodec_find_encoder(AV_CODEC_ID_MJPEG);
  if (mWidth <= 0 || mHeight <= 0 || mBitrate <= 0) {
    Log.Warning(QString("ffmpeg: main context not initialized"));
    return false;
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

#if LIBAVCODEC_VERSION_INT >= AV_VERSION_INT(55, 45, 101)
  mFrame.reset(av_frame_alloc(), av_frame_dtor);
#else
  mFrame.reset(avcodec_alloc_frame(), av_frame_dtor);
#endif

  mContextJ->bit_rate      = mBitrate;
  mContextJ->width         = mWidth;
  mContextJ->height        = mHeight;
  mContextJ->pix_fmt       = AV_PIX_FMT_YUVJ420P;
  mContextJ->codec_id      = AV_CODEC_ID_MJPEG;
  mContextJ->codec_type    = AVMEDIA_TYPE_VIDEO;
  mContextJ->time_base.num = 1;
  mContextJ->time_base.den = 30;

  mContextJ->mb_lmin        = mContextJ->qmin * FF_QP2LAMBDA;
  mContextJ->mb_lmax        = mContextJ->qmax * FF_QP2LAMBDA;
  mContextJ->flags          = CODEC_FLAG_QSCALE;
  mContextJ->global_quality = mContextJ->qmin * FF_QP2LAMBDA;

  if (avcodec_open2(mContextJ.data(), codec, nullptr) < 0) {
    Log.Warning(QString("ffmpeg: Could not open codec (jpeg)"));
    return false;
  }

  mCodecJ = codec;
  return true;
}

bool FfmpegIn::EncodeJpeg()
{
  if (!mCodecJ || mContextJ->width != mWidth || mContextJ->height != mHeight) {
    if (!InitJ()) {
      return false;
    }
  }

  if (!ConvertToJpegYuv()) {
    return false;
  }

  int got = 0;
  int ret = avcodec_encode_video2(mContextJ.data(), mPacketJ.data(), mFrame.data(), &got);
  if (ret < 0 || got <= 0) {
    Log.Warning(QString("Jpeg encode fail (err: %1)").arg(ret));
    return false;
  }
  return true;
}

FfmpegIn::FfmpegIn(Source *_Source, const ThumbnailS& _Thumbnail)
  : mSource(_Source), mStreamOpened(true), mStreamRead(false), mFileContext(nullptr)
  , mCompression(eCmprNone), mCompressionA(eCmprNone), mChangeColorspace(false), mWidth(0), mHeight(0), mBitrate(0)
  , mSwsContext(nullptr), mNeedRegister(true), mWarning(false)
  , mThumbnail(_Thumbnail), mCodecJ(nullptr), mSwsContextJ(nullptr)
{
  FfmpegRegister();
}

FfmpegIn::~FfmpegIn()
{
}
