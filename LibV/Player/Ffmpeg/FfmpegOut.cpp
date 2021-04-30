#include <Lib/Log/Log.h>

#include "FfmpegOut.h"


const int kFrameBufferSize = 30;

static void FfmpegRegister()
{
  static volatile bool gInit = false;
  if (!gInit) {
    gInit = true;
    av_register_all();
    Log.Info("av_register_all");
  }
}

bool FfmpegOut::Open(const QString& filename)
{
  if (!CloseFile()) {
    return false;
  }

  mFilename = filename;
  Log.Info(QString("Ffmpeg: file creating (filename: '%1')").arg(mFilename));

  mCompression = eCmprNone;
  mCompressionA = eCmprNone;
  mTimestamp = mTimestampA = 0;

  mWidth = mHeight = 0;

  mErrVideo = mErrAudio = 0;
  return true;
}

bool FfmpegOut::WriteNext(FrameS& frame)
{
  Frame::Header* header = frame->GetHeader();
  bool comprChanged = false;
  if ((header->Compression == mCompression || header->Compression == eCmprNone)
      && (header->CompressionAudio == mCompressionA || header->CompressionAudio == eCmprNone)) {
    comprChanged = false;
  } else if (!mInit) {
    if (mCompression == eCmprNone) {
      mCompression = header->Compression;
      mWidth = header->Width;
      mHeight = header->Height;
    } else if (header->Compression != mCompression && header->Compression != eCmprNone) {
      comprChanged = true;
    }
    if (mCompressionA == eCmprNone) {
      mCompressionA = header->CompressionAudio;
    } else if (header->CompressionAudio != mCompressionA && header->CompressionAudio != eCmprNone) {
      comprChanged = true;
    }
  } else {
    comprChanged = true;
  }

  if (comprChanged) {
    Log.Error(QString("Ffmpeg: Unimplemented change of compression during file write (old: %1/%2, new: %3/%4)")
              .arg(mCompression).arg(mCompressionA).arg(header->Compression).arg(header->CompressionAudio));
    return false;
  }

  if (mFrameBuffer.size() < kFrameBufferSize) {
    mFrameBuffer.append(frame);
  } else {
    return FlushFrames() && WriteFile(frame);
  }
  return true;
}

bool FfmpegOut::Close()
{
  return CloseFile();
}

