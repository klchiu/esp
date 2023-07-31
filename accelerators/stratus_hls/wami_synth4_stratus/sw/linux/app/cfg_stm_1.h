// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_STM_1_H__
#define __CFG_STM_1_H__

#include "libesp.h"
#include "wami_synth4_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

struct wami_synth4_stratus_access wami_synth4_cfg_stm_1[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 1073741824,    // 2^30
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 0,
        .wami_p2p_config_0     = 0,
        .wami_p2p_config_1     = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

esp_thread_info_t cfg_wami_synth4_stm_1[] = {
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.0",
        .devname_noid = "wami_synth4_stratus",
	    .puffinname = "Black Bird",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_stm_1[0].esp),
    // },
    // {
        // .run       = true,
        // .devname   = "wami_synth4_stratus.1",
        // .devname_noid = "wami_synth4_stratus",
	    // .puffinname = "White Peacock",
        // .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        // .esp_desc  = &(wami_synth4_cfg_stm_1[0].esp),
    }
};


#endif /* __CFG_STM_1_H__ */
