// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "huangemm3s2048_stratus.h"

typedef int32_t token_t;

/* <<--params-def-->> */
// #define HUANGEMM3S2048_N 1
// #define HUANGEMM3S2048_VEC 100
// #define HUANGEMM3S2048_LEN 64

#define HUANGEMM3S2048_ROWS 4
#define HUANGEMM3S2048_COLS 1000
#define HUANGEMM3S2048_LOADED_COLS 4

/* <<--params-->> */
const int32_t rows = HUANGEMM3S2048_ROWS;
const int32_t cols = HUANGEMM3S2048_COLS;
const int32_t loaded_cols = HUANGEMM3S2048_LOADED_COLS;

// #define NACC 2

struct huangemm3s2048_stratus_access huangemm3s2048_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.rows = HUANGEMM3S2048_ROWS,
		.cols = HUANGEMM3S2048_COLS,
		.loaded_cols = HUANGEMM3S2048_LOADED_COLS,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};



// usage: esp_run(cfg_000, 2);
esp_thread_info_t cfg_000[] = {
	{
		.run = true,
		.devname = "huangemm3s2048_stratus.0",
		.ioctl_req = HUANGEMM3S2048_STRATUS_IOC_ACCESS,
		.esp_desc = &(huangemm3s2048_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */