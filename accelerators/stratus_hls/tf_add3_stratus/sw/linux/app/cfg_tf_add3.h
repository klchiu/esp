// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_TF_ADD3_H__
#define __CFG_TF_ADD3_H__

#include "libesp.h"
#include "tf_add3_stratus.h"

/* User defined */

// Define data type (decomment the one needed)
// #define __UINT
// #define __INT
#define __FIXED
//#define __FLOAT

// Define bit width (decomment the one needed)
// #ifndef __riscv
#define BITWIDTH 32
// #define BITWIDTH 64
// #else
// #define BITWIDTH 32
// #define BITWIDTH 64
// #endif

/* End of user defined */

#ifdef __UINT
#if (BITWIDTH == 32)
typedef unsigned token_t;
#elif (BITWIDTH == 64)
typedef long long unsigned token_t;
#endif
#endif

#ifdef __INT
#if (BITWIDTH == 32)
typedef int token_t;
#elif (BITWIDTH == 64)
typedef long long token_t;
#endif
#endif

#ifdef __FIXED
#if (BITWIDTH == 32)
typedef int token_t;
#define fx2float fixed32_to_float
#define float2fx float_to_fixed32
#define FX_IL 16
#elif (BITWIDTH == 64)
typedef long long token_t;
#define fx2float fixed64_to_double
#define float2fx double_to_fixed64
#define FX_IL 32
#endif
#endif

#ifdef __FLOAT
#if (BITWIDTH == 32)
typedef float token_t;
#elif (BITWIDTH == 64)
typedef double token_t;
#endif
#endif

typedef float native_t;

#define MAX_PRINTED_ERRORS 512

/* <<--params-def-->> */
#define NACC 1
#define ACC_TLB_ENTRIES 128
#define ACC_PAGE_SIZE (1 << 20)
#define MAX_SIZE (ACC_PAGE_SIZE * ACC_TLB_ENTRIES)
#define MAX_TESTS 30

struct tf_add3_stratus_access tf_add3_cfg_000[] = {
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

esp_thread_info_t cfg_tf_add3[] = {
	{
		.run = true,
		.devname = "tf_add3_stratus.0",
		.ioctl_req = TF_ADD3_STRATUS_IOC_ACCESS,
		.esp_desc = &(tf_add3_cfg_000[0].esp),
	}
};

#endif /* __CFG_TF_ADD3_H__ */
