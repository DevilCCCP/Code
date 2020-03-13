#ifdef H264PARSE_VERBOSE
#include <stdio.h>
#endif

#include <Lib/Include/BitstreamReader.h>
#include <Lib/Include/BitstreamWriter.h>

#include "H264Sprop.h"
#include "H264Sps.h"
#include "H264Pps.h"
#include "H264Sei.h"
#include "H264Slice.h"


#ifdef H264PARSE_VERBOSE
#define PRINT(name) printf("%s: %d\n", #name, name)
#define READ(var, value) if (mReader->EndOfData()) return false; var = (value); PRINT(var)
#else
#define READ(var, value) if (mReader->EndOfData()) return false; var = (value)
#endif
#define WRITE(var, value) var = (value)


int H264Sprop::Width() const
{
  if (HasSps()) {
    return (mSps->pic_width_in_mbs_minus1 + 1) * 16;
  }
  return 0;
}

int H264Sprop::Height() const
{
  if (HasSps()) {
    return (mSps->pic_height_in_map_units_minus1 + 1) * 16;
  }
  return 0;
}

H264Sprop::SliceType H264Sprop::GetSliceType() const
{
  return (SliceType)mSlice->slice_type;
}

QString H264Sprop::NaluTypeToString() const
{
  switch ((NaluType)mNalUnit.nal_unit_type) {
  case eNaluNone:    return QStringLiteral("No Nalu");
  case eSliceNonIdr: return QStringLiteral("Slice non Idr");
  case eSlicePartA:  return QStringLiteral("Slice part A");
  case eSlicePartB:  return QStringLiteral("Slice part B");
  case eSlicePartC:  return QStringLiteral("Slice part C");
  case eSliceIdr:    return QStringLiteral("Slice Idr");
  case eSei:         return QStringLiteral("Sei");
  case eSps:         return QStringLiteral("Sps");
  case ePps:         return QStringLiteral("Pps");
  default:           return QString("Nalu %1").arg(mNalUnit.nal_unit_type);
  }
}

QString H264Sprop::SliceTypeToString() const
{
  SliceType sliceType = (SliceType)((mSlice->slice_type >= 5)? mSlice->slice_type - 5: mSlice->slice_type);
  if (mSlice->slice_type >= 5) {
    switch (sliceType) {
    case eSliceP:      return QStringLiteral("P(all)");
    case eSliceB:      return QStringLiteral("B(all)");
    case eSliceI:      return QStringLiteral("I(all)");
    case eSliceSp:     return QStringLiteral("SP(all)");
    case eSliceSi:     return QStringLiteral("SI(all)");
    default:           return QString("Slice %1(all)").arg(mSlice->slice_type);
    }
  } else {
    switch (sliceType) {
    case eSliceP:      return QStringLiteral("P");
    case eSliceB:      return QStringLiteral("B");
    case eSliceI:      return QStringLiteral("I");
    case eSliceSp:     return QStringLiteral("SP");
    case eSliceSi:     return QStringLiteral("SI");
    default:           return QString("Slice %1").arg(mSlice->slice_type);
    }
  }
}

int H264Sprop::Write(char* data, int size)
{
  mWriter.reset(new BitstreamWriter(data, size));

  mWriter->U(1, mNalUnit.forbidden_zero_bit);
  mWriter->U(2, mNalUnit.nal_ref_idc);
  mWriter->U(5, mNalUnit.nal_unit_type);

  if (mSps) {
    WriteSps();
    return WriteTrail();
  } else if (mPps) {
    WritePps();
    return WriteTrail();
  } else {
    return 0;
  }
}

