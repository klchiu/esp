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
        .src_offset       = 0,
        .dst_offset       = 0,
        .batch            = 1,
        .num_load         = 128,
        .num_store        = 128,
        .delay            = 16384,
        .src_dst_offset_0 = 0,
        .src_dst_offset_1 = 16384, // 128*128
        .src_dst_offset_2 = 0,
        .src_dst_offset_3 = 0,
        .src_dst_offset_4 = 0,
        .esp.coherence    = ACC_COH_NONE,
        .esp.p2p_store    = 0,
        .esp.p2p_nsrcs    = 0,
        .esp.p2p_srcs     = {"", "", "", ""},
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
