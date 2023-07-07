// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_P2P_B_H__
#define __CFG_P2P_B_H__

#include "libesp.h"
#include "ny_acc_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

struct ny_acc_stratus_access ny_acc_cfg_p2p_b_0[] = {
    /* ny_acc_0 */
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
        .esp.p2p_store    = 1,
        .esp.p2p_nsrcs    = 0,
        .esp.p2p_srcs     = {"", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_b_1[] = {
    /* ny_acc_1 */
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
        .esp.p2p_store    = 1,
        .esp.p2p_nsrcs    = 1,
        .esp.p2p_srcs     = {"ny_acc_stratus.0", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_b_2[] = {
    /* ny_acc_2 */
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
        .esp.p2p_store    = 1,
        .esp.p2p_nsrcs    = 1,
        .esp.p2p_srcs     = {"ny_acc_stratus.0", "", "", ""},
    }};
struct ny_acc_stratus_access ny_acc_cfg_p2p_b_3[] = {
    /* ny_acc_3 */
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
        .esp.p2p_nsrcs    = 2,
        .esp.p2p_srcs     = {"ny_acc_stratus.1", "ny_acc_stratus.2", "", ""},
    }};

esp_thread_info_t cfg_ny_acc_p2p_b[] = {
    /* blank placeholder for formatter */
    {
        .run       = true,
        .devname   = "ny_acc_stratus.0",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_b_0[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.1",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_b_1[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.2",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_b_2[0].esp),
    },
    {
        .run       = true,
        .devname   = "ny_acc_stratus.3",
        .ioctl_req = NY_ACC_STRATUS_IOC_ACCESS,
        .esp_desc  = &(ny_acc_cfg_p2p_b_3[0].esp),
    }};

#endif /* __CFG_P2P_B_H__ */
