#pragma once

#include <vector>
#include <QSize>

#include <LibV/Include/Frame.h>
#include "Def.h"
#include "../Source.h"


DefineClassS(EventLoop);

class MediaSinkImpl: public MediaSink
{
  EventLoop*   mSource;
  int          mMaxFrameSize;
  int          mMaxHeaderSize;
  ECompression mCompression;
  FrameS       mNextFrame;
  const char*  mSprop;
  QSize        mCurrentFrameSize;
  QSize        mFrameSize;

  bool         mHasKey;
  QByteArray   mExHeader;
  bool         mFrameSPS;
  bool         mFramePPS;

  QByteArray   mFrameBuffer;
  qint64       mBaseTimestamp;

  friend void CallbackGetNextFrame(void* clientData, unsigned frameSize, unsigned truncatedBytesCount, struct timeval presentationTime, unsigned durationInMicroseconds);

  struct H264Data
  {
    bool key;
    bool data;

    struct PS
    {
      bool is;
      int  pos;
      int  len;
    };

    PS sps;
    PS pps;
  };

protected:
  /*override */virtual Boolean continuePlaying() Q_DECL_OVERRIDE;

private:
  void OnFrame(int frameSize, const qint64& ts);
  bool ModifyH264Frame(char*& frameData, int& frameSize, bool& key);
  bool ModifyAacFrame(char*& frameData, int& frameSize);
  void ParseH264Frame(char* frame, int len, H264Data& pd);

public:
  MediaSinkImpl(EventLoop* _Source, UsageEnvironment& env, int _MaxFrameSize, ECompression _Compression, const char* _Sprop = nullptr);
  /*override */virtual ~MediaSinkImpl();
};

