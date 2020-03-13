#pragma once

#include <QByteArray>

#ifdef __cplusplus
extern "C" {
#endif

#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/imgutils.h>

#ifdef __cplusplus
}
#endif

#include <LibV/Include/Frame.h>

DefineClassS(FfmpegDec);
DefineClassS(Thumbnail);
DefineStructS(AVCodec);
DefineStructS(AVCodecContext);
DefineStructS(AVFrame);
DefineStructS(AVPacket);
DefineStructS(SwsContext);

class FfmpegDec
{
  FrameS          mDecodedFrame;

  AVCodecID       mCodecId;
  AVCodec*        mCodec;
  AVHWAccel*      mHwAccel;
  AVCodecContextS mContext;
  QByteArray      mExtraData;
  AVFrameS        mFrame;
  AVPacketS       mPacket;
  SwsContext*     mSwsContext;
  SwsContext*     mSwsContextMid;
  QByteArray      mBufferMid;
  AVPixelFormat   mDestPixelFormat;
  ECompression    mDestCompression;

  ThumbnailS      mThumbnail;
  AVCodec*        mCodecJ;
  AVCodecContextS mContextJ;
  AVPacketS       mPacketJ;

public:
  FrameS& DecodedFrame() { return mDecodedFrame; }

  bool InitDecoder(AVCodecID _CodecId, const char* frameData);
  void SetDestCompression(ECompression _DestCompression);

public:
  bool Decode(char* frameData, int frameSize, int width, int height, bool canSkip);
  bool DecodeAudio(char* frameData, int frameSize);

private:
  bool CreateFrame();
  bool SkipFrame();
  void CreateThumbnail();
  bool ConvertVideoFrameDest();
  bool ConvertVideoFrameYuvDest();
  bool CreateAudioFrame();

  bool InitJ();
  bool EncodeJpeg();

public:
  FfmpegDec(const ThumbnailS& _Thumbnail);
  ~FfmpegDec();
};

