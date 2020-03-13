#include <QImage>
#include <QImageWriter>
#include <QBuffer>
#include <QX11Info>
#include <vdpau/vdpau.h>
//#include <vdpau/vdpau_x11.h>

#include <Lib/Log/Log.h>
#include <LibV/MediaServer/H264/H264NalUnit.h>
#include <LibV/MediaServer/H264/H264Sprop.h>
#include <LibV/MediaServer/H264/H264Sps.h>
#include <LibV/MediaServer/H264/H264Pps.h>
#include <LibV/MediaServer/H264/H264Slice.h>

#include "VdpDec.h"
#include "VdpContext.h"
#include "../Thumbnail.h"


#define LogLive(X)
//#define LogLive Log.Info

bool VdpDec::InitDecoder()
{
  DeinitDecoder();

  mVdpContext.reset(new VdpContext(QX11Info::display(), QX11Info::appScreen()));
  if (!mVdpContext->Init()) {
    if (!mHasError) {
      Log.Warning("Init vdpau context fail");
      mHasError = true;
    }
    return false;
  }

  Log.Info("Init vdpau context done");
  return true;
}

bool VdpDec::InitSurface()
{
  VdpVideoSurface surface;
  VdpStatus status = mVdpContext->funcVdpVideoSurfaceCreate(mVdpContext->Device(), VDP_CHROMA_TYPE_420, mWidth, mHeight, &surface);
  if (status != VDP_STATUS_OK) {
    Log.Error("Vdpau create surface fail");
    return false;
  }
  mSurfaces.append(surface);
  mFreeSurfaces.append(surface);
  return true;
}

void VdpDec::DeinitDecoder()
{
  DeinitVdpDecoder();

  mVdpContext.reset();
}