bool FfmpegOut::PrepareFile()
{
  AVFormatContext* fcontext;
#if LIBAVFORMAT_VERSION_MAJOR >= 56
  int ret = avformat_alloc_output_context2(&fcontext, nullptr, nullptr, mFilename.toUtf8().constData());
  if (ret < 0) {
    Log.Error(QString("Ffmpeg: alloc output context fail (filename: '%1', code: %2)").arg(mFilename).arg(ret, 8, 16));
    return false;
  }
#else
  fcontext = avformat_alloc_context();
  int ret = !fcontext? -1: 0;
  if (ret < 0) {
    Log.Error(QString("Ffmpeg: alloc output context fail (filename: '%1', code: %2)").arg(mFilename).arg(ret, 8, 16));
    return false;
  }
#endif

  if (!fcontext) {
      return false;
  }

  mFileContext = AVFormatContextS(fcontext, avformat_free_context);
  if (!mFileContext || !mFileContext.data()) {
      return false;
  }
  mFileContext->ctx_flags = AVFMTCTX_NOHEADER;

  mVideoStream = nullptr;
  if (mCompression != eCmprNone) {
    Log.Info(QString("Ffmpeg: Saving video stream (compression: %1)").arg(CompressionToString(mCompression)));
    AVCodecID codecId = AV_CODEC_ID_NONE;
    switch (mCompression) {
    case eH264:  codecId = AV_CODEC_ID_H264; break;
    case eMpeg4: codecId = AV_CODEC_ID_MPEG4; break;
    case eJpeg:  codecId = AV_CODEC_ID_MJPEG; break;
    default:     codecId = AV_CODEC_ID_NONE; break;
    }
    if (codecId != AV_CODEC_ID_NONE) {
      AVCodec* codec = avcodec_find_encoder(codecId);
      if (!codec) {
        Log.Error(QString("Ffmpeg: Codec not found (id: %1, compr: %2)").arg(codecId).arg(CompressionToString(mCompression)));
        return false;
      }
      mVideoStream = avformat_new_stream(mFileContext.data(), codec);
      //AVRational time = { 1, 0 };
      //mVideoStream->r_frame_rate = mVideoStream->avg_frame_rate = time;
      mTimeBase.num = 1;
      mTimeBase.den = 90000;
      mVideoStream->time_base = mVideoStream->codec->time_base = mTimeBase;

      mVideoStream->codec->width = mWidth;
      mVideoStream->codec->height = mHeight;
      mVideoStream->codec->pix_fmt = codec->pix_fmts[0];
      if (mFileContext->oformat->flags & AVFMT_GLOBALHEADER) {
        mVideoStream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
      }

      ret = avcodec_open2(mVideoStream->codec, codec, NULL);
      if (ret < 0) {
        Log.Error(QString("Ffmpeg: Codec open fail (id: %1, compr: %2)").arg(codecId).arg(CompressionToString(mCompression)));
        return false;
      }
    }
  }
  mAudioStream = nullptr;
  if (mCompressionA != eCmprNone) {
    Log.Info(QString("Ffmpeg: Saving audio stream (compression: %1)").arg(CompressionToString(mCompressionA)));
    AVCodecID codecId = AV_CODEC_ID_NONE;
    switch (mCompressionA) {
    case eAac16b: codecId = AV_CODEC_ID_AAC; break;
    default:     codecId = AV_CODEC_ID_NONE; break;
    }
    if (codecId != AV_CODEC_ID_NONE) {
      AVCodec* codec = avcodec_find_encoder(codecId);
      if (!codec) {
        Log.Error(QString("Ffmpeg: Codec not found (id: %1, compr: %2)").arg(codecId).arg(CompressionToString(mCompressionA)));
        return false;
      }
      mAudioStream = avformat_new_stream(mFileContext.data(), codec);
      mTimeBaseA.num = 1;
      mTimeBaseA.den = 16000;

      mAudioStream->codec->sample_rate = 16000;
      mAudioStream->codec->sample_fmt = AV_SAMPLE_FMT_FLTP;
      mExtraDataA.clear();
      mExtraDataA.append(0x14);
      mExtraDataA.append(0x08);
      mExtraDataA.reserve(mExtraDataA.size() + AV_INPUT_BUFFER_PADDING_SIZE);
      mAudioStream->codec->extradata = (uchar*)mExtraDataA.data();
      mAudioStream->codec->extradata_size = mExtraDataA.size();
    }
  }

  if (mFileContext->oformat->flags & AVFMT_NOFILE) {
    Log.Warning("Ffmpeg: flags & AVFMT_NOFILE");
    return false;
  }
  ret = avio_open2(&mFileContext->pb, mFilename.toUtf8().constData(), AVIO_FLAG_WRITE, nullptr, nullptr);
  if (ret < 0) {
    Log.Error(QString("Ffmpeg: open file fail (filename: '%1', code: %2)").arg(mFilename).arg(ret, 8, 16));
    return false;
  }

  ret = avformat_write_header(mFileContext.data(), nullptr);
  if (ret < 0) {
    Log.Error(QString("Ffmpeg: open file fail (filename: '%1', code: %2)").arg(mFilename).arg(ret, 8, 16));
    return false;
  }

  mPacket = AVPacketS(new AVPacket());
  av_init_packet(mPacket.data());

  mInit = true;
  Log.Info(QString("Ffmpeg: file saving (filename: '%1')").arg(mFilename));
  return true;
}

bool FfmpegOut::CloseFile()
{
  if (!FlushFrames()) {
    return false;
  }
  while (!mFrameBuffer.isEmpty()) {
    FrameS frame = mFrameBuffer.takeFirst();
    WriteFile(frame);
  }

  if (!mInit) {
    return true;
  }
  if (!FinalFile()) {
    return false;
  }
  mInit = false;
  return true;
}

bool FfmpegOut::FinalFile()
{
  int ret = av_write_trailer(mFileContext.data());
  if (ret == 0) {
    Log.Info(QString("Ffmpeg: file saved (filename: '%1')").arg(mFilename));
  } else {
    Log.Warning(QString("Ffmpeg: final file fail (code: %1)").arg(ret, 8, 16));
  }
  if (mAudioStream) {
    mAudioStream->codec->extradata = 0;
    mAudioStream->codec->extradata_size = 0;
  }
  if (mFileContext && mFileContext->pb) {
    avio_close(mFileContext->pb);
  }
  mFileContext.clear();
  return true;
}

