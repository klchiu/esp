/* Copyright (c) 2011-2022 Columbia University, System Level Design Group */
/* SPDX-License-Identifier: Apache-2.0 */

#ifndef __WAMI_BARE_H__
#define __WAMI_BARE_H__

#include <stdio.h>
#ifndef __riscv
    #include <stdlib.h>
#endif

#include <esp_accelerator.h>
#include <esp_probe.h>
#include <fixed_point.h>

#include "wami_C_data.h"

#define __FIXED
#define BITWIDTH 32
// typedef int token_t;
typedef uint64_t token_t;

#define fx2float fixed32_to_float
#define float2fx float_to_fixed32
#define FX_IL    16

typedef float native_t;

static unsigned DMA_WORD_PER_BEAT(unsigned _st) { return (sizeof(void *) / _st); }

#define MAX_PRINTED_ERRORS  10
#define REL_ERROR_THRESHOLD 0.01

/* Size of the contiguous chunks for scatter/gather */
#define CHUNK_SHIFT 20
#define CHUNK_SIZE  BIT(CHUNK_SHIFT)
#define NCHUNK(_sz) ((_sz % CHUNK_SIZE == 0) ? (_sz / CHUNK_SIZE) : (_sz / CHUNK_SIZE) + 1)

/* User defined registers */
#define WAMI_SRC_DST_OFFSET_0_REG 0x40
#define WAMI_NUM_IMG_REG          0x44
#define WAMI_NUM_COL_REG          0x48
#define WAMI_NUM_ROW_REG          0x4C
#define WAMI_PAD_REG              0x50
#define WAMI_KERN_ID_REG          0x54
#define WAMI_BATCH_REG            0x58
#define WAMI_SRC_DST_OFFSET_1_REG 0x5C
#define WAMI_SRC_DST_OFFSET_2_REG 0x60
#define WAMI_SRC_DST_OFFSET_3_REG 0x64
#define WAMI_SRC_DST_OFFSET_4_REG 0x68

// Kernel IDs
#define DEBAYER_KERN_ID             1
#define GRAYSCALE_KERN_ID           2
#define GRADIENT_KERN_ID            3
#define WARP_KERN_ID                4
#define SUBTRACT_KERN_ID            5
#define STEEPEST_DESCENT_KERN_ID    6
#define HESSIAN_KERN_ID             7
#define INVERT_GAUSS_JORDAN_KERN_ID 8
#define SD_UPDATE_KERN_ID           9
#define MULT_KERN_ID                10
#define RESHAPE_KERN_ID             11
#define ADD_KERN_ID                 12

// Device IDs
#define SLD_DEBAYER3       0x071
#define SLD_GRAYSCALE3     0x072
#define SLD_GRADIENT3      0x073
#define SLD_WARP3          0x074
#define SLD_SUB3           0x075
#define SLD_STEEP_DESCENT3 0x076
#define SLD_HESSIAN3       0x077
#define SLD_INV3           0x078
#define SLD_SD_UPDATE3     0x079
#define SLD_MULT3          0x07A
#define SLD_RESHAPE3       0x07B
#define SLD_ADD3           0x07C

// Device Names
#define DEV_NAME_DEBAYER3       "sld,wami_debayer3_stratus"
#define DEV_NAME_GRAYSCALE3     "sld,wami_grayscale3_stratus"
#define DEV_NAME_GRADIENT3      "sld,wami_gradient3_stratus"
#define DEV_NAME_WARP3          "sld,wami_warp3_stratus"
#define DEV_NAME_SUB3           "sld,wami_sub3_stratus"
#define DEV_NAME_STEEP_DESCENT3 "sld,wami_steep_descent3_stratus"
#define DEV_NAME_HESSIAN3       "sld,wami_hessian3_stratus"
#define DEV_NAME_INV3           "sld,wami_inv3_stratus"
#define DEV_NAME_SD_UPDATE3     "sld,wami_sd_update3_stratus"
#define DEV_NAME_MULT3          "sld,wami_mult3_stratus"
#define DEV_NAME_RESHAPE3       "sld,wami_reshape3_stratus"
#define DEV_NAME_ADD3           "sld,wami_add3_stratus"

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

