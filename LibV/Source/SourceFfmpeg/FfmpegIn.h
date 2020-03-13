#pragma once

#include <QElapsedTimer>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavdevice/avdevice.h>
#include <libswscale/swscale.h>
#ifdef __cplusplus
}
#endif

#include <LibV/Include/Frame.h>
#include "../Source.h"

DefineClassS(FfmpegIn);
DefineClassS(Thumbnail);
DefineStructS(AVFormatContext);
DefineStructS(AVCodecContext);
DefineStructS(AVPacket);
DefineStructS(AVFrame);
DefineStructS(AVRational);

class FfmpegIn
{
  Source*           mSource;
  QString           mPath;
  St::EType         mType;
  bool              mStreamOpened;
  bool              mStreamRead;
  QElapsedTimer     mStreamOpenTimer;
  AVFormatContextS  mFileContext;
  int               mStreamIndex;
  int               mStreamAIndex;
  AVPixelFormat     mPixFormat;

  ECompression      mCompression;
  ECompression      mCompressionA;
  bool              mChangeColorspace;
  int               mWidth;
  int               mHeight;
  int               mBitrate;

  AVRational        mTimeBase;
  AVRational        mTimeBaseA;
  bool              mFirstFrame;
  qint64            mLastTimestamp;

  AVPacketS         mPacket;
  SwsContext*       mSwsContext;
  bool              mNeedRegister;
  bool              mWarning;

  ThumbnailS        mThumbnail;
  AVCodec*          mCodecJ;
  AVCodecContextS   mContextJ;
  AVFrameS          mFrame;
  AVPacketS         mPacketJ;
  SwsContext*       mSwsContextJ;

public:
  bool Open(const QString& filename, St::EType type, const QString& extraOptions);
  bool SeekTime(const qint64& posMs);
  bool ReadNext(FrameS& frame);
  void CloseStream();

private:
  int OpenCallback();
  Frame::Header* InitFrame(FrameS& frame, int fullSize, int key);

  bool ConvertToNv12(FrameS& frame);
  bool ConvertToJpegYuv();

  bool InitJ();
  bool EncodeJpeg();

public:
  FfmpegIn(Source* _Source, const ThumbnailS& _Thumbnail);
  ~FfmpegIn();

  friend int InterruptCallback(void* _FfmpegIn);
};

