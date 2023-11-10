# Copyright (c) 2011-2023 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>
set_directive_interface -mode ap_none "top" conf_info_do_relu
set_directive_interface -mode ap_none "top" conf_info_stride
set_directive_interface -mode ap_none "top" conf_info_feature_map_width
set_directive_interface -mode ap_none "top" conf_info_n_channels
set_directive_interface -mode ap_none "top" conf_info_n_filters
set_directive_interface -mode ap_none "top" conf_info_batch_size
set_directive_interface -mode ap_none "top" conf_info_filter_dim
set_directive_interface -mode ap_none "top" conf_info_is_padded
set_directive_interface -mode ap_none "top" conf_info_pool_type
set_directive_interface -mode ap_none "top" conf_info_feature_map_height

# Insert here any custom directive
set_directive_dataflow "top/go"
