// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef __CFG_P2P_COMBO4_H__
#define __CFG_P2P_COMBO4_H__

#include "libesp.h"
#include "wami_run_stratus.h"

// typedef int32_t token_t;
typedef uint64_t token_t;

#define P2P_BATCH_NUM 100

// [humu]: TODO: update the hardcoded values for the offsets
struct wami_run_stratus_access wami_debayer3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_DEBAYER3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = DEBAYER_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 0,
        .esp.p2p_srcs          = {"", "", "", ""},
    }};

struct wami_run_stratus_access wami_grayscale3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
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
        .wami_src_dst_offset_4 = 3,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_debayer3_stratus.0", "", "", ""},
    }};

struct wami_run_stratus_access wami_gradient3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRADIENT3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRADIENT_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRADIENT3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_GRADIENT3 + 16384 * 2,
        .wami_src_dst_offset_3 = 0, // is_dummy
        .wami_src_dst_offset_4 = 2,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_grayscale3_stratus.0", "", "", ""},
    }};

struct wami_run_stratus_access wami_warp_img3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_IMG3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_IMG3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_IMG3 + 16384 * 2,
        .wami_src_dst_offset_3 = 0, // warp_no: 0: warp-img, 1: warp-dx, 2: warp-dy, 3: warp-iwxp
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 2,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_srcs = {"wami_mult_reshape_add_stratus.0", "wami_grayscale3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_sub3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_SUB3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = SUBTRACT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_SUB3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_SUB3 + 16384 * 2,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 2,
        .esp.p2p_srcs          = {"wami_warp3_stratus.3", "wami_warp3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_warp_x3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_X3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_X3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_X3 + 16384 * 2,
        .wami_src_dst_offset_3 = 1, // warp_no: 0: warp-img, 1: warp-dx, 2: warp-dy, 3: warp-iwxp
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        //.esp.p2p_nsrcs         = 0,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_nsrcs = 2,
        .esp.p2p_srcs  = {"wami_mult_reshape_add_stratus.0", "wami_gradient3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_warp_y3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_Y3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_Y3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_Y3 + 16384 * 2,
        .wami_src_dst_offset_3 = 2, // warp_no: 0: warp-img, 1: warp-dx, 2: warp-dy, 3: warp-iwxp
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        //.esp.p2p_nsrcs         = 0,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_nsrcs = 2,
        .esp.p2p_srcs  = {"wami_mult_reshape_add_stratus.0", "wami_gradient3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_steep_descent3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = STEEPEST_DESCENT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + 16384 * 6,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + 16384 * 7,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 2,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        //.esp.p2p_nsrcs         = 0,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_nsrcs = 2,
        .esp.p2p_srcs  = {"wami_warp3_stratus.1", "wami_warp3_stratus.2", "", ""},
    }};

struct wami_run_stratus_access wami_hessian3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_HESSIAN3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = HESSIAN_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_HESSIAN3 + 36,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_steep_descent3_stratus.0", "", "", ""},
    }};

struct wami_run_stratus_access wami_inv3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_INV3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = INVERT_GAUSS_JORDAN_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_INV3 + 36,
        .wami_src_dst_offset_2 = 0,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_hessian3_stratus.0", "", "", ""},
    }};

