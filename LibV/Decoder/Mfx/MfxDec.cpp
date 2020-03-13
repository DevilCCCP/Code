#include <QImage>
#include <QImageWriter>
#include <QBuffer>
#include <mfxvideo++.h>

#include <Lib/Log/Log.h>
#include <Lib/Common/Format.h>

#include "MfxDec.h"
#include "MfxContainer.h"
#include "../Thumbnail.h"


const int kContainerPoolLimit = 32;

bool MfxDec::InitDecoder(int _CodecId, const char* frameData, int frameSize)
{
  DeinitDecoder();

  mfxVersion ver;
  ver.Major = 1;
  ver.Minor = 0;
  mMfxVideoSession.reset(new MfxVideoSession());
  mLastRetCode = mMfxVideoSession->Init(MFX_IMPL_AUTO, &ver);
  if (mLastRetCode < 0) {
    return Fail("Init decoder");
  }

  mMfxVideoDecode.reset(new MfxVideoDecode(*mMfxVideoSession));

  memset(&mMfxBitstream, 0, sizeof(mMfxBitstream));
  mMfxBitstream.Data = (uchar*)(const uchar*)frameData;
  mMfxBitstream.MaxLength = mMfxBitstream.DataLength = frameSize;

  mfxVideoParam mfxVideoParams;
  memset(&mfxVideoParams, 0, sizeof(mfxVideoParams));
  mfxVideoParams.mfx.CodecId = _CodecId;
  mfxVideoParams.IOPattern = MFX_IOPATTERN_OUT_SYSTEM_MEMORY;

  mLastRetCode = mMfxVideoDecode->DecodeHeader(&mMfxBitstream, &mfxVideoParams);
  if (mLastRetCode < 0) {
    return Fail("Read frame header");
  }
  memcpy(&mMfxFrameInfo, &mfxVideoParams.mfx.FrameInfo, sizeof(MfxFrameInfo));

  mfxFrameAllocRequest frameRequest;
  memset(&frameRequest, 0, sizeof(frameRequest));
  int numSurfaces = 8;
  mLastRetCode = mMfxVideoDecode->QueryIOSurf(&mfxVideoParams, &frameRequest);
  if (mLastRetCode == 0) {
    numSurfaces = frameRequest.NumFrameSuggested + 4;
  } else {
    Log.Warning(QString("Mfx retrive surface count fail (code: '%1')").arg(MfxErrorToString(mLastRetCode)));
  }

  mWidth = (int)(frameRequest.Info.Width);
  mHeight = (int)(frameRequest.Info.Height);
  mStride = (mWidth + 0x1f) & (~0x1f);
  mVertStride = (mHeight + 0x1f) & (~0x1f);
  mSurfaceSize = mStride * mVertStride * 3 / 2;

  for (int i = 0; i < numSurfaces; i++) {
    CreateNextContainer();
  }

  mLastRetCode = mMfxVideoDecode->Init(&mfxVideoParams);
  if (mLastRetCode < 0) {
    return Fail("Init video decoder");
  }

  Log.Info(QString("Init MFX decoder done (containers: %1, mem: %2)").arg(numSurfaces).arg(FormatBytes(numSurfaces * mSurfaceSize)));
  mMfxBitstream.DataLength = 0;
  return true;
}

bool MfxDec::QueryHardware()
{
  mfxVersion ver;
  ver.Major = 1;
  ver.Minor = 0;
  mMfxVideoSession.reset(new MfxVideoSession());
  mLastRetCode = mMfxVideoSession->Init(MFX_IMPL_AUTO, &ver);
  if (mLastRetCode < 0) {
    return Fail("Init decoder");
  }

  return IsHardware();
}

bool MfxDec::IsHardware()
{
  mfxIMPL impl;
  mLastRetCode = mMfxVideoSession->QueryIMPL(&impl);
  if (mLastRetCode != 0) {
    return Fail("Query hardware");
  }
  if ((impl & MFX_IMPL_HARDWARE) == MFX_IMPL_HARDWARE) {
    Log.Info(QString("MFX implementation is hardware (flag: %1)").arg((int)impl, 0, 16));
    return true;
  } else if ((impl & MFX_IMPL_SOFTWARE) == MFX_IMPL_SOFTWARE) {
    Log.Info(QString("MFX implementation is software (flag: %1)").arg((int)impl, 0, 16));
    return false;
  }
  Log.Warning(QString("MFX implementation is 0x%1").arg(impl, 0, 16));
  return false;
}

bool MfxDec::Decode(char *frameData, int frameSize, bool canSkip)
{
  Q_UNUSED(canSkip);

  if (!FeedNextData(frameData, frameSize)) {
    return false;
  }

  bool result = DecodeAsync();
  SaveTrailData();
  return result;
}

bool MfxDec::FeedNextData(char* frameData, int frameSize)
{
  if (mMfxBitstream.DataLength) {
    mInnerDataBuffer.resize(mMfxBitstream.DataLength + frameSize);
    memmove(mInnerDataBuffer.data() + mMfxBitstream.DataLength, frameData, frameSize);
    frameData = mInnerDataBuffer.data();
    frameSize = mInnerDataBuffer.size();
  }
  mMfxBitstream.DataOffset = 0;
  mMfxBitstream.Data = (uchar*)frameData;
  mMfxBitstream.DataLength = mMfxBitstream.MaxLength = frameSize;
  return true;
}

bool MfxDec::DecodeAsync()
{
  forever {
    MfxContainer* nextContainer;
    while (!TakeNextSurface(&nextContainer)) {
      return false;
    }
    MfxFrameSurface* outSurface = nullptr;
    mLastRetCode = mMfxVideoDecode->DecodeFrameAsync(&mMfxBitstream, nextContainer->Surface(), &outSurface, &mMfxSyncPoint);
    if (mLastRetCode > 0) {
      if (!mWarnCodes.contains(mLastRetCode)) {
        Log.Warning(QString("Mfx decode warning (code: %1)").arg(MfxErrorToString(mLastRetCode)));
        mWarnCodes.insert(mLastRetCode);
      }
    }
    if (mMfxSyncPoint && mLastRetCode >= 0) {
      mLastRetCode = mMfxVideoSession->SyncOperation(mMfxSyncPoint, 1000);
      if (mLastRetCode == 0) {
        auto itr = mMfxSurfacesIndexMap.find(outSurface);
        if (itr != mMfxSurfacesIndexMap.end()) {
          int index = itr.value();
          MfxContainer* outContainer = mMfxContainers[index].data();
          mDecodedFrames.append(outContainer->CreateOutputFrame());
//          static int gFrame = 0;
//          Log.Trace(QString("Frame: %3, Index: %1 (ptr: %2)").arg(index).arg((int)outContainer->FrameHeader(), 0, 16).arg(++gFrame));
        } else {
          Log.Error(QString("Mfx decode fail (output surface not found)"));
        }
        continue;
      }
    }

    switch (mLastRetCode) {
    case MFX_ERR_NONE:
    case MFX_ERR_MORE_DATA:
      return true;
    case MFX_ERR_MORE_SURFACE:
//      Log.Trace("MORE_SURFACE");
      break;
    default:
      if (mLastRetCode < 0) {
        return Fail("Decode");
      }
      break;
    }
  }
}

void MfxDec::SaveTrailData()
{
  if (mMfxBitstream.DataLength > 0) {
    mInnerDataBuffer.resize(mMfxBitstream.DataLength);
    memmove(mInnerDataBuffer.data(), mMfxBitstream.Data + mMfxBitstream.DataOffset, mMfxBitstream.DataLength);
    mMfxBitstream.Data = (uchar*)mInnerDataBuffer.data();
  }
}

bool MfxDec::TakeNextSurface(MfxContainer** nextContainer)
{
  for (int i = 0; i < mMfxContainers.size(); i++) {
    if (mMfxContainers[i]->IsReady()) {
      *nextContainer = mMfxContainers[i].data();
      return true;
    }
  }

  if (mMfxContainers.size() < mContainerPoolLimit) {
    CreateNextContainer();
    *nextContainer = mMfxContainers.last().data();
    Log.Info(QString("Mfx add extra container (cnt: %1, mem: %2)").arg(mMfxContainers.size()).arg(FormatBytes(mMfxContainers.size() * mSurfaceSize)));
    return true;
  }
  return false;
}

bool MfxDec::TakeDecodedFrame(FrameS& frame)
{
  if (!mDecodedFrames.isEmpty()) {
    frame = mDecodedFrames.takeFirst();
    if (mThumbnail && mThumbnail->IsTimeToCreate()) {
      mThumbnail->Create(frame);
    }
    return true;
  }
  return false;
}

void MfxDec::DeinitDecoder()
{
  if (mMfxVideoDecode) {
    mMfxVideoDecode->Close();
    mMfxVideoDecode.reset();
  }
  mMfxVideoSession.reset();

  mMfxContainers.clear();
}

void MfxDec::CreateNextContainer()
{
  MfxContainer* nextContainer;
  mMfxContainers.append(MfxContainerS(nextContainer = new MfxContainer(mSurfaceSize)));
  Frame::Header* header = nextContainer->FrameHeader();
  header->HeaderSize = sizeof(Frame::Header);
  header->Compression = eRawNv12A;
  header->Width = mWidth;
  header->Height = mHeight;
  header->VideoDataSize = mSurfaceSize;
  header->Size = header->VideoDataSize;

  MfxFrameSurface* lastSurf = nextContainer->Surface();
  memcpy(&lastSurf->Info, &mMfxFrameInfo, sizeof(MfxFrameInfo));
  lastSurf->Data.Y = (uchar*)nextContainer->FrameData();
  lastSurf->Data.U = (uchar*)nextContainer->FrameData() + mStride * mVertStride;
  lastSurf->Data.V = (uchar*)nextContainer->FrameData() + mStride * mVertStride + 1;
  lastSurf->Data.Pitch = mStride;
  mMfxSurfacesIndexMap[lastSurf] = mMfxContainers.size() - 1;
}

bool MfxDec::Fail(const QString& operation)
{
  if (mLastFailCode != mLastRetCode) {
    mLastFailCode = mLastRetCode;
    Log.Error(QString("Mfx %1 fail (code: '%2')").arg(operation, MfxErrorToString(mLastRetCode)));
  }
  return false;
}


MfxDec::MfxDec(const ThumbnailS& _Thumbnail)
  : mThumbnail(_Thumbnail), mContainerPoolLimit(kContainerPoolLimit)
  , mWidth(0), mHeight(0), mStride(0), mSurfaceSize(0)
  , mLastFailCode(0)
{
}

MfxDec::~MfxDec()
{
}
