// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_TF_MULT3_H__
#define __CFG_TF_MULT3_H__

#include "libesp.h"
#include "tf_mult3_stratus.h"


typedef uint64_t token_t;
typedef float native_t;

#define fx2float             fixed64_to_double
#define float2fx             double_to_fixed64
#define FX_IL           34



struct tf_mult3_stratus_access tf_mult3_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.tf_length = 1024,
		.tf_src_dst_offset_0 = 0,
		.tf_src_dst_offset_1 = 1024,
		.tf_src_dst_offset_2 = 2048,
		.src_offset = 0,
		.dst_offset = 0,
		.esp.coherence = ACC_COH_NONE,
		.esp.p2p_store = 0,
		.esp.p2p_nsrcs = 0,
		.esp.p2p_srcs = {"", "", "", ""},
	}
};

esp_thread_info_t cfg_tf_mult3[] = {
	{
		.run = true,
		.devname = "tf_mult3_stratus.0",
		.ioctl_req = TF_MULT3_STRATUS_IOC_ACCESS,
		.esp_desc = &(tf_mult3_cfg_000[0].esp),
	}
};

#endif /* __CFG_TF_MULT3_H__ */
