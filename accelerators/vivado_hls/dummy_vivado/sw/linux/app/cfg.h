// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "dummy_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define OUTPUT 8
#define INPUT2 8
#define INPUT1 8

/* <<--params-->> */
const int32_t output = OUTPUT;
const int32_t input2 = INPUT2;
const int32_t input1 = INPUT1;

#define NACC 1

struct dummy_vivado_access dummy_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.output = OUTPUT,
		.input2 = INPUT2,
		.input1 = INPUT1,
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
		.devname = "dummy_vivado.0",
		.ioctl_req = DUMMY_VIVADO_IOC_ACCESS,
		.esp_desc = &(dummy_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