bool VdpDec::DecodeIn(char* frameData, int frameSize, const qint64& timestamp)
{
  H264NalUnit nalu(frameData, frameSize);
  mH264Sprop->ClearSlice();

  const char* dataStart  = 0;
  const char* dataFinish = 0;
  int  sliceCount  = 0;
  bool isReference = false;
  bool isIdr       = false;
  while (nalu.FindNext()) {
    if (mH264Sprop->Parse(nalu.CurrentUnit(), nalu.CurrentUnitSize(), true) && (mWidth != mH264Sprop->Width() || mHeight != mH264Sprop->Height())) {
      if (mH264Sprop->HasSps()) {
        *mH264Sps = mH264Sprop->GetSps();
      }
      if (mH264Sprop->HasPps()) {
        *mH264Pps = mH264Sprop->GetPps();
      }
    } else if (mH264Sprop->HasSlice()) {
      if (!dataStart) {
        dataStart  = nalu.CurrentMarkedUnit();
      }
      const H264Slice& nextSlice = mH264Sprop->GetSlice();
      if (mH264Sprop->GetNaluType() == H264Sprop::eSliceIdr) {
        isIdr = true;
      }
      isReference = mH264Sprop->IsRefference();
      if (nextSlice.first_mb_in_slice == 0) {
        if (dataFinish) {
          break;
        }
        *mH264Slice = nextSlice;
      }
      dataFinish = nalu.CurrentUnit() + nalu.CurrentUnitSize();
      sliceCount++;
    }
//    Log.Trace(QString("pos: %1, size: %2, type: %3").arg(nalu.CurrentUnitPos()).arg(nalu.CurrentUnitSize()).arg(mH264Sprop->NaluTypeToString()));
  }

  if (!mH264Sprop->HasSps() || !mH264Sprop->HasPps()) {
    Log.Warning(QString("Decode H264 fail, no sps and pps"));
    return false;
  }
  if (!mH264Sprop->HasSlice()) {
    Log.Warning(QString("Decode H264 frame fail, no slice"));
    return false;
  }

  if (!InitVdpDecoder()) {
    return false;
  }

  VdpBitstreamBuffer vbit;
  vbit.struct_version  = VDP_BITSTREAM_BUFFER_VERSION;
  vbit.bitstream       = dataStart;
  vbit.bitstream_bytes = dataFinish - dataStart;

  VdpPictureInfoH264 info;
  memset(&info, 0, sizeof(info));
  info.slice_count = sliceCount;                                      // * may be zero (always zero in ffmpeg)
  info.field_order_cnt[0] = 0x010000 + mH264Slice->pic_order_cnt_lsb; // * POC type 0 only
  info.field_order_cnt[1] = 0x010000 + mH264Slice->pic_order_cnt_lsb; // * POC type 0 only
  info.is_reference = isReference;
  info.frame_num = mH264Slice->frame_num;                             // good, except gaps_in_frame_num_value_allowed case
  info.field_pic_flag = 0;                                            // * for interlaced video
  info.bottom_field_flag = 0;                                         // * if top or bottom part of interlaced video frame
  info.num_ref_frames                         = mH264Sps->max_num_ref_frames;
  info.mb_adaptive_frame_field_flag           = mH264Sps->mb_adaptive_frame_field_flag;
  info.constrained_intra_pred_flag            = mH264Pps->constrained_intra_pred_flag;
  info.weighted_pred_flag                     = mH264Pps->weighted_pred_flag;
  info.weighted_bipred_idc                    = mH264Pps->weighted_bipred_idc;
  info.frame_mbs_only_flag                    = mH264Sps->frame_mbs_only_flag;
  info.transform_8x8_mode_flag                = mH264Pps->transform_8x8_mode_flag;
  info.chroma_qp_index_offset                 = mH264Pps->chroma_qp_index_offset;
  info.second_chroma_qp_index_offset          = mH264Pps->second_chroma_qp_index_offset;
  info.pic_init_qp_minus26                    = mH264Pps->pic_init_qp_minus26;
  info.num_ref_idx_l0_active_minus1           = mH264Pps->num_ref_idx_l0_default_active_minus1;
  info.num_ref_idx_l1_active_minus1           = mH264Pps->num_ref_idx_l1_default_active_minus1;
  info.log2_max_frame_num_minus4              = mH264Sps->log2_max_frame_num_minus4;
  info.pic_order_cnt_type                     = mH264Sps->pic_order_cnt_type;
  info.log2_max_pic_order_cnt_lsb_minus4      = mH264Sps->log2_max_pic_order_cnt_lsb_minus4;
  info.delta_pic_order_always_zero_flag       = mH264Sps->delta_pic_order_always_zero_flag;
  info.direct_8x8_inference_flag              = mH264Sps->direct_8x8_inference_flag;

  info.entropy_coding_mode_flag               = mH264Pps->entropy_coding_mode_flag;
  info.pic_order_present_flag                 = mH264Pps->pic_order_present_flag;
  info.deblocking_filter_control_present_flag = mH264Pps->deblocking_filter_control_present_flag;
  info.redundant_pic_cnt_present_flag         = mH264Pps->redundant_pic_cnt_present_flag;

  memcpy (&info.scaling_lists_4x4, &mH264Sps->scaling_lists_4x4, 96);
  memcpy (&info.scaling_lists_8x8, &mH264Sps->scaling_lists_8x8, 128);

  if (isIdr) {
    mFreeSurfaces = mSurfaces.toList();
    mReferenceMap.clear();
  }
  if (mFreeSurfaces.isEmpty()) {
    if (!InitSurface()) {
      return false;
    }
  }
  VdpVideoSurface surface = mFreeSurfaces.first();
  for (int i = 0; i < mReferenceMap.size(); i++) {
    const ReferenceInfo& refInfo = mReferenceMap.at(mReferenceMap.size()-1 - i);
    info.referenceFrames[i].frame_idx           = refInfo.FrameIndex;
    info.referenceFrames[i].surface             = refInfo.Surface;
    info.referenceFrames[i].top_is_reference    = 1;
    info.referenceFrames[i].bottom_is_reference = 1;
    info.referenceFrames[i].field_order_cnt[0]  = refInfo.FieldOrder0;
    info.referenceFrames[i].field_order_cnt[1]  = refInfo.FieldOrder1;
  }
  for (int i = mReferenceMap.size(); i < 16; i++) {
    info.referenceFrames[i].surface = VDP_INVALID_HANDLE;
  }

  if (isReference) {
    mFreeSurfaces.removeFirst();
    ReferenceInfo refInfo;
    refInfo.FrameIndex   = info.frame_num;
    refInfo.Surface      = surface;
    refInfo.FieldOrder0  = info.field_order_cnt[0];
    refInfo.FieldOrder1  = info.field_order_cnt[1];
    mReferenceMap.append(refInfo);
    if (mReferenceMap.size() > mH264Sps->max_num_ref_frames) {
      const ReferenceInfo& refInfo = mReferenceMap.first();
      mFreeSurfaces.append(refInfo.Surface);
      mReferenceMap.removeFirst();
    }
  }

  VdpStatus status = mVdpContext->funcVdpDecoderRender(mVdpDecoder, surface, &info, 1, &vbit);
  if (status != VDP_STATUS_OK) {
    return false;
  }

//  int maxDrawNumber = 1 << (mH264Sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
  int drawNumber = mH264Slice->pic_order_cnt_lsb/2;
  if (isIdr) {
    mDecoded.clear();
    mOutOrder = 0;
  }
  FrameS frame;
  if (!FrameFromSurface(surface, frame)) {
    frame.reset(new Frame());
    frame->GetHeader()->Timestamp = timestamp;
    mDecoded[drawNumber] = frame;
    return false;
  }
  frame->GetHeader()->Timestamp = timestamp;
  mDecoded[drawNumber] = frame;
  return true;
}