// Gold images
rgb_pixel_t *golden_rgb_imgs;
// uint16_t *   golden_rgb_imgs;
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
uint8_t *    golden_foreground;
uint16_t *   golden_gmm_img;

// Input images
uint32_t     nTestImages;
uint16_t *   u16tmpimg;
uint16_t *   images;
uint8_t *    results;
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
float *   mu, *mu_dump;
float *   sigma, *sigma_dump;
float *   weight, *weight_dump;
uint8_t * foreground;

// Affine warp parameter set p
float warp_p[6];

// Variables for the accelerators
struct esp_device *espdevs_debayer3;
struct esp_device *dev_debayer3;
unsigned **        ptable_debayer3;
token_t *          mem_debayer3;
native_t *         sw_buf_debayer3;

struct esp_device *espdevs_grayscale3;
struct esp_device *dev_grayscale3;
unsigned **        ptable_grayscale3;
token_t *          mem_grayscale3;
native_t *         sw_buf_grayscale3;

struct esp_device *espdevs_gradient3;
struct esp_device *dev_gradient3;
unsigned **        ptable_gradient3;
token_t *          mem_gradient3;
native_t *         sw_buf_gradient3;

struct esp_device *espdevs_warp_img3;
struct esp_device *dev_warp_img3;
unsigned **        ptable_warp_img3;
token_t *          mem_warp_img3;
native_t *         sw_buf_warp_img3;

struct esp_device *espdevs_sub3;
struct esp_device *dev_sub3;
unsigned **        ptable_sub3;
token_t *          mem_sub3;
native_t *         sw_buf_sub3;

struct esp_device *espdevs_warp_dx3;
struct esp_device *dev_warp_dx3;
unsigned **        ptable_warp_dx3;
token_t *          mem_warp_dx3;
native_t *         sw_buf_warp_dx3;

struct esp_device *espdevs_warp_dy3;
struct esp_device *dev_warp_dy3;
unsigned **        ptable_warp_dy3;
token_t *          mem_warp_dy3;
native_t *         sw_buf_warp_dy3;

struct esp_device *espdevs_steep_descent3;
struct esp_device *dev_steep_descent3;
unsigned **        ptable_steep_descent3;
token_t *          mem_steep_descent3;
native_t *         sw_buf_steep_descent3;

struct esp_device *espdevs_hessian3;
struct esp_device *dev_hessian3;
unsigned **        ptable_hessian3;
token_t *          mem_hessian3;
native_t *         sw_buf_hessian3;

struct esp_device *espdevs_inv3;
struct esp_device *dev_inv3;
unsigned **        ptable_inv3;
token_t *          mem_inv3;
native_t *         sw_buf_inv3;

struct esp_device *espdevs_sd_update3;
struct esp_device *dev_sd_update3;
unsigned **        ptable_sd_update3;
token_t *          mem_sd_update3;
native_t *         sw_buf_sd_update3;

struct esp_device *espdevs_mult3;
struct esp_device *dev_mult3;
unsigned **        ptable_mult3;
token_t *          mem_mult3;
native_t *         sw_buf_mult3;

struct esp_device *espdevs_reshape3;
struct esp_device *dev_reshape3;
unsigned **        ptable_reshape3;
token_t *          mem_reshape3;
native_t *         sw_buf_reshape3;

struct esp_device *espdevs_add3;
struct esp_device *dev_add3;
unsigned **        ptable_add3;
token_t *          mem_add3;
native_t *         sw_buf_add3;

struct esp_device *espdevs_warp_iwxp3;
struct esp_device *dev_warp_iwxp3;
unsigned **        ptable_warp_iwxp3;
token_t *          mem_warp_iwxp3;
native_t *         sw_buf_warp_iwxp3;

#endif // __WAMI_BARE_H__
