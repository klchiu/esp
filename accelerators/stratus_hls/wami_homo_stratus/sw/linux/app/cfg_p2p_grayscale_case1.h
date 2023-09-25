// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_P2P_GRAYSCALE_CASE1_H__
#define __CFG_P2P_GRAYSCALE_CASE1_H__

#include "libesp.h"
#include "wami_homo_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

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
#define NACC          1
#define P2P_BATCH_NUM 100

struct wami_homo_stratus_access wami_grayscale3_cfg_p2p_case1[] = {
    // wami_grayscale3_stratus.0
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    },
    // wami_grayscale3_stratus.1
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.0", "", "", ""},
    },
    // wami_grayscale3_stratus.2
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.1", "", "", ""},
    },
    // wami_grayscale3_stratus.3
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.2", "", "", ""},
    },
    // wami_grayscale3_stratus.4
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.5", "", "", ""},
    },
    // wami_grayscale3_stratus.5
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.6", "", "", ""},
    },
    // wami_grayscale3_stratus.6
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.7", "", "", ""},
    },
    // wami_grayscale3_stratus.7
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.3", "", "", ""},
    },
    // wami_grayscale3_stratus.8
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.4", "", "", ""},
    },
    // wami_grayscale3_stratus.9
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.8", "", "", ""},
    },
    // wami_grayscale3_stratus.10
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.9", "", "", ""},
    },
    // wami_grayscale3_stratus.11
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.10", "", "", ""},
    },
    // wami_grayscale3_stratus.12
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.13", "", "", ""},
    },
    // wami_grayscale3_stratus.13
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.14", "", "", ""},
    },
    // wami_grayscale3_stratus.14
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.15", "", "", ""},
    },
    // wami_grayscale3_stratus.15
     {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.11", "", "", ""},
    }
    };

esp_thread_info_t cfg_wami_p2p_grayscale3_case1[] = {
    /* blank placeholder for formatter */
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.0",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.1",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[1].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.2",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[2].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.3",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[3].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.4",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[4].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.5",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[5].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.6",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[6].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.7",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[7].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.8",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[8].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.9",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[9].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.10",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[10].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.11",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[11].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.12",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[12].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.13",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[13].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.14",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[14].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.15",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_case1[15].esp),
    }};

#endif /* __CFG_P2P_GRAYSCALE_CASE1_H__ */
