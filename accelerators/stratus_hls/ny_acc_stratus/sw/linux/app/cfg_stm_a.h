// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_STM_A_H__
#define __CFG_STM_A_H__

#include "libesp.h"
#include "ny_acc_stratus.h"

typedef uint64_t token_t;

struct ny_acc_stratus_access ny_acc_cfg_stm_a[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384,
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

esp_thread_info_t cfg_ny_acc_0_stm_a[] = {{
    .run       = true,
    .devname   = "ny_acc_stratus.0",
    .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
    .esp_desc  = &(ny_acc_cfg_stm_a[0].esp),
}};

esp_thread_info_t cfg_ny_acc_1_stm_a[] = {{
    .run       = true,
    .devname   = "ny_acc_stratus.1",
    .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
    .esp_desc  = &(ny_acc_cfg_stm_a[0].esp),
}};

esp_thread_info_t cfg_ny_acc_2_stm_a[] = {{
    .run       = true,
    .devname   = "ny_acc_stratus.2",
    .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
    .esp_desc  = &(ny_acc_cfg_stm_a[0].esp),
}};

esp_thread_info_t cfg_ny_acc_3_stm_a[] = {{
    .run       = true,
    .devname   = "ny_acc_stratus.3",
    .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
    .esp_desc  = &(ny_acc_cfg_stm_a[0].esp),
}};

#endif /* __CFG_STM_A_H__ */
