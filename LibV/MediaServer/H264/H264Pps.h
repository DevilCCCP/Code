#pragma once


struct H264Pps {
  int pic_parameter_set_id;
  int seq_parameter_set_id;
  int entropy_coding_mode_flag;
  int pic_order_present_flag;
  int num_slice_groups_minus1;
  int run_length_minus1_iGroup;
  int top_left_iGroup;
  int top_right_iGroup;
  int slice_group_map_type;
  int slice_group_change_direction_flag;
  int slice_group_change_rate_minus1;
  int pic_size_in_map_units_minus1;
  int slice_group_id_i;
  int num_ref_idx_l0_default_active_minus1;
  int num_ref_idx_l1_default_active_minus1;
  int weighted_pred_flag;
  int weighted_bipred_idc;
  int pic_init_qp_minus26; /* relative to 26 */
  int pic_init_qs_minus26; /* relative to 26 */
  int chroma_qp_index_offset;
  int deblocking_filter_control_present_flag;
  int constrained_intra_pred_flag;
  int redundant_pic_cnt_present_flag;
  int transform_8x8_mode_flag;
  int pic_scaling_matrix_present_flag;
  int second_chroma_qp_index_offset;
};

