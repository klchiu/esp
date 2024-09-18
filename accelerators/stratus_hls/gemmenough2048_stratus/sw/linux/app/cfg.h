// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "gemmenough2048_stratus.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define NINPUTS 1
#define D3 8
#define D2 8
#define D1 8
#define ST_OFFSET 128
#define LD_OFFSET1 0
#define LD_OFFSET2 64

/* <<--params-->> */
const int32_t ninputs = NINPUTS;
const int32_t d3 = D3;
const int32_t d2 = D2;
const int32_t d1 = D1;
const int32_t st_offset = ST_OFFSET;
const int32_t ld_offset1 = LD_OFFSET1;
const int32_t ld_offset2 = LD_OFFSET2;

#define NACC 1

struct gemmenough2048_stratus_access gemmenough2048_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.ninputs = NINPUTS,
		.d3 = D3,
		.d2 = D2,
		.d1 = D1,
		.st_offset = ST_OFFSET,
		.ld_offset1 = LD_OFFSET1,
		.ld_offset2 = LD_OFFSET2,
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
		.devname = "gemmenough2048_stratus.0",
		.ioctl_req = GEMMENOUGH2048_STRATUS_IOC_ACCESS,
		.esp_desc = &(gemmenough2048_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
