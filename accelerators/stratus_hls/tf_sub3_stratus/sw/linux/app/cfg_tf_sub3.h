// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_TF_SUB3_H__
#define __CFG_TF_SUB3_H__

#include "libesp.h"
#include "tf_sub3_stratus.h"

typedef int32_t token_t;

/* <<--params-def-->> */

/*
#define NUM_BATCH_K 1
#define BATCH_SIZE_K 16
#define NUM_BATCH_X 1
#define BATCH_SIZE_X 4
*/
/* <<--params-->> */
/*
const int32_t num_batch_k = NUM_BATCH_K;
const int32_t batch_size_k = BATCH_SIZE_K;
const int32_t num_batch_x = NUM_BATCH_X;
const int32_t batch_size_x = BATCH_SIZE_X;
*/
#define NACC 1

struct tf_sub3_stratus_access tf_sub3_cfg[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = 2,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = 0,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

esp_thread_info_t cfg_tf_sub3[] = {{
    .run       = true,
    .devname   = "tf_sub3_stratus.0",
    .ioctl_req = TF_SUB3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(tf_sub3_cfg[0].esp),
}};

#endif /* __CFG_TF_SUB3_H__ */
