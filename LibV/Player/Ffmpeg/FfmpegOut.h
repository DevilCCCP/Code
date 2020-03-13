#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <libavformat/avformat.h>

#ifdef __cplusplus
}
#endif

#include <LibV/Include/Frame.h>

DefineStructS(AVFormatContext);
DefineStructS(AVPacket);
DefineStructS(AVRational);
DefineClassS(FfmpegOut);

class FfmpegOut
{
  QString           mFilename;
  bool              mInit;
  AVFormatContextS  mFileContext;
  AVStream*         mVideoStream;
  AVStream*         mAudioStream;

  ECompression      mCompression;
  ECompression      mCompressionA;
  AVRational        mTimeBase;
  AVRational        mTimeBaseA;
  qint64            mTimestamp;
  qint64            mTimestampA;
  int               mWidth;
  int               mHeight;
  AVPacketS         mPacket;
  QList<FrameS>     mFrameBuffer;
  QByteArray        mExtraDataA;

  int               mErrVideo;
  int               mErrAudio;

public:
  bool Open(const QString& filename);
  bool WriteNext(FrameS& frame);
  bool Close();

private:
  bool PrepareFile();
  bool CloseFile();
  bool FinalFile();
  bool WriteFile(FrameS& frame);
  bool FlushFrames();
  int CalcBestFps();
  void DumpFrame(FrameS& frame);

public:
  FfmpegOut();
  ~FfmpegOut();
};

