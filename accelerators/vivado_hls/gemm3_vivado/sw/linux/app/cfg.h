// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "gemm3_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define M3_OFFSET 0
#define D3 32
#define D2 32
#define D1 32
#define M2_OFFSET 2048
#define M1_OFFSET 1024

/* <<--params-->> */
const int32_t m3_offset = M3_OFFSET;
const int32_t d3 = D3;
const int32_t d2 = D2;
const int32_t d1 = D1;
const int32_t m2_offset = M2_OFFSET;
const int32_t m1_offset = M1_OFFSET;

#define NACC 1

struct gemm3_vivado_access gemm3_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.m3_offset = M3_OFFSET,
		.d3 = D3,
		.d2 = D2,
		.d1 = D1,
		.m2_offset = M2_OFFSET,
		.m1_offset = M1_OFFSET,
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
		.devname = "gemm3_vivado.0",
		.ioctl_req = GEMM3_VIVADO_IOC_ACCESS,
		.esp_desc = &(gemm3_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
