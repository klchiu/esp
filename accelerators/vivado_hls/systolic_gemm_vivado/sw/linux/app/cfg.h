// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __ESP_CFG_000_H__
#define __ESP_CFG_000_H__

#include "libesp.h"
#include "systolic_gemm_vivado.h"

typedef int32_t token_t;

/* <<--params-def-->> */
#define MAC_VEC 100
#define MAC_LEN 64
#define MAC_N 1

/* <<--params-->> */
const int32_t mac_vec = MAC_VEC;
const int32_t mac_len = MAC_LEN;
const int32_t mac_n = MAC_N;

#define NACC 1

struct systolic_gemm_vivado_access systolic_gemm_cfg_000[] = {
	{
		/* <<--descriptor-->> */
		.mac_vec = MAC_VEC,
		.mac_len = MAC_LEN,
		.mac_n = MAC_N,
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
		.devname = "systolic_gemm_vivado.0",
		.ioctl_req = SYSTOLIC_GEMM_VIVADO_IOC_ACCESS,
		.esp_desc = &(systolic_gemm_cfg_000[0].esp),
	}
};

#endif /* __ESP_CFG_000_H__ */
