// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_MTM_H__
#define __CFG_MTM_H__

#include "libesp.h"
#include "nightp2p_stratus.h"


#define P2P_BATCH_NUM 100

// [humu]: TODO: update the hardcoded values for the offsets
struct nightNF_stratus_access nightNF_cfg_mtm[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .is_p2p        = 0,
        .p2p_config_0  = 1,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 0,
        .esp.p2p_nsrcs = 0,
        .esp.p2p_srcs  = {"", "", "", ""},

    }};

struct nightvision_stratus_access nightHist_cfg_mtm[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 0,
        .esp.p2p_nsrcs = 0,
        .esp.p2p_srcs  = {"", "", "", ""},

    }};

struct nightvision_stratus_access nightHistEq_cfg_mtm[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 0,
        .esp.p2p_nsrcs = 0,
        .esp.p2p_srcs  = {"", "", "", ""},

    }};

struct nightvision_stratus_access nightDwt_cfg_mtm[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 0,
        .esp.p2p_nsrcs = 0,
        .esp.p2p_srcs  = {"", "", "", ""},

    }};

esp_thread_info_t cfg_nightNF_mtm[] = {{
    .run       = true,
    .devname   = "nightNF_stratus.0",
    .ioctl_req = NIGHTNF_STRATUS_IOC_ACCESS,
    .esp_desc  = &(nightNF_cfg_mtm[0].esp),
}};

esp_thread_info_t cfg_nightHist_mtm[] = {{
    .run       = true,
    .devname   = "nightHist_stratus.0",
    .ioctl_req = NIGHTHIST_STRATUS_IOC_ACCESS,
    .esp_desc  = &(nightHist_cfg_mtm[0].esp),
}};

esp_thread_info_t cfg_nightHistEq_mtm[] = {{
    .run       = true,
    .devname   = "nightHistEq_stratus.0",
    .ioctl_req = NIGHTHISTEQ_STRATUS_IOC_ACCESS,
    .esp_desc  = &(nightHistEq_cfg_mtm[0].esp),
}};

esp_thread_info_t cfg_nightDwt_mtm[] = {{
    .run       = true,
    .devname   = "nightDwt_stratus.0",
    .ioctl_req = NIGHTDWT_STRATUS_IOC_ACCESS,
    .esp_desc  = &(nightDwt_cfg_mtm[0].esp),
}};

#endif /* __CFG_MTM_H__ */