bool VdpDec::DecodeOut(bool canSkip, FrameS& decodedFrame)
{
  Q_UNUSED(canSkip);

  auto itr = mDecoded.find(mOutOrder);
  if (itr == mDecoded.end()) {
    return false;
  }

  decodedFrame = itr.value();
  mDecoded.erase(itr);
  return true;
}

bool VdpDec::InitVdpDecoder()
{
  if (mVdpDecoder >= 0) {
    return true;
  }

  mOutCompression = eRawNv12;
  mWidth  = mH264Sprop->Width();
  mHeight = mH264Sprop->Height();

  VdpStatus status = mVdpContext->funcVdpDecoderCreate(mVdpContext->Device(), VDP_DECODER_PROFILE_H264_MAIN, mWidth, mHeight, 16, (VdpDecoder*)&mVdpDecoder);
  if (status != VDP_STATUS_OK) {
    if (!mHasError) {
      Log.Error("Create vdpau decoder fail");
      mHasError = true;
    }
    return false;
  }

  mHasError = false;
  return true;
}

void VdpDec::DeinitVdpDecoder()
{
  mOutOrder = 0;
  mReferenceMap.clear();
  mFreeSurfaces.clear();
  foreach (int surface, mSurfaces) {
    mVdpContext->funcVdpVideoSurfaceDestroy(surface);
  }
  mSurfaces.clear();

  if (mVdpDecoder >= 0) {
    mVdpContext->funcVdpDecoderDestroy(mVdpDecoder);
    mVdpDecoder = -1;
  }
}

bool VdpDec::FrameFromSurface(int surface, FrameS& decodedFrame)
{
  int frameSize = mWidth*mHeight*3/2;
  decodedFrame.reset(new Frame());
  decodedFrame->ReserveData(frameSize);
  Frame::Header* header = decodedFrame->InitHeader();
  header->Compression   = mOutCompression;
  header->Width         = mWidth;
  header->Height        = mHeight;
  header->VideoDataSize = frameSize;

  const int planes = 2;
  int pitch[planes];
  pitch[0] = mWidth;
  pitch[1] = mWidth;
  char* data[planes];
  data[0] = decodedFrame->VideoData();
  data[1] = decodedFrame->VideoData() + mWidth*mHeight;
//  memset(data[0], 0, height * pitch[0]);
//  memset(data[1], 0, height * pitch[1]);
  VdpStatus status = mVdpContext->funcVdpVideoSurfaceGetBitsYCbCr(surface, VDP_YCBCR_FORMAT_NV12, (void**)data, (uint32_t*)pitch);
  if (status != VDP_STATUS_OK) {
    LOG_ERROR_ONCE("funcVdpVideoSurfaceGetBitsYCbCr fail");
    return false;
  }

  return true;
}


VdpDec::VdpDec(const ThumbnailS& _Thumbnail)
  : mVdpDecoder(-1), mH264Sprop(new H264Sprop()), mH264Sps(new H264Sps()), mH264Pps(new H264Pps()), mH264Slice(new H264Slice())
  , mWidth(0), mHeight(0)
  , mThumbnail(_Thumbnail), mHasError(false)
{
}

VdpDec::~VdpDec()
{
  DeinitDecoder();
}