void H264Sprop::WriteSps()
{
  mWriter->U(8, mSps->profile_idc);
  mWriter->U(1, mSps->constraint_set0_flag);
  mWriter->U(1, mSps->constraint_set1_flag);
  mWriter->U(1, mSps->constraint_set2_flag);
  mWriter->U(1, mSps->constraint_set3_flag);
  mWriter->U(1, mSps->constraint_set4_flag);
  mWriter->U(1, mSps->constraint_set5_flag);
  mWriter->U(2, mSps->reserved_zero_2bits);
  mWriter->U(8, mSps->level_idc);
  mWriter->Uev(mSps->seq_parameter_set_id);

  if ( mSps->profile_idc  == 100 || mSps->profile_idc  == 110
       || mSps->profile_idc  == 122 || mSps->profile_idc  == 244 || mSps->profile_idc  == 44
       || mSps->profile_idc  ==  83 || mSps->profile_idc  ==  86 || mSps->profile_idc  == 118
       || mSps->profile_idc  == 128 || mSps->profile_idc  == 138)
  {
    mWriter->Uev(mSps->chroma_format_idc);
    if (mSps->chroma_format_idc == 3) {
      mWriter->U(1, mSps->separate_colour_plane_flag);
    }

    mWriter->Uev(mSps->bit_depth_luma_minus8);
    mWriter->Uev(mSps->bit_depth_chroma_minus8);
    mWriter->U(1, mSps->qpprime_y_zero_transform_bypass_flag);
    mWriter->U(1, mSps->seq_scaling_matrix_present_flag);

    if (mSps->seq_scaling_matrix_present_flag) {
      for (int i = 0; i < ((mSps->chroma_format_idc != 3)? 8: 12); i++) {
        mWriter->U(1, 0);
      }
    }
  }

  mWriter->Uev(mSps->log2_max_frame_num_minus4);
  mWriter->Uev(mSps->pic_order_cnt_type);

  if (mSps->pic_order_cnt_type == 0) {
    mWriter->Uev(mSps->log2_max_pic_order_cnt_lsb_minus4);
  } else if (mSps->pic_order_cnt_type == 1) {
    mWriter->U(1, mSps->delta_pic_order_always_zero_flag);
    mWriter->Sev(mSps->offset_for_non_ref_pic);
    mWriter->Sev(mSps->offset_for_top_to_bottom_field);
    mWriter->Uev(mSps->num_ref_frames_in_pic_order_cnt_cycle);

    for (int i = 0; i < mSps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
      mWriter->Sev(0);
    }
  }

  mWriter->Uev(mSps->max_num_ref_frames);
  mWriter->U(1, mSps->gaps_in_frame_num_value_allowed_flag);
  mWriter->Uev(mSps->pic_width_in_mbs_minus1);
  mWriter->Uev(mSps->pic_height_in_map_units_minus1);
  mWriter->U(1, mSps->frame_mbs_only_flag);
  if (!mSps->frame_mbs_only_flag) {
    mWriter->U(1, mSps->mb_adaptive_frame_field_flag);
  }

  mWriter->U(1, mSps->direct_8x8_inference_flag);
  mWriter->U(1, mSps->frame_cropping_flag);

  if (mSps->frame_cropping_flag) {
    mWriter->Uev(mSps->frame_crop_left_offset);
    mWriter->Uev(mSps->frame_crop_right_offset);
    mWriter->Uev(mSps->frame_crop_top_offset);
    mWriter->Uev(mSps->frame_crop_bottom_offset);
  }

  mWriter->U(1, mSps->vui_parameters_present_flag);
  if (mSps->vui_parameters_present_flag) {
    WriteSpsVuiParameters();
  }
}

void H264Sprop::WriteSpsVuiParameters()
{
  mWriter->U(1, mSps->Vui.aspect_ratio_info_present_flag);
  if (mSps->Vui.aspect_ratio_info_present_flag) {
    mWriter->U(8, mSps->Vui.aspect_ratio_idc);
    if (mSps->Vui.aspect_ratio_idc == 255) {
      mWriter->U(16, mSps->Vui.sar_width);
      mWriter->U(16, mSps->Vui.sar_height);
    }
  }
  mWriter->U(1, mSps->Vui.overscan_info_present_flag);
  if (mSps->Vui.overscan_info_present_flag) {
    mWriter->U(1, mSps->Vui.overscan_appropriate_flag);
  }
  mWriter->U(1, mSps->Vui.video_signal_type_present_flag);
  if (mSps->Vui.video_signal_type_present_flag) {
    mWriter->U(3, mSps->Vui.video_format);
    mWriter->U(1, mSps->Vui.video_full_range_flag);
    mWriter->U(1, mSps->Vui.colour_description_present_flag);
    if (mSps->Vui.colour_description_present_flag) {
      mWriter->U(8, mSps->Vui.colour_primaries);
      mWriter->U(8, mSps->Vui.transfer_characteristics);
      mWriter->U(8, mSps->Vui.matrix_coefficients);
    }
  }
  mWriter->U(1, mSps->Vui.chroma_loc_info_present_flag);
  if (mSps->Vui.chroma_loc_info_present_flag) {
    mWriter->Uev(mSps->Vui.chroma_sample_loc_type_top_field);
    mWriter->Uev(mSps->Vui.chroma_sample_loc_type_bottom_field);
  }
  mWriter->U(1, mSps->Vui.timing_info_present_flag);
  if (mSps->Vui.timing_info_present_flag) {
    mWriter->U(32, mSps->Vui.num_units_in_tick);
    mWriter->U(32, mSps->Vui.time_scale);
    mWriter->U(1, mSps->Vui.fixed_frame_rate_flag);
  }
  mWriter->U(1, 0);
  mWriter->U(1, 0);
  if (mSps->Vui.nal_hrd_parameters_present_flag || mSps->Vui.vcl_hrd_parameters_present_flag) {
    mWriter->U(1, mSps->Vui.low_delay_hrd_flag);
  }
  mWriter->U(1, mSps->Vui.pic_struct_present_flag);
  mWriter->U(1, mSps->Vui.bitstream_restriction_flag);
  if (mSps->Vui.bitstream_restriction_flag) {
    mWriter->U(1, mSps->Vui.motion_vectors_over_pic_boundaries_flag);
    mWriter->Uev(mSps->Vui.max_bytes_per_pic_denom);
    mWriter->Uev(mSps->Vui.max_bits_per_mb_denom);
    mWriter->Uev(mSps->Vui.log2_max_mv_length_horizontal);
    mWriter->Uev(mSps->Vui.log2_max_mv_length_vertical);
    mWriter->Uev(mSps->Vui.max_num_reorder_frames);
    mWriter->Uev(mSps->Vui.max_dec_frame_buffering);
  }
}

int H264Sprop::WritePps()
{
  mWriter->Uev(mPps->pic_parameter_set_id);
  mWriter->Uev(mPps->seq_parameter_set_id);
  mWriter->U(1, mPps->entropy_coding_mode_flag);
  mWriter->U(1, mPps->pic_order_present_flag);
  mWriter->Uev(0);

  mWriter->Uev(mPps->num_ref_idx_l0_default_active_minus1);
  mWriter->Uev(mPps->num_ref_idx_l1_default_active_minus1);
  mWriter->U(1, mPps->weighted_pred_flag);
  mWriter->U(2, mPps->weighted_bipred_idc);
  mWriter->Sev(mPps->pic_init_qp_minus26);
  mWriter->Sev(mPps->pic_init_qs_minus26);
  mWriter->Sev(mPps->chroma_qp_index_offset);
  mWriter->U(1, mPps->deblocking_filter_control_present_flag);
  mWriter->U(1, mPps->constrained_intra_pred_flag);
  mWriter->U(1, mPps->redundant_pic_cnt_present_flag);

  return true;
}

int H264Sprop::WriteTrail()
{
  mWriter->U(1, 1);
  int pos = mWriter->Position();
  for (int i = 0; i < 7; i++) {
    mWriter->U(1, 0);
    if (mWriter->Position() > pos) {
      return mWriter->Position();
    }
  }
  return pos;
}

bool H264Sprop::Test(const char* data, int size)
{
  mReader.reset(new BitstreamReader(data, size));

  READ(mNalUnit.forbidden_zero_bit, mReader->U(1));
  READ(mNalUnit.nal_ref_idc, mReader->U(2));
  READ(mNalUnit.nal_unit_type, mReader->U(5));

  if (mNalUnit.nal_unit_type == 14 || mNalUnit.nal_unit_type == 20 || mNalUnit.nal_unit_type == 21) {
    return false;
  } else if (mNalUnit.nal_unit_type == eSps) {
    mSps.reset(new H264Sps());
    return true;
  } else if (mNalUnit.nal_unit_type == ePps) {
    mPps.reset(new H264Pps());
    return true;
  } else {
    return false;
  }
}

