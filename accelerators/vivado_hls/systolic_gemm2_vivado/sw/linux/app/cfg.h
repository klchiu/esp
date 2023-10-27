// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "systolic_gemm2_vivado.h"

typedef int16_t token_t;

/* <<--params-def-->> */
#define MATRIX_C_DIM 2
#define MATRIX_A_DIM 2
#define MATRIX_B_DIM 2

/* <<--params-->> */
const int32_t matrix_C_dim = MATRIX_C_DIM;
const int32_t matrix_A_dim = MATRIX_A_DIM;
const int32_t matrix_B_dim = MATRIX_B_DIM;

#define NACC 1

struct systolic_gemm2_vivado_access systolic_gemm2_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.matrix_C_dim = MATRIX_C_DIM,
		.matrix_A_dim = MATRIX_A_DIM,
		.matrix_B_dim = MATRIX_B_DIM,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};

esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "systolic_gemm2_vivado.0",
		.ioctl_req = SYSTOLIC_GEMM2_VIVADO_IOC_ACCESS,
		.esp_desc = &(systolic_gemm2_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
