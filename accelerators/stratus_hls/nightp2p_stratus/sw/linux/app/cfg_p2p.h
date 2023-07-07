// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_P2P_H__
#define __CFG_P2P_H__

#include "libesp.h"
#include "nightp2p_stratus.h"



#define P2P_BATCH_NUM 100

// [humu]: TODO: update the hardcoded values for the offsets
struct nightNF_stratus_access nightNF_cfg_p2p[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .is_p2p        = 1,
        .p2p_config_0  = 2,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 1,
        .esp.p2p_nsrcs = 0,
        .esp.p2p_srcs  = {"", "", "", ""},

    }};

struct nightvision_stratus_access nightHist_cfg_p2p[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 1,
        .esp.p2p_nsrcs = 1,
        .esp.p2p_srcs  = {"nightNF_stratus.0", "", "", ""},

    }};

struct nightvision_stratus_access nightHistEq_cfg_p2p[] = {
    /* <<--descriptor-->> */
    {
        .src_offset    = 0,
        .dst_offset    = 0,
        .nimages       = 1,
        .rows          = 480,
        .cols          = 640,
        .do_dwt        = 1,
        .esp.coherence = ACC_COH_NONE,
        .esp.p2p_store = 1,
        .esp.p2p_nsrcs = 0,
        // .esp.p2p_srcs  = {"nightNF_stratus.0", "nightHist_stratus.0", "", ""},
        .esp.p2p_srcs  = {"", "", "", ""},

    }};

struct nightvision_stratus_access nightDwt_cfg_p2p[] = {
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
        .esp.p2p_nsrcs = 1,
        .esp.p2p_srcs  = {"nightHistEq_stratus.0", "", "", ""},

    }};

esp_thread_info_t cfg_night_p2p[] = {
//     {
//     .run       = true,
//     .devname   = "nightNF_stratus.0",
//     .ioctl_req = NIGHTNF_STRATUS_IOC_ACCESS,
//     .esp_desc  = &(nightNF_cfg_p2p[0].esp),
// },

// {
//     .run       = true,
//     .devname   = "nightHist_stratus.0",
//     .ioctl_req = NIGHTHIST_STRATUS_IOC_ACCESS,
//     .esp_desc  = &(nightHist_cfg_p2p[0].esp),
// },

{
    .run       = true,
    .devname   = "nightHistEq_stratus.0",
    .ioctl_req = NIGHTHISTEQ_STRATUS_IOC_ACCESS,
    .esp_desc  = &(nightHistEq_cfg_p2p[0].esp),
},

{
    .run       = true,
    .devname   = "nightDwt_stratus.0",
    .ioctl_req = NIGHTDWT_STRATUS_IOC_ACCESS,
    .esp_desc  = &(nightDwt_cfg_p2p[0].esp),
}};

#endif /* __CFG_P2P_H__ */
