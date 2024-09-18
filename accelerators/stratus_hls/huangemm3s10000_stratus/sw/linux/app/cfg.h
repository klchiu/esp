// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "huangemm3s10000_stratus.h"

typedef int32_t token_t;

/* <<--params-def-->> */
// #define HUANGEMM3s10000_N 1
// #define HUANGEMM3s10000_VEC 100
// #define HUANGEMM3s10000_LEN 64

#define HUANGEMM3s10000_ROWS 4
#define HUANGEMM3s10000_COLS 1000
#define HUANGEMM3s10000_LOADED_COLS 4

/* <<--params-->> */
const int32_t rows = HUANGEMM3s10000_ROWS;
const int32_t cols = HUANGEMM3s10000_COLS;
const int32_t loaded_cols = HUANGEMM3s10000_LOADED_COLS;

// #define NACC 2

struct huangemm3s10000_stratus_access huangemm3s10000_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.rows = HUANGEMM3s10000_ROWS,
		.cols = HUANGEMM3s10000_COLS,
		.loaded_cols = HUANGEMM3s10000_LOADED_COLS,
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
		.devname = "huangemm3s10000_stratus.0",
		.ioctl_req = HUANGEMM3s10000_STRATUS_IOC_ACCESS,
		.esp_desc = &(huangemm3s10000_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
