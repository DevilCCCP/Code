#pragma once


struct H264Slice {
  int first_mb_in_slice;
  int slice_type;
  int pic_parameter_set_id;
  int frame_num;
  int field_pic_flag;
  int bottom_field_flag;
  int idr_pic_id;
  int pic_order_cnt_lsb;
  int delta_pic_order_cnt_bottom;
  int delta_pic_order_cnt0;
  int delta_pic_order_cnt1;
  int redundant_pic_cnt;
  int direct_spatial_mv_pred_flag;
  int num_ref_idx_active_override_flag;
  int num_ref_idx_l0_active_minus1;
  int num_ref_idx_l1_active_minus1;
  int cabac_init_idc;
  int slice_qp_delta;
  int sp_for_switch_flag;
  int slice_qs_delta;
  int disable_deblocking_filter_idc;
  int slice_alpha_c0_offset_div2;
  int slice_beta_offset_div2;
  int slice_group_change_cycle;

  struct RefPicListReordering {
    int ref_pic_list_reordering_flag_l0;
    int ref_pic_list_reordering_flag_l1;

    struct RefPicReordering {
      int reordering_of_pic_nums_idc;
      int abs_diff_pic_num_minus1;
      int long_term_pic_num;
    };
  };

  struct PredWeightTable {
    int luma_log2_weight_denom;
    int chroma_log2_weight_denom;
    int chroma_weight_l0_flag;

    struct LumaWeight {
      int luma_weight_lx_flag;
      int luma_weight_lx;
      int luma_offset_lx;
    };
    struct ChromaWeight {
      int chroma_weight_lx;
      int chroma_offset_lx;
    };
  };

  struct DecRefPicMarking {
    int no_output_of_prior_pics_flag;
    int long_term_reference_flag;
    int adaptive_ref_pic_marking_mode_flag;

    struct MemoryManagementOperation {
      int memory_management_control_operation;
      int difference_of_pic_nums_minus1;
      int long_term_pic_num;
      int long_term_frame_idx;
      int max_long_term_frame_idx_plus1;
    };
  };
};