bool H264Sprop::Parse(const char* data, int size, bool useSlice)
{
  mReader.reset(new BitstreamReader(data, size));

  READ(mNalUnit.forbidden_zero_bit, mReader->U(1));
  READ(mNalUnit.nal_ref_idc, mReader->U(2));
  READ(mNalUnit.nal_unit_type, mReader->U(5));

  if (mNalUnit.nal_unit_type == 14 || mNalUnit.nal_unit_type == 20 || mNalUnit.nal_unit_type == 21) {
    return false;
  } else if (mNalUnit.nal_unit_type == eSps) {
    mSps.reset(new H264Sps());
    return ParseSps() && ParseTrail();
  } else if (mNalUnit.nal_unit_type == ePps) {
    mPps.reset(new H264Pps());
    return ParsePps() && ParseTrail();
  } else if (mNalUnit.nal_unit_type == eSei) {
    mSei.reset(new H264Sei());
    ParseSei() && ParseTrail();
    return false;
  } else if (useSlice && mNalUnit.nal_unit_type >= eSliceNonIdr && mNalUnit.nal_unit_type <= eSliceIdr) {
    mSlice.reset(new H264Slice());
    ParseSlice() && ParseTrail();
    return false;
  } else {
    return false;
  }
}

bool H264Sprop::AtEnd()
{
  return mReader->EndOfData();
}

bool H264Sprop::MoreRbspData()
{
  return mReader->BytePosition() < mReader->Size() - 1;
}

bool H264Sprop::ParseSps()
{
  memset(mSps.data(), 0, sizeof(H264Sps));
  mSps->chroma_format_idc = 1;
  memset(mSps->scaling_lists_4x4, 16, 6*16);
  memset(mSps->scaling_lists_8x8, 16, 6*64);

  READ(mSps->profile_idc,          mReader->U(8));
  READ(mSps->constraint_set0_flag, mReader->U(1));
  READ(mSps->constraint_set1_flag, mReader->U(1));
  READ(mSps->constraint_set2_flag, mReader->U(1));
  READ(mSps->constraint_set3_flag, mReader->U(1));
  READ(mSps->constraint_set4_flag, mReader->U(1));
  READ(mSps->constraint_set5_flag, mReader->U(1));
  READ(mSps->reserved_zero_2bits,  mReader->U(2));
  READ(mSps->level_idc,            mReader->U(8));
  READ(mSps->seq_parameter_set_id, mReader->Uev());

  if ( mSps->profile_idc  == 100 || mSps->profile_idc  == 110
       || mSps->profile_idc  == 122 || mSps->profile_idc  == 244 || mSps->profile_idc  == 44
       || mSps->profile_idc  ==  83 || mSps->profile_idc  ==  86 || mSps->profile_idc  == 118
       || mSps->profile_idc  == 128 || mSps->profile_idc  == 138)
  {
    READ(mSps->chroma_format_idc, mReader->Uev());
    if (mSps->chroma_format_idc == 3) {
      READ(mSps->separate_colour_plane_flag, mReader->U(1));
    }

    READ(mSps->bit_depth_luma_minus8,                 mReader->Uev());
    READ(mSps->bit_depth_chroma_minus8,               mReader->Uev());
    READ(mSps->qpprime_y_zero_transform_bypass_flag , mReader->U(1));
    READ(mSps->seq_scaling_matrix_present_flag,       mReader->U(1));

    if (mSps->seq_scaling_matrix_present_flag) {
      for (int i = 0; i < ((mSps->chroma_format_idc != 3)? 8: 12); i++) {
        int seq_scaling_list_present_flag_i;
        READ(seq_scaling_list_present_flag_i, mReader->U(1));
        if (seq_scaling_list_present_flag_i) {
          if (!ParseSpsScalingList(i < 6? 16: 64)) {
            return false;
          }
        }
        if (mReader->EndOfData()) {
          return false;
        }
      }
    }
  }

  READ(mSps->log2_max_frame_num_minus4, mReader->Uev());
  READ(mSps->pic_order_cnt_type,        mReader->Uev());

  if (mSps->pic_order_cnt_type == 0) {
    READ(mSps->log2_max_pic_order_cnt_lsb_minus4, mReader->Uev());
  } else if (mSps->pic_order_cnt_type == 1) {
    READ(mSps->delta_pic_order_always_zero_flag,      mReader->U(1));
    READ(mSps->offset_for_non_ref_pic,                mReader->Sev());
    READ(mSps->offset_for_top_to_bottom_field,        mReader->Sev());
    READ(mSps->num_ref_frames_in_pic_order_cnt_cycle, mReader->Uev());

    for (int i = 0; i < mSps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
      READ(mSps->offset_for_ref_frame_i, mReader->Sev());
      if (mReader->EndOfData()) {
        return false;
      }
    }
  }

  READ(mSps->max_num_ref_frames,                   mReader->Uev());
  READ(mSps->gaps_in_frame_num_value_allowed_flag, mReader->U(1));
  READ(mSps->pic_width_in_mbs_minus1,              mReader->Uev());
  READ(mSps->pic_height_in_map_units_minus1,       mReader->Uev());
  READ(mSps->frame_mbs_only_flag,                  mReader->U(1));
  if (!mSps->frame_mbs_only_flag) {
    READ(mSps->mb_adaptive_frame_field_flag, mReader->U(1));
  }

  READ(mSps->direct_8x8_inference_flag, mReader->U(1));
  READ(mSps->frame_cropping_flag,       mReader->U(1));

  if (mSps->frame_cropping_flag) {
    READ(mSps->frame_crop_left_offset,   mReader->Uev());
    READ(mSps->frame_crop_right_offset,  mReader->Uev());
    READ(mSps->frame_crop_top_offset,    mReader->Uev());
    READ(mSps->frame_crop_bottom_offset, mReader->Uev());
  }

  READ(mSps->vui_parameters_present_flag, mReader->U(1));
  if (mSps->vui_parameters_present_flag) {
    if (!ParseSpsVuiParameters()) {
      return false;
    }
  }
  return true;
}