struct wami_run_stratus_access wami_sd_update3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_SD_UPDATE3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = SD_UPDATE_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + 6,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + 6 + 16384 * 6,
        .wami_src_dst_offset_3 = 0,
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        //.esp.p2p_nsrcs         = 0,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_nsrcs = 2,
        .esp.p2p_srcs  = {"wami_steep_descent3_stratus.0", "wami_sub3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_mult_reshape_add_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_ADD3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = MULT_RESHAPE_ADD_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_ADD3 + 6,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_ADD3 + 6 + 6,
        .wami_src_dst_offset_3 = 1, // num_forward_pass
        .wami_src_dst_offset_4 = 3, // num_backward_pass
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 2,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_srcs = {"wami_inv3_stratus.0", "wami_sd_update3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_warp_iwxp3_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_WARP_IWXP3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = WARP_KERN_ID,
        .wami_batch            = 1, // is_p2p
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + 16384 * 2,
        .wami_src_dst_offset_3 = 3, // warp_no: 0: warp-img, 1: warp-dx, 2: warp-dy, 3: warp-iwxp
        .wami_src_dst_offset_4 = (1 << 16) + 1,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 1,
        .esp.p2p_nsrcs         = 2,
        //.esp.p2p_srcs          = {"", "", "", ""},
        .esp.p2p_srcs = {"wami_mult_reshape_add_stratus.0", "wami_grayscale3_stratus.0", "", ""},
    }};

struct wami_run_stratus_access wami_gradient3_dummy_cfg_p2p_combo4[] = {
    /* <<--descriptor-->> */
    {
        .src_offset            = 0,
        .dst_offset            = 0,
        .wami_src_dst_offset_0 = MEM_BASE_ADDR_WAMI_GRADIENT3,
        .wami_num_img          = P2P_BATCH_NUM, // num_img
        .wami_num_col          = 128,
        .wami_num_row          = 128,
        .wami_pad              = 2,
        .wami_kern_id          = GRADIENT_KERN_ID,
        .wami_batch            = 1,
        .wami_src_dst_offset_1 = MEM_BASE_ADDR_WAMI_GRADIENT3 + 16384,
        .wami_src_dst_offset_2 = MEM_BASE_ADDR_WAMI_GRADIENT3 + 16384 * 2,
        .wami_src_dst_offset_3 = 24, // is_dummy
        .wami_src_dst_offset_4 = 0,
        .esp.coherence         = ACC_COH_NONE,
        .esp.p2p_store         = 0,
        .esp.p2p_nsrcs         = 1,
        .esp.p2p_srcs          = {"wami_warp3_stratus.3", "", "", ""},
    }};

esp_thread_info_t cfg_wami_p2p_combo4[] = {
    /* blank placeholder for formatter */
    {
        .run       = true,
        .devname   = "wami_mult_reshape_add_stratus.0",
        .ioctl_req = WAMI_MULT_RESHAPE_ADD_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_mult_reshape_add_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_debayer3_stratus.0",
        .ioctl_req = WAMI_DEBAYER3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_debayer3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_grayscale3_stratus.0",
        .ioctl_req = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_grayscale3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_gradient3_stratus.0",
        .ioctl_req = WAMI_GRADIENT3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_gradient3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_warp3_stratus.0",
        .ioctl_req = WAMI_WARP_IMG3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_warp_img3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_sub3_stratus.0",
        .ioctl_req = WAMI_SUB3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_sub3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_warp3_stratus.1",
        .ioctl_req = WAMI_WARP_X3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_warp_x3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_warp3_stratus.2",
        .ioctl_req = WAMI_WARP_Y3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_warp_y3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_steep_descent3_stratus.0",
        .ioctl_req = WAMI_STEEP_DESCENT3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_steep_descent3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_hessian3_stratus.0",
        .ioctl_req = WAMI_HESSIAN3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_hessian3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_inv3_stratus.0",
        .ioctl_req = WAMI_INV3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_inv3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_sd_update3_stratus.0",
        .ioctl_req = WAMI_SD_UPDATE3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_sd_update3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_warp3_stratus.3",
        .ioctl_req = WAMI_WARP_IWXP3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_warp_iwxp3_cfg_p2p_combo4[0].esp),
    },
    {
        .run       = true,
        .devname   = "wami_gradient3_stratus.1",
        .ioctl_req = WAMI_GRADIENT3_STRATUS_IOC_ACCESS,
        .esp_desc  = &(wami_gradient3_dummy_cfg_p2p_combo4[0].esp),
    }
};

#endif /* __CFG_P2P_COMBO4_H__ */
