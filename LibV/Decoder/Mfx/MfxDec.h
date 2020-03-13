#pragma once

#include <QVector>
#include <QMap>
#include <QSet>
#include <QByteArray>

#include <LibV/Include/Frame.h>

#include "MfxDef.h"


DefineClassS(MfxDec);
DefineClassS(MfxContainer);
DefineClassS(FrameM);
DefineClassS(Thumbnail);

class MfxDec
{
  ThumbnailS                  mThumbnail;
  QList<MfxContainerS>        mMfxContainers;
  int                         mContainerPoolLimit;
  QList<FrameS>               mDecodedFrames;
  QMap<MfxFrameSurface*, int> mMfxSurfacesIndexMap;

  MfxVideoSessionS            mMfxVideoSession;
  MfxVideoDecodeS             mMfxVideoDecode;
  MfxFrameInfo                mMfxFrameInfo;
  MfxBitstream                mMfxBitstream;
  QByteArray                  mInnerDataBuffer;
  MfxSyncPoint                mMfxSyncPoint;

  int                         mWidth;
  int                         mHeight;
  int                         mStride;
  int                         mVertStride;
  int                         mSurfaceSize;

  int                         mLastRetCode;
  int                         mLastFailCode;
  QSet<int>                   mWarnCodes;

public:
  bool InitDecoder(int _CodecId, const char* frameData, int frameSize);
  bool QueryHardware();
  bool IsHardware();

  bool Decode(char* frameData, int frameSize, bool canSkip);
private:
  bool FeedNextData(char* frameData, int frameSize);
  bool DecodeAsync();
  void SaveTrailData();

public:
  bool TakeNextSurface(MfxContainer** nextContainer);
  bool TakeDecodedFrame(FrameS& frame);
  void DeinitDecoder();

private:
  void CreateNextContainer();
  bool Fail(const QString& operation);

public:
  MfxDec(const ThumbnailS& _Thumbnail);
  ~MfxDec();
};