bool H264Sprop::ParsePps()
{
  READ(mPps->pic_parameter_set_id,                  mReader->Uev());
  READ(mPps->seq_parameter_set_id,                  mReader->Uev());
  READ(mPps->entropy_coding_mode_flag,              mReader->U(1));
  READ(mPps->pic_order_present_flag,                mReader->U(1));
  READ(mPps->num_slice_groups_minus1,               mReader->Uev());
  if (mPps->num_slice_groups_minus1 > 0) {
    READ(mPps->slice_group_map_type,                mReader->Uev());
    if (mPps->slice_group_map_type == 0) {
      for (int iGroup = 0; iGroup <= mPps->num_slice_groups_minus1; iGroup++) {
        READ(mPps->run_length_minus1_iGroup,        mReader->Uev());
      }
    } else if (mPps->slice_group_map_type == 2) {
      for (int iGroup = 0; iGroup <= mPps->num_slice_groups_minus1; iGroup++) {
        READ(mPps->top_left_iGroup,                 mReader->Uev());
        READ(mPps->top_right_iGroup,                mReader->Uev());
      }
    } else if (mPps->slice_group_map_type == 3 || mPps->slice_group_map_type == 4 || mPps->slice_group_map_type == 5) {
      READ(mPps->slice_group_change_direction_flag, mReader->U(1));
      READ(mPps->slice_group_change_rate_minus1,    mReader->Uev());
    } else if (mPps->slice_group_map_type == 6) {
      READ(mPps->pic_size_in_map_units_minus1,      mReader->Uev());
      int sz = 1;
      for (int v = mPps->num_slice_groups_minus1; v > 1; v >>= 1) {
        sz++;
      }
      for (int i = 0; i <= mPps->pic_size_in_map_units_minus1; i++) {
        READ(mPps->slice_group_id_i,                mReader->U(sz));
      }
    }
  }

  READ(mPps->num_ref_idx_l0_default_active_minus1,  mReader->Uev());
  READ(mPps->num_ref_idx_l1_default_active_minus1,  mReader->Uev());
  READ(mPps->weighted_pred_flag,                    mReader->U(1));
  READ(mPps->weighted_bipred_idc,                   mReader->U(2));
  READ(mPps->pic_init_qp_minus26,                   mReader->Sev());
  READ(mPps->pic_init_qs_minus26,                   mReader->Sev());
  READ(mPps->chroma_qp_index_offset,                mReader->Sev());
  READ(mPps->deblocking_filter_control_present_flag,mReader->U(1));
  READ(mPps->constrained_intra_pred_flag,           mReader->U(1));
  READ(mPps->redundant_pic_cnt_present_flag,        mReader->U(1));

  if (mReader->BytePosition() < mReader->Size() - 1 || (mReader->BytePosition() == mReader->Size() - 1 && mReader->ByteAligned() && (int)(uchar)mReader->CurrentByte() != 0x80)) {
    READ(mPps->transform_8x8_mode_flag,             mReader->U(1));
    READ(mPps->pic_scaling_matrix_present_flag,     mReader->U(1));
    if (mPps->pic_scaling_matrix_present_flag) {
      for (int i = 0; i < 6 + 2 * mPps->transform_8x8_mode_flag; i++) {
        int pic_scaling_list_present_flag_i;
        READ(pic_scaling_list_present_flag_i,       mReader->U(1));
        if (pic_scaling_list_present_flag_i) {
          if (!ParseSpsScalingList(i < 6? 16: 64)) {
            return false;
          }
        }
      }
    }
    READ(mPps->second_chroma_qp_index_offset,       mReader->Sev());
  }
  return true;
}

