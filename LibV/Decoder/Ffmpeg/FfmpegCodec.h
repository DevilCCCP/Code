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


DefineClassS(FfmpegCodec);
DefineStructS(AVCodec);
DefineStructS(AVCodecContext);
DefineStructS(AVFrame);
DefineStructS(AVPacket);
DefineStructS(SwsContext);

class FfmpegCodec
{
  FrameS          mCodedFrame;
  QList<AVFrameS> mDecodedFrames;
  QList<qint64>   mDecodedTimestamps;
  bool            mPackSynced;

  AVCodec*        mCodecDec;
  AVCodec*        mCodecEnc;
  AVCodecContextS mContextDec;
  AVCodecContextS mContextEnc;
  AVFrameS        mFrame;
  AVPacketS       mPacketDec;
  AVPacketS       mPacketEnc;
  QByteArray      mBufferMid;

public:
  FrameS& CodedFrame() { return mCodedFrame; }

  bool InitCodec(AVCodecID _SrcCodecId, AVCodecID _DestCodecId);

public:
  bool Code(char* frameData, int frameSize, int width, int height);
  bool DecodeToPack(char* frameData, int frameSize, qint64 timestamp, int width, int height);
  bool EncodeFromPack(FrameS& frame);
  bool CodeAudio(char* frameData, int frameSize);

private:
  bool InitFrame();
  bool InitDecoder(AVCodecID _CodecId);
  bool InitEncoder(AVCodecID _CodecId);
  bool InitEncodeContext(AVFrameS& avframe);
  bool EncodeFrame(AVFrameS& avframe);
  bool CreateFrame();
  void DebugOutFrame(const QString& prefix, int counter, const AVFrameS& frame);

public:
  FfmpegCodec();
  ~FfmpegCodec();
};

