// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_INDEPENDENT_H__
#define __CFG_INDEPENDENT_H__

#include "libesp.h"
#include "wami_synth3_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

// [humu]: TODO: update the hardcoded values for the offsets
struct wami_synth3_stratus_access wami_synth3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 0,
        .wami_p2p_config_0     = 128,
        .wami_p2p_config_1     = 128,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

esp_thread_info_t cfg_wami_synth3_0_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.0",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_synth3_1_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.1",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_synth3_2_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.2",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_synth3_3_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.3",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};


esp_thread_info_t cfg_wami_synth3_4_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.4",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_synth3_5_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.5",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_synth3_6_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.6",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_synth3_7_indep[] = {{
    .run       = true,
    .devname   = "wami_synth3_stratus.7",
    .ioctl_req = WAMI_SYNTH3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_synth3_cfg_indep[0].esp),
}};

#endif /* __CFG_INDEPENDENT_H__ */