bool FfmpegOut::WriteFile(FrameS &frame)
{
  if (!mInit) {
    if (!PrepareFile()) {
      return false;
    }
    mErrVideo = mErrAudio = 0;
  }

  if (mVideoStream && frame->VideoDataSize() > 0) {
    mPacket->flags = frame->GetHeader()->Key? AV_PKT_FLAG_KEY: 0;
    mPacket->stream_index = mVideoStream->index;
    mPacket->data = (uint8_t*)frame->VideoData();
    mPacket->size = frame->VideoDataSize();
    qint64 ts = frame->GetHeader()->Timestamp * mTimeBase.den / mTimeBase.num / 1000;
    if (mTimestamp == 0) {
      mTimestamp = ts;
    }
    mPacket->dts = mPacket->pts = ts - mTimestamp;

    int ret = av_interleaved_write_frame(mFileContext.data(), mPacket.data());
    if (ret < 0) {
      if (!mErrVideo) {
        Log.Warning(QString("Ffmpeg: write video frame fail (code: %1)").arg(ret, 8, 16));
      }
      mErrVideo++;
    } else if (mErrVideo) {
      Log.Info(QString("Ffmpeg: write video frame ok (fails: %1)").arg(mErrVideo));
      mErrVideo = 0;
    }
  }
  if (mAudioStream && frame->AudioDataSize() > 0) {
    mPacket->flags = AV_PKT_FLAG_KEY;
    mPacket->stream_index = mAudioStream->index;
    mPacket->data = (uint8_t*)frame->AudioData();
    mPacket->size = frame->AudioDataSize();
    qint64 ts = frame->GetHeader()->Timestamp * mTimeBaseA.den / mTimeBaseA.num / 1000;
    if (mTimestampA == 0) {
      mTimestampA = ts;
    }
    mPacket->dts = mPacket->pts = ts - mTimestampA;
//    mPacket->dts = mPacket->pts = AV_NOPTS_VALUE;
    int ret = av_interleaved_write_frame(mFileContext.data(), mPacket.data());
    if (ret < 0) {
      if (!mErrAudio) {
        Log.Warning(QString("Ffmpeg: write audio frame fail (code: %1)").arg(ret, 8, 16));
      }
      mErrAudio++;
    } else if (mErrAudio) {
      Log.Info(QString("Ffmpeg: write audio frame ok (fails: %1)").arg(mErrAudio));
      mErrAudio = 0;
    }
  }
  return true;
}

bool FfmpegOut::FlushFrames()
{
  while (!mFrameBuffer.isEmpty()) {
    FrameS frame = mFrameBuffer.takeFirst();
    if (!WriteFile(frame)) {
      return false;
    }
  }
  return true;
}

int FfmpegOut::CalcBestFps()
{
  if (mFrameBuffer.size() > 4) {
    qint64 ts1 = mFrameBuffer.first()->GetHeader()->Timestamp;
    qint64 ts2 = mFrameBuffer.last()->GetHeader()->Timestamp;
    if (ts2 > ts1) {
      float dts = (float)(ts2 - ts1);
      float fps = (mFrameBuffer.size() - 1) * 1000.0f / dts;
      return (int)fps;
    }
  }
  Log.Warning("Ffmpeg: FPS not defined, used default");
  return 25;
}

void FfmpegOut::DumpFrame(FrameS& frame)
{
  Q_UNUSED(frame);
//  static int gCounter = 0;
//  if (gCounter++ < 200) {
//    FILE* f = fopen(QString("frame_%1.bin").arg(gCounter, 3, 10, QChar('0')).toLatin1(), "wb");
//    fwrite(frame->VideoData(), 1, frame->VideoDataSize(), f);
//    fclose(f);
//  }
}


FfmpegOut::FfmpegOut()
  : mInit(false), mVideoStream(nullptr), mAudioStream(nullptr)
  , mCompression(eCmprNone), mCompressionA(eCmprNone), mTimestamp(0), mTimestampA(0), mWidth(0), mHeight(0)
  , mErrVideo(0), mErrAudio(0)
{
  FfmpegRegister();
}

FfmpegOut::~FfmpegOut()
{
}
