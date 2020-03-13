#pragma once

#include <QByteArray>

#include <LibV/Include/Frame.h>


DefineClassS(OmxDec);
DefineClassS(Thumbnail);
DefineClassS(IlComponents);
DefineStructS(_ILCLIENT_T);
DefineStructS(_COMPONENT_T);

class OmxDec
{
  _ILCLIENT_TS    mIlClient;
  IlComponentsS   mIlDecoder;
  bool            mInit;
  bool            mFirstFrame;
  bool            mOutReady;
  ECompression    mOutCompression;
  int             mWidth;
  int             mHeight;
  int             mStride;
  int             mSliceHeight;
  int             mFrameSize;

  ThumbnailS      mThumbnail;

public:
  bool InitDecoder();
  bool CloseDecoder();
  void DeinitDecoder();

public:
  bool DecodeIn(char* frameData, int frameSize);
  bool DecodeOut(bool canSkip, FrameS& decodedFrame);

private:
  bool GetOutputFormat();

public:
  OmxDec(const ThumbnailS& _Thumbnail);
  ~OmxDec();
};

