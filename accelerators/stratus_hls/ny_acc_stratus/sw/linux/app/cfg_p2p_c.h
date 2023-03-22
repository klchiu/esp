// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_P2P_C_H__
#define __CFG_P2P_C_H__

#include "libesp.h"
#include "ny_acc_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

struct ny_acc_stratus_access ny_acc_cfg_p2p_c_0[] = {
    /* ny_acc_0 */
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
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_c_1[] = {
    /* ny_acc_1 */
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
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"ny_acc_stratus.0", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_c_2[] = {
    /* ny_acc_2 */
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
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"ny_acc_stratus.1", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_c_3[] = {
    /* ny_acc_3 */
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
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,                // 1,
        .esp.p2p_srcs          = {"", "", "", ""}, // {"ny_acc_stratus.1", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_c_4[] = {
    /* ny_acc_4 */
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
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1, // 1,
        .esp.p2p_nsrcs         = 1, // 2,
        .esp.p2p_srcs          = {"ny_acc_stratus.2", "", "",
                         ""}, // {"ny_acc_stratus.2", "ny_acc_stratus.3", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_c_5[] = {
    /* ny_acc_5*/
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
        .wami_is_p2p           = 1,
        .wami_p2p_config_0     = 1, // num_forward_pass
        .wami_p2p_config_1     = 0, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"ny_acc_stratus.4", "", "", ""},
    }};

esp_thread_info_t cfg_ny_acc_p2p_c[] = {
    /* blank placeholder for formatter */
    {
        .run       = true,
        .devname   = "ny_acc_stratus.0",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_c_0[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.1",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_c_1[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.2",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_c_2[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.3",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_c_3[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.4",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_c_4[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.5",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_c_5[0].esp),
    }};

#endif /* __CFG_P2P_C_H__ */
