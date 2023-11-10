// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "conv2d_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define DO_RELU 0
#define STRIDE 1
#define FEATURE_MAP_WIDTH 6
#define N_CHANNELS 2
#define N_FILTERS 2
#define BATCH_SIZE 1
#define FILTER_DIM 3
#define IS_PADDED 1
#define POOL_TYPE 0
#define FEATURE_MAP_HEIGHT 6

/* <<--params-->> */
const int32_t do_relu = DO_RELU;
const int32_t stride = STRIDE;
const int32_t feature_map_width = FEATURE_MAP_WIDTH;
const int32_t n_channels = N_CHANNELS;
const int32_t n_filters = N_FILTERS;
const int32_t batch_size = BATCH_SIZE;
const int32_t filter_dim = FILTER_DIM;
const int32_t is_padded = IS_PADDED;
const int32_t pool_type = POOL_TYPE;
const int32_t feature_map_height = FEATURE_MAP_HEIGHT;

#define NACC 1

struct conv2d_vivado_access conv2d_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.do_relu = DO_RELU,
		.stride = STRIDE,
		.feature_map_width = FEATURE_MAP_WIDTH,
		.n_channels = N_CHANNELS,
		.n_filters = N_FILTERS,
		.batch_size = BATCH_SIZE,
		.filter_dim = FILTER_DIM,
		.is_padded = IS_PADDED,
		.pool_type = POOL_TYPE,
		.feature_map_height = FEATURE_MAP_HEIGHT,
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
		.devname = "conv2d_vivado.0",
		.ioctl_req = CONV2D_VIVADO_IOC_ACCESS,
		.esp_desc = &(conv2d_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
