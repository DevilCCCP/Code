#include <qsystemdetection.h>
#ifdef __cplusplus
extern "C" {
#endif
#ifdef __GNUC__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#include <libavutil/imgutils.h>
#ifdef __cplusplus
}
#endif

#include <Lib/Log/Log.h>

#include "Convert.h"


bool Convert::ConvertFrame(ECompression compression, const FrameS& srcFrame, FrameS& dstFrame)
{
  const Frame::Header* header = srcFrame->GetHeader();
  switch (header->Compression) {
  case eRawRgb:   mPixFormat = AV_PIX_FMT_RGB24; break;
  case eRawRgba:  mPixFormat = AV_PIX_FMT_RGB32; break;
  case eRawNv12:
  case eRawNv12A: mPixFormat = AV_PIX_FMT_NV12; break;
  case eRawY: mPixFormat = AV_PIX_FMT_GRAY8; break;
  default: return false;
  }
  mWidth = header->Width;
  mHeight = header->Height;

  switch (compression) {
  case eRawY:
    return ConvertToY(srcFrame, dstFrame);
  case eRawNv12:
    return ConvertToNv12(srcFrame, dstFrame);
  default:
    return false;
  }
}

bool Convert::ConvertToY(const FrameS& srcFrame, FrameS& frame)
{
#ifndef SWS_SCALE_BUG
  mSwsContext = sws_getCachedContext(mSwsContext, mWidth, mHeight, mPixFormat
                                     , mWidth, mHeight, AV_PIX_FMT_GRAY8, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);
#else
  mSwsContext = sws_getCachedContext(mSwsContext, mWidth, mHeight, mPixFormat
                                     , mWidth-1, mHeight, AV_PIX_FMT_GRAY8, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);
#endif

  int dstStrides[8] = { 0 };
  av_image_fill_linesizes(dstStrides, AV_PIX_FMT_GRAY8, mWidth);

  int decodedSize = avpicture_get_size(AV_PIX_FMT_GRAY8, mWidth, mHeight);
  Frame::Header* header = InitFrame(frame, decodedSize, true);

  uint8_t* dstSlices[8] = { (uint8_t*)frame->VideoData() };

  int srcStrides[8] = { 0 };
  av_image_fill_linesizes(srcStrides, mPixFormat, mWidth);
  uint8_t* srcSlices[8] = { (uint8_t*)srcFrame->VideoData() };
  int disp = srcStrides[0];
  for (int i = 1; i < 8 && srcStrides[i]; i++) {
    srcSlices[i] = (uint8_t*)srcFrame->VideoData() + srcStrides[i];
    disp += srcStrides[i];
  }
  int height = sws_scale(mSwsContext, srcSlices, srcStrides, 0, mHeight, dstSlices, dstStrides);
  if (height != mHeight) {
    Log.Warning(QString("ffmpeg: reformat Y fail (src: %1x%2(%3), dst height: %4").arg(mWidth).arg(mHeight)
                .arg(mPixFormat).arg(mHeight));
    return false;
  }
  header->Compression = eRawY;
  header->Width = mWidth;
  header->Height = mHeight;
  header->VideoDataSize = decodedSize;
  return true;
}

bool Convert::ConvertToNv12(const FrameS& srcFrame, FrameS& frame)
{
#ifndef SWS_SCALE_BUG
  mSwsContext = sws_getCachedContext(mSwsContext, mWidth, mHeight, mPixFormat
                                     , mWidth, mHeight, AV_PIX_FMT_NV12, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);
#else
  mSwsContext = sws_getCachedContext(mSwsContext, mWidth, mHeight, mPixFormat
                                     , mWidth-1, mHeight, AV_PIX_FMT_NV12, SWS_GAUSS
                                     , nullptr, nullptr, nullptr);
#endif

  int dstStrides[8] = { 0 };
  av_image_fill_linesizes(dstStrides, AV_PIX_FMT_NV12, mWidth);

  int stride = dstStrides[0];
  int decodedSize = avpicture_get_size(AV_PIX_FMT_NV12, mWidth, mHeight);
  Frame::Header* header = InitFrame(frame, decodedSize, true);

  uint8_t* dstSlices[8] = { (uint8_t*)frame->VideoData(), (uint8_t*)frame->VideoData() + stride * mHeight };

  int srcStrides[8] = { 0 };
  av_image_fill_linesizes(srcStrides, mPixFormat, mWidth);
  uint8_t* srcSlices[8] = { (uint8_t*)srcFrame->VideoData() };
  int disp = srcStrides[0];
  for (int i = 1; i < 8 && srcStrides[i]; i++) {
    srcSlices[i] = (uint8_t*)srcFrame->VideoData() + srcStrides[i];
    disp += srcStrides[i];
  }
  int height = sws_scale(mSwsContext, srcSlices, srcStrides, 0, mHeight, dstSlices, dstStrides);
  if (height != mHeight) {
    Log.Warning(QString("ffmpeg: reformat NV12 fail (src: %1x%2(%3), dst height: %4").arg(mWidth).arg(mHeight)
                .arg(mPixFormat).arg(mHeight));
    return false;
  }
  header->Compression = eRawNv12;
  header->Width = mWidth;
  header->Height = mHeight;
  header->VideoDataSize = decodedSize;
  return true;
}

Frame::Header*Convert::InitFrame(FrameS& frame, int fullSize, int key)
{
  frame = FrameS(new Frame());
  frame->ReserveData(fullSize);
  Frame::Header* header = frame->InitHeader();
  header->Key = key;
  header->Size = sizeof(Frame::Header) + fullSize;
  return header;
}


Convert::Convert()
  : mWidth(0), mHeight(0), mSwsContext(nullptr)
{
}
