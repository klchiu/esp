// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _WAMI_HOMO_STRATUS_H_
#define _WAMI_HOMO_STRATUS_H_

#ifdef __KERNEL__
    #include <linux/ioctl.h>
    #include <linux/types.h>
#else
    #include <sys/ioctl.h>
    #include <stdint.h>
    #ifndef __user
        #define __user
    #endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

#include "wami_C_data.h"

// [humu]: TODO: change 132 and 128 to nRows, NCols and M, N
#define MEM_BASE_ADDR_WAMI_DEBAYER3       0
#define MEM_BASE_ADDR_WAMI_GRAYSCALE3     MEM_BASE_ADDR_WAMI_DEBAYER3 + (132 * 132) + (128 * 128)
#define MEM_BASE_ADDR_WAMI_GRADIENT3      MEM_BASE_ADDR_WAMI_GRAYSCALE3 + (128 * 128) * 2
#define MEM_BASE_ADDR_WAMI_WARP_IMG3      MEM_BASE_ADDR_WAMI_GRADIENT3 + (128 * 128) * 3
#define MEM_BASE_ADDR_WAMI_SUB3           MEM_BASE_ADDR_WAMI_WARP_IMG3 + (128 * 128) * 2 + 6
#define MEM_BASE_ADDR_WAMI_WARP_X3        MEM_BASE_ADDR_WAMI_SUB3 + (128 * 128) * 3
#define MEM_BASE_ADDR_WAMI_WARP_Y3        MEM_BASE_ADDR_WAMI_WARP_X3 + (128 * 128) * 2 + 6
#define MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 MEM_BASE_ADDR_WAMI_WARP_Y3 + (128 * 128) * 2 + 6
#define MEM_BASE_ADDR_WAMI_HESSIAN3       MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + (128 * 128) * 8
#define MEM_BASE_ADDR_WAMI_INV3           MEM_BASE_ADDR_WAMI_HESSIAN3 + (128 * 128) * 6 + 36
#define MEM_BASE_ADDR_WAMI_SD_UPDATE3     MEM_BASE_ADDR_WAMI_INV3 + (36 * 2)
#define MEM_BASE_ADDR_WAMI_MULT3          MEM_BASE_ADDR_WAMI_SD_UPDATE3 + (128 * 128) * 7 + 6
#define MEM_BASE_ADDR_WAMI_RESHAPE3       MEM_BASE_ADDR_WAMI_MULT3 + 36 + 6 + 6
#define MEM_BASE_ADDR_WAMI_ADD3           MEM_BASE_ADDR_WAMI_RESHAPE3 + 6 + 6
#define MEM_BASE_ADDR_WAMI_WARP_IWXP3     MEM_BASE_ADDR_WAMI_ADD3 + 6 * 2 + 6
#define MEM_ONE_IMAGE_SIZE                MEM_BASE_ADDR_WAMI_WARP_IWXP3 + (128 * 128) * 2 + 6 // original version
//#define MEM_BASE_ADDR_WAMI_GRADIENT3_DUMMY MEM_BASE_ADDR_WAMI_WARP_IWXP3 + (128 * 128) * 2 + 6
//#define MEM_ONE_IMAGE_SIZE                 MEM_BASE_ADDR_WAMI_GRADIENT3_DUMMY + (128 * 128) * 3 // gradient dummy only
// use 2 (M*N)

/*
#define MEM_BASE_ADDR_WAMI_GRAYSCALE3_OUT          MEM_BASE_ADDR_WAMI_GRAYSCALE3
#define MEM_BASE_ADDR_WAMI_GRAYSCALE3_IN          MEM_BASE_ADDR_WAMI_GRAYSCALE3 + (132*132)
#define MEM_BASE_ADDR_WAMI_GRADIENT3_OUT           MEM_BASE_ADDR_WAMI_GRADIENT3
#define MEM_BASE_ADDR_WAMI_WARP_IMG3_OUT           MEM_BASE_ADDR_WAMI_WARP_IMG3
#define MEM_BASE_ADDR_WAMI_SUB3_OUT                MEM_BASE_ADDR_WAMI_SUB3
#define MEM_BASE_ADDR_WAMI_WARP_X3_OUT             MEM_BASE_ADDR_WAMI_WARP_X3
#define MEM_BASE_ADDR_WAMI_WARP_Y3             MEM_BASE_ADDR_WAMI_WARP_Y3
#define MEM_BASE_ADDR_WAMI_STEEP_DESCENT3      MEM_BASE_ADDR_WAMI_STEEP_DESCENT3
#define MEM_BASE_ADDR_WAMI_HESSIAN3            MEM_BASE_ADDR_WAMI_HESSIAN3
#define MEM_BASE_ADDR_WAMI_INV3                MEM_BASE_ADDR_WAMI_INV3
#define MEM_BASE_ADDR_WAMI_SD_UPDATE3          MEM_BASE_ADDR_WAMI_SD_UPDATE3
#define MEM_BASE_ADDR_WAMI_MULT3               MEM_BASE_ADDR_WAMI_MULT3
#define MEM_BASE_ADDR_WAMI_RESHAPE3            MEM_BASE_ADDR_WAMI_RESHAPE3
#define MEM_BASE_ADDR_WAMI_ADD3                MEM_BASE_ADDR_WAMI_ADD3
#define MEM_BASE_ADDR_WAMI_WARP_IWXP3          MEM_BASE_ADDR_WAMI_WARP_IWXP3
#define MEM_ONE_IMAGE_SIZE
*/

struct wami_homo_stratus_access {
    struct esp_access esp;

