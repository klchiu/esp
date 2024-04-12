# Copyright (c) 2011-2023 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>
set_directive_interface -mode ap_none "top" conf_info_m3_offset
set_directive_interface -mode ap_none "top" conf_info_d3
set_directive_interface -mode ap_none "top" conf_info_d2
set_directive_interface -mode ap_none "top" conf_info_d1
set_directive_interface -mode ap_none "top" conf_info_m2_offset
set_directive_interface -mode ap_none "top" conf_info_m1_offset

# Insert here any custom directive
set_directive_dataflow "top/go"
