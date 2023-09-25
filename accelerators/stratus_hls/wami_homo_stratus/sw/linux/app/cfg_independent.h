// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_INDEPENDENT_H__
#define __CFG_INDEPENDENT_H__

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
#define NACC 1

// [humu]: TODO: update the hardcoded values for the offsets
struct wami_homo_stratus_access wami_debayer3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_DEBAYER3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = DEBAYER_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424, // from bin
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_grayscale3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRAYSCALE3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRAYSCALE_KERN_ID,
        .wami_batch            = 0,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + 16384, // from debayer
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_gradient3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRADIENT3, // output: grad_x
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRADIENT_KERN_ID,
        .wami_batch            = 0,                                        // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRADIENT3 + 16384,     // output: grad y
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_GRADIENT3 + 16384 * 2, // from grayscale
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_warp_img3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_IMG3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 0,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_IMG3 + 16384,     // from grayscale
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_IMG3 + 16384 * 2, // array 0
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_sub3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_SUB3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = SUBTRACT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_SUB3 + 16384,     // IWxp (from bin)
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_SUB3 + 16384 * 2, // from warp-img
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_warp_x3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_X3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 0,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_X3 + 16384,     // grad x
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_X3 + 16384 * 2, // array 0
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_warp_y3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_Y3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 0,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_Y3 + 16384,     // grad y
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_Y3 + 16384 * 2, // array 0
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_steep_descent3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = STEEPEST_DESCENT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + 16384 * 6, // from warp-x
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + 16384 * 7, // from warp-y
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_hessian3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_HESSIAN3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = HESSIAN_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_HESSIAN3 + 36, // from steep_descent
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_inv3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_INV3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = INVERT_GAUSS_JORDAN_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_INV3 + 36, // from hessian
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_sd_update3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_SD_UPDATE3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = SD_UPDATE_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + 6,             // from steep_descent
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + 6 + 16384 * 6, // from sub
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_mult3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_MULT3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = MULT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_MULT3 + 6,      // from inv
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_MULT3 + 6 + 36, // from sd update
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_reshape3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_RESHAPE3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = RESHAPE_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_RESHAPE3 + 6, // from mult
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_add3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_ADD3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = ADD_KERN_ID,
        .wami_batch            = 0,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_ADD3 + 6,     // array 0
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_ADD3 + 6 + 6, // from reshape
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_warp_iwxp3_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_IWXP3,
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 0,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + 16384,     // from grayscale
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + 16384 * 2, // golden_affine_warp (from bin)
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_homo_stratus_access wami_gradient3_dummy_cfg_indep[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRADIENT3, // output: same as input
        .wami_num_img          = 1,
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRADIENT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = 0,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_IWXP3, // input
        .wami_src_dst_offset_3 = 24,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

esp_thread_info_t cfg_wami_debayer3_indep[] = {{
    .run       = true,
    .devname   = "wami_debayer3_stratus.0",
    .ioctl_req = WAMI_DEBAYER3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_debayer3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_grayscale3_indep[] = {{
    .run       = true,
    .devname   = "wami_grayscale3_stratus.0",
    .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_grayscale3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_gradient3_indep[] = {{
    .run       = true,
    .devname   = "wami_gradient3_stratus.0",
    .ioctl_req = WAMI_GRADIENT3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_gradient3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_warp_img3_indep[] = {{
    .run       = true,
    .devname   = "wami_warp3_stratus.0",
    .ioctl_req = WAMI_WARP_IMG3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_warp_img3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_sub3_indep[] = {{
    .run       = true,
    .devname   = "wami_sub3_stratus.0",
    .ioctl_req = WAMI_SUB3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_sub3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_warp_x3_indep[] = {{
    .run       = true,
    .devname   = "wami_warp3_stratus.1",
    .ioctl_req = WAMI_WARP_X3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_warp_x3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_warp_y3_indep[] = {{
    .run       = true,
    .devname   = "wami_warp3_stratus.2",
    .ioctl_req = WAMI_WARP_Y3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_warp_y3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_steep_descent3_indep[] = {{
    .run       = true,
    .devname   = "wami_steep_descent3_stratus.0",
    .ioctl_req = WAMI_STEEP_DESCENT3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_steep_descent3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_hessian3_indep[] = {{
    .run       = true,
    .devname   = "wami_hessian3_stratus.0",
    .ioctl_req = WAMI_HESSIAN3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_hessian3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_inv3_indep[] = {{
    .run       = true,
    .devname   = "wami_inv3_stratus.0",
    .ioctl_req = WAMI_INV3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_inv3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_sd_update3_indep[] = {{
    .run       = true,
    .devname   = "wami_sd_update3_stratus.0",
    .ioctl_req = WAMI_SD_UPDATE3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_sd_update3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_mult3_indep[] = {{
    .run       = true,
    .devname   = "wami_mult3_stratus.0",
    .ioctl_req = WAMI_MULT3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_mult3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_reshape3_indep[] = {{
    .run       = true,
    .devname   = "wami_reshape3_stratus.0",
    .ioctl_req = WAMI_RESHAPE3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_reshape3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_add3_indep[] = {{
    .run       = true,
    .devname   = "wami_add3_stratus.0",
    .ioctl_req = WAMI_ADD3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_add3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_warp_iwxp3_indep[] = {{
    .run       = true,
    .devname   = "wami_warp3_stratus.3",
    .ioctl_req = WAMI_WARP_IWXP3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_warp_iwxp3_cfg_indep[0].esp),
}};

esp_thread_info_t cfg_wami_gradient3_dummy_indep[] = {{
    .run       = true,
    .devname   = "wami_gradient3_stratus.1",
    .ioctl_req = WAMI_GRADIENT3_STRATUS_IOC_ACCESS,
    .esp_desc  = &(wami_gradient3_dummy_cfg_indep[0].esp),
}};

#endif /* __CFG_INDEPENDENT_H__ */