bool H264Sprop::ParseSei()
{
  do {
    ParseSeiMsg();
  } while (MoreRbspData());
  return true;
}

bool H264Sprop::ParseSlice()
{
  if (!mSps || !mPps) {
    return false;
  }
  READ(mSlice->first_mb_in_slice,              mReader->Uev());
  READ(mSlice->slice_type,                     mReader->Uev());
  READ(mSlice->pic_parameter_set_id,           mReader->Uev());
  READ(mSlice->frame_num,                      mReader->U(mSps->log2_max_frame_num_minus4 + 4));
  if (!mSps->frame_mbs_only_flag) {
    READ(mSlice->field_pic_flag,               mReader->U(1));
    if (mSlice->field_pic_flag) {
      READ(mSlice->bottom_field_flag,          mReader->U(1));
    }
  }
  if (mNalUnit.nal_unit_type == 5) {
    READ(mSlice->idr_pic_id,                   mReader->Uev());
  }
  if (mSps->pic_order_cnt_type == 0) {
    READ(mSlice->pic_order_cnt_lsb,            mReader->U(mSps->log2_max_pic_order_cnt_lsb_minus4 + 4));
    if (mPps->pic_order_present_flag &&  !mSlice->field_pic_flag) {
      READ(mSlice->delta_pic_order_cnt_bottom, mReader->Sev());
    }
  }
  if (mSps->pic_order_cnt_type == 1 && !mSps->delta_pic_order_always_zero_flag) {
    READ(mSlice->delta_pic_order_cnt0,         mReader->Sev());
    if (mPps->pic_order_present_flag && !mSlice->field_pic_flag) {
      READ(mSlice->delta_pic_order_cnt1,       mReader->Sev());
    }
  }

  if (mPps->redundant_pic_cnt_present_flag) {
    READ(mSlice->redundant_pic_cnt,            mReader->Uev());
  }
  if (mSlice->slice_type == eSliceB) {
    READ(mSlice->direct_spatial_mv_pred_flag,  mReader->U(1));
  }
  if (mSlice->slice_type == eSliceP || mSlice->slice_type == eSliceSp || mSlice->slice_type == eSliceB) {
    READ(mSlice->num_ref_idx_active_override_flag, mReader->U(1));
    if (mSlice->num_ref_idx_active_override_flag) {
      READ(mSlice->num_ref_idx_l0_active_minus1,   mReader->Uev());
      if (mSlice->slice_type == eSliceB) {
        READ(mSlice->num_ref_idx_l1_active_minus1, mReader->Uev());
      }
    }
  }
  return false;
  ParseSliceRefPicListReordering();
  if ( (mPps->weighted_pred_flag && (mSlice->slice_type == eSliceP || mSlice->slice_type == eSliceSp) )
       || (mPps->weighted_bipred_idc == 1 && mSlice->slice_type == eSliceB) ) {
    ParseSlicePredWeightTable();
  }
  if (mNalUnit.nal_ref_idc != 0) {
    ParseSliceDecRefPicMarking();
  }
  if (mPps->entropy_coding_mode_flag && mSlice->slice_type != eSliceI && mSlice->slice_type != eSliceSi) {
    READ(mSlice->cabac_init_idc,                   mReader->Uev());
  }
  READ(mSlice->slice_qp_delta,                     mReader->Sev());

  if (mSlice->slice_type == eSliceSp || mSlice->slice_type == eSliceSi) {
    if (mSlice->slice_type == eSliceSp) {
      READ(mSlice->sp_for_switch_flag,             mReader->U(1));
    }
    READ(mSlice->slice_qs_delta,                   mReader->Sev());
  }

  if (mPps->deblocking_filter_control_present_flag) {
    READ(mSlice->disable_deblocking_filter_idc,    mReader->Uev());
    if (mSlice->disable_deblocking_filter_idc != 1) {
      READ(mSlice->slice_alpha_c0_offset_div2,     mReader->Sev());
      READ(mSlice->slice_beta_offset_div2,         mReader->Sev());
    }
  }
  if (mPps->num_slice_groups_minus1 > 0 && mPps->slice_group_map_type >= 3 && mPps->slice_group_map_type <= 5) {
   READ(mSlice->slice_group_change_cycle,          mReader->Uev());
  }

  return true;
}

