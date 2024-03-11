# Copyright (c) 2011-2023 Columbia University, System Level Design Group
# SPDX-License-Identifier: Apache-2.0

# User-defined configuration ports
# <<--directives-param-->>
set_directive_interface -mode ap_none "top" conf_info_matrix_dim_x
set_directive_interface -mode ap_none "top" conf_info_matrix_dim_y
set_directive_interface -mode ap_none "top" conf_info_load_state

# Insert here any custom directive
# set_directive_dataflow "top/go"
set_directive_dataflow "compute_dataflow"
# set_directive_inline "compute_dataflow"
# set_directive_function_instantiate "compute/compute_dateflow/compute_PE_2" PE_index
# set_directive_function_instantiate "compute/compute_dateflow/compute_PE_3" PE_index
# set_directive_function_instantiate "compute/compute_dateflow/compute_PE_4" PE_index
# set_directive_function_instantiate "compute/compute_dateflow/compute_PE_5" PE_index
