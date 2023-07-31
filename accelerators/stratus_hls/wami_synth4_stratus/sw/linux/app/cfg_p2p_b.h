// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_P2P_B_H__
#define __CFG_P2P_B_H__

#include "libesp.h"
#include "wami_synth4_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

struct wami_synth4_stratus_access wami_synth4_cfg_p2p_b[] = {
    /* wami_synth4_0 */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384, // delay
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    },
    /* wami_synth4_1 */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384, // delay
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 2,
        .esp.p2p_srcs          = {"wami_synth4_stratus.4", "wami_synth4_stratus.0", "", ""},
    },
    /* wami_synth4_2 */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384, // delay
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_synth4_stratus.1", "", "", ""},
    },
    /* wami_synth4_3 */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384, // delay
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_synth4_stratus.2", "", "", ""},
    },
    /* wami_synth4_4 */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384, // delay
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 1, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    },
    /* wami_synth4_5*/
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 0,
        .wami_kern_id          = SYNTH_KERN_ID,
        .wami_batch            = 16384, // delay
        .wami_src_dst_offset_0 = 0,
        .wami_src_dst_offset_1 = 16384, // 128*128
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_synth4_stratus.4", "", "", ""},
    }};

esp_thread_info_t cfg_wami_synth4_p2p_b[] = {
    /* blank placeholder for formatter */
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.4",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_p2p_b[4].esp),
    },
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.0",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_p2p_b[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.1",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_p2p_b[1].esp),
    },
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.2",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_p2p_b[2].esp),
    },
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.3",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_p2p_b[3].esp),
    },
    {
        .run       = true,
        .devname   = "wami_synth4_stratus.5",
        .ioctl_req = WAMI_SYNTH4_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_synth4_cfg_p2p_b[5].esp),
    }};

#endif /* __CFG_P2P_B_H__ */
