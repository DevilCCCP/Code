#pragma once

#include <QVector>
#include <QList>
#include <QMap>
#include <QByteArray>

#include <LibV/Include/Frame.h>


DefineClassS(VdpDec);
DefineClassS(Thumbnail);
DefineClassS(VdpContext);
DefineClassS(H264Sprop);
DefineClassS(H264Sps);
DefineClassS(H264Pps);
DefineClassS(H264Slice);

class VdpDec
{
  VdpContextS          mVdpContext;
  int                  mVdpDecoder;
  QVector<int>         mSurfaces;
  QList<int>           mFreeSurfaces;
  H264SpropS           mH264Sprop;
  H264SpsS             mH264Sps;
  H264PpsS             mH264Pps;
  H264SliceS           mH264Slice;

  struct ReferenceInfo {
    int FrameIndex;
    int Surface;
    int FieldOrder0;
    int FieldOrder1;
  };
  QList<ReferenceInfo> mReferenceMap;
  QMap<int, FrameS>    mDecoded;
  int                  mOutOrder;

  ECompression         mOutCompression;
  int                  mWidth;
  int                  mHeight;

  ThumbnailS           mThumbnail;
  bool                 mHasError;

public:
  bool InitDecoder();
  bool InitSurface();
  void DeinitDecoder();

public:
  bool DecodeIn(char* frameData, int frameSize, const qint64& timestamp);
  bool DecodeOut(bool canSkip, FrameS& decodedFrame);

private:
  bool InitVdpDecoder();
  void DeinitVdpDecoder();
  bool GetOutputFormat();

  bool FrameFromSurface(int surface, FrameS& frame);

public:
  VdpDec(const ThumbnailS& _Thumbnail);
  ~VdpDec();
};