bool H264Sprop::ParseTrail()
{
  int rbsp_stop_one_bit;
  READ(rbsp_stop_one_bit, mReader->U(1));
  if (rbsp_stop_one_bit != 1) {
    return false;
  }
  for (int i = 0; i < 7; i++) {
    if (mReader->ByteAligned()) {
      return true;
    }
    int rbsp_alignment_zero_bit = 0;
    READ(rbsp_alignment_zero_bit, mReader->U(1));
    if (rbsp_alignment_zero_bit != 0) {
      return false;
    }
  }
  return true;
}

bool H264Sprop::ParseSpsScalingList(int sizeOfScalingList)
{
  int lastScale = 8;
  int nextScale = 8;

  for (int j = 0; j < sizeOfScalingList; j++) {
    if (nextScale != 0) {
      int delta_scale;
      READ(delta_scale, mReader->Sev());
      nextScale = (lastScale + delta_scale + 256) % 256;
    }
    lastScale = (nextScale == 0)? lastScale: nextScale;
    if (mReader->EndOfData()) {
      return false;
    }
  }
  return !mReader->EndOfData();
}

bool H264Sprop::ParseSpsVuiParameters()
{
  READ(mSps->Vui.aspect_ratio_info_present_flag, mReader->U(1));
  if (mSps->Vui.aspect_ratio_info_present_flag) {
    READ(mSps->Vui.aspect_ratio_idc, mReader->U(8));
    if (mSps->Vui.aspect_ratio_idc == 255) {
      READ(mSps->Vui.sar_width, mReader->U(16));
      READ(mSps->Vui.sar_height, mReader->U(16));
    }
  }
  READ(mSps->Vui.overscan_info_present_flag, mReader->U(1));
  if (mSps->Vui.overscan_info_present_flag) {
    READ(mSps->Vui.overscan_appropriate_flag, mReader->U(1));
  }
  READ(mSps->Vui.video_signal_type_present_flag, mReader->U(1));
  if (mSps->Vui.video_signal_type_present_flag) {
    READ(mSps->Vui.video_format,                    mReader->U(3));
    READ(mSps->Vui.video_full_range_flag,           mReader->U(1));
    READ(mSps->Vui.colour_description_present_flag, mReader->U(1));
    if (mSps->Vui.colour_description_present_flag) {
      READ(mSps->Vui.colour_primaries,         mReader->U(8));
      READ(mSps->Vui.transfer_characteristics, mReader->U(8));
      READ(mSps->Vui.matrix_coefficients,      mReader->U(8));
    }
  }
  READ(mSps->Vui.chroma_loc_info_present_flag, mReader->U(1));
  if (mSps->Vui.chroma_loc_info_present_flag) {
    READ(mSps->Vui.chroma_sample_loc_type_top_field,    mReader->Uev());
    READ(mSps->Vui.chroma_sample_loc_type_bottom_field, mReader->Uev());
  }
  READ(mSps->Vui.timing_info_present_flag, mReader->U(1));
  if (mSps->Vui.timing_info_present_flag) {
    READ(mSps->Vui.num_units_in_tick,     mReader->U(32));
    READ(mSps->Vui.time_scale,            mReader->U(32));
    READ(mSps->Vui.fixed_frame_rate_flag, mReader->U(1));
  }
  READ(mSps->Vui.nal_hrd_parameters_present_flag, mReader->U(1));
  if (mSps->Vui.nal_hrd_parameters_present_flag) {
    ParseHdrParameters();
  }
  READ(mSps->Vui.vcl_hrd_parameters_present_flag, mReader->U(1));
  if (mSps->Vui.vcl_hrd_parameters_present_flag) {
    ParseHdrParameters();
  }
  if (mSps->Vui.nal_hrd_parameters_present_flag || mSps->Vui.vcl_hrd_parameters_present_flag) {
    READ(mSps->Vui.low_delay_hrd_flag, mReader->U(1));
  }
  READ(mSps->Vui.pic_struct_present_flag, mReader->U(1));
  READ(mSps->Vui.bitstream_restriction_flag, mReader->U(1));
  if (mSps->Vui.bitstream_restriction_flag) {
    READ(mSps->Vui.motion_vectors_over_pic_boundaries_flag, mReader->U(1));
    READ(mSps->Vui.max_bytes_per_pic_denom,                 mReader->Uev());
    READ(mSps->Vui.max_bits_per_mb_denom,                   mReader->Uev());
    READ(mSps->Vui.log2_max_mv_length_horizontal,           mReader->Uev());
    READ(mSps->Vui.log2_max_mv_length_vertical,             mReader->Uev());
    READ(mSps->Vui.max_num_reorder_frames,                  mReader->Uev());
    READ(mSps->Vui.max_dec_frame_buffering,                 mReader->Uev());
  }
  return !mReader->EndOfData();
}

