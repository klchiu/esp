# Copyright (c) 2011-2023 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>
set_directive_interface -mode ap_none "top" conf_info_matrix_C_dim
set_directive_interface -mode ap_none "top" conf_info_matrix_A_dim
set_directive_interface -mode ap_none "top" conf_info_matrix_B_dim

# Insert here any custom directive
# set_directive_dataflow "top/go"
set_directive_dataflow "compute_dataflow"



set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_west_11
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_west_12
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_west_21
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_west_22
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_north_11
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_north_12
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_north_21
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" input_north_22
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_east_12
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_east_22
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_south_21
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_south_22
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_buff_11
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_buff_12
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_buff_21
set_directive_array_partition -type cyclic -factor ${unroll_factor} -dim 1 "top" output_buff_22