    unsigned int wami_src_dst_offset_0;
    unsigned int wami_num_img;
    unsigned int wami_num_col;
    unsigned int wami_num_row;
    unsigned int wami_pad;
    unsigned int wami_kern_id;
    unsigned int wami_batch;
    unsigned int wami_src_dst_offset_1;
    unsigned int wami_src_dst_offset_2;
    unsigned int wami_src_dst_offset_3;
    unsigned int wami_src_dst_offset_4;

    unsigned int src_offset; // Input offset (bytes) used for P2P setup
    unsigned int dst_offset; // Output offset (bytes) used for P2P setup
};

#define WAMI_HOMO_STRATUS_IOC_ACCESS            _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_DEBAYER3_STRATUS_IOC_ACCESS       _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS     _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_GRADIENT3_STRATUS_IOC_ACCESS      _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_WARP_IMG3_STRATUS_IOC_ACCESS      _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_SUB3_STRATUS_IOC_ACCESS           _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_WARP_X3_STRATUS_IOC_ACCESS        _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_WARP_Y3_STRATUS_IOC_ACCESS        _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_STEEP_DESCENT3_STRATUS_IOC_ACCESS _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_HESSIAN3_STRATUS_IOC_ACCESS       _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_INV3_STRATUS_IOC_ACCESS           _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_SD_UPDATE3_STRATUS_IOC_ACCESS     _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_MULT3_STRATUS_IOC_ACCESS          _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_RESHAPE3_STRATUS_IOC_ACCESS       _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_ADD3_STRATUS_IOC_ACCESS           _IOW('S', 0, struct wami_homo_stratus_access)
#define WAMI_WARP_IWXP3_STRATUS_IOC_ACCESS     _IOW('S', 0, struct wami_homo_stratus_access)

unsigned long long time_s;

struct timespec t_sw_1, t_sw_2;
struct timespec t_test_indep_mem_1, t_test_indep_mem_2;
struct timespec t_test_p2p_grayscale3_1, t_test_p2p_grayscale3_2;
struct timespec t_test_p2p_reshape3_1, t_test_p2p_reshape3_2;

struct timespec t_debayer_1, t_debayer_2;
struct timespec t_grayscale_1, t_grayscale_2;
struct timespec t_gradient_1, t_gradient_2;
struct timespec t_warp_img_1, t_warp_img_2;
struct timespec t_subtract_1, t_subtract_2;
struct timespec t_warp_x_1, t_warp_x_2;
struct timespec t_warp_y_1, t_warp_y_2;
struct timespec t_steep_descent_1, t_steep_descent_2;
struct timespec t_hessian_1, t_hessian_2;
struct timespec t_inv_1, t_inv_2;
struct timespec t_sd_update_1, t_sd_update_2;
struct timespec t_mult_1, t_mult_2;
struct timespec t_reshape_1, t_reshape_2;
struct timespec t_add_1, t_add_2;
struct timespec t_warp_iwxp_1, t_warp_iwxp_2;
struct timespec t_gradient_dummy_1, t_gradient_dummy_2;

// for batch tests
uint32_t indep_batch_num;

uint32_t src_dst_offset_0; // Data base address in main memory
uint32_t num_img;          // Number of images
uint32_t num_col;          // Number of columns for each image
uint32_t num_row;          // Number of rows for each image
uint32_t pad;              // Some accelerators may require padding information
uint8_t  kern_id;
uint32_t batch = 1;
uint32_t src_dst_offset_1;
uint32_t src_dst_offset_2;
uint32_t src_dst_offset_3;
uint32_t src_dst_offset_4;

// Gold images
rgb_pixel_t *golden_rgb_imgs;
flt_pixel_t *golden_gs_imgs;
flt_pixel_t *golden_grad_x;
flt_pixel_t *golden_grad_y;
flt_pixel_t *golden_warp_img;
flt_pixel_t *affine_warp;
flt_pixel_t *golden_sub;
flt_pixel_t *golden_warp_dx;
flt_pixel_t *golden_warp_dy;
flt_pixel_t *golden_warp_iwxp;
flt_pixel_t *golden_I_steepest;
flt_pixel_t *golden_hess;
flt_pixel_t *golden_hess_inv;
flt_pixel_t *igj_workspace;
flt_pixel_t *golden_delta_p;
flt_pixel_t *golden_sd_delta_p;
flt_pixel_t *golden_sd_delta_p_nxt;
flt_pixel_t *golden_affine_warp;
uint8_t     *golden_foreground;
uint16_t    *golden_gmm_img;

// Input images
uint32_t     nTestImages;
uint16_t    *u16tmpimg;
uint16_t    *images;
uint8_t     *results;
rgb_pixel_t *rgbtmpimg;
uint32_t     nRows;
uint32_t     nCols;
uint32_t     M;
uint32_t     N;

// Warp and registration - k2 lucas kanade
flt_pixel_t *imgs;
flt_pixel_t *grad_x;
flt_pixel_t *grad_y;
flt_pixel_t *IWxp_; // TODO
flt_pixel_t *sub;
flt_pixel_t *nabla_Ix;
flt_pixel_t *nabla_Iy;
flt_pixel_t *I_steepest;
flt_pixel_t *hess;
flt_pixel_t *hess_inv;
flt_pixel_t *sd_delta_p;
flt_pixel_t *delta_p;
flt_pixel_t *sd_delta_p_nxt;
flt_pixel_t *affine_warp_nxt;
flt_pixel_t *warp_iwxp;

flt_pixel_t *tmplt;
flt_pixel_t *IWxp;
flt_pixel_t *swap;

int      i, a, f, errors;
uint32_t nModels, padding_in;

// GMM storage
uint16_t *gmm_img;
float    *mu, *mu_dump;
float    *sigma, *sigma_dump;
float    *weight, *weight_dump;
uint8_t  *foreground;

// Affine warp parameter set p
float warp_p[6];

#endif /* _WAMI_HOMO_STRATUS_H_ */