bool H264Sprop::ParseHdrParameters()
{
  H264Sps::HrdParameters hdr_parameters;
  READ(hdr_parameters.cpb_cnt_minus1, mReader->Uev());
  READ(hdr_parameters.bit_rate_scale, mReader->U(4));
  READ(hdr_parameters.cpb_size_scale, mReader->U(4));
  for (int i = 0; i <= hdr_parameters.cpb_cnt_minus1; i++) {
    READ(hdr_parameters.bit_rate_value_minus1_i, mReader->Uev());
    READ(hdr_parameters.cpb_size_value_minus1_i, mReader->Uev());
    READ(hdr_parameters.cbr_flag_i,              mReader->U(1));
    if (mReader->EndOfData()) {
      return false;
    }
  }
  READ(hdr_parameters.initial_cpb_removal_delay_length_minus1, mReader->U(5));
  READ(hdr_parameters.cpb_removal_delay_length_minus1,         mReader->U(5));
  READ(hdr_parameters.dpb_output_delay_length_minus1,          mReader->U(5));
  READ(hdr_parameters.time_offset_length,                      mReader->U(5));

  return !mReader->EndOfData();
}

bool H264Sprop::ParseSeiMsg()
{
  int type = 0;
  int size = 0;
  int nextByte;
  do {
    READ(nextByte, mReader->U(8));
    type += nextByte;
  } while (nextByte == 0xff);
  do {
    READ(nextByte, mReader->U(8));
    size += nextByte;
  } while (nextByte == 0xff);

  mReader->SkipBytes(size);
  return true;
}

bool H264Sprop::ParseSliceRefPicListReordering()
{

  return !mReader->EndOfData();
}

bool H264Sprop::ParseSlicePredWeightTable()
{

  return !mReader->EndOfData();
}

bool H264Sprop::ParseSliceDecRefPicMarking()
{

  return !mReader->EndOfData();
}


H264Sprop::H264Sprop()
{
}

H264Sprop::~H264Sprop()
{
}
