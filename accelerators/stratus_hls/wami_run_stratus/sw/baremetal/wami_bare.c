// Copyright (c) 2011-2022 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
/**
 * Baremetal device driver for WAMI
 *
 * Select Scatter-Gather in ESP configuration
 */

#include <stdio.h>
#ifndef __riscv
    #include <stdlib.h>
#endif

#include <monitors.h>

#include "wami_bare.h"
#include "wami_C_data.h"
#include "wami_config_tb.h"

#include "wami_debayer_pv.h"
#include "wami_gmm_pv.h"
#include "wami_gradient_pv.h"
#include "wami_grayscale_pv.h"
#include "wami_lucas_kanade_pv.h"
#include "wami_matrix_ops.h"

static unsigned mem_size_debayer3;
static unsigned mem_size_grayscale3;
static unsigned mem_size_gradient3;
static unsigned mem_size_warp_img3;
static unsigned mem_size_sub3;
static unsigned mem_size_warp_dx3;
static unsigned mem_size_warp_dy3;
static unsigned mem_size_steep_descent3;
static unsigned mem_size_hessian3;
static unsigned mem_size_inv3;
static unsigned mem_size_sd_update3;
static unsigned mem_size_mult3;
static unsigned mem_size_reshape3;
static unsigned mem_size_add3;
static unsigned mem_size_warp_iwxp3;

const float ERROR_COUNT_TH = 0.001;
// const float CLOCK_PERIOD   = 0.00000002; // seconds, vc707 at 50MHz
const float CLOCK_PERIOD = 0.0000000128205; // seconds, agatha at 78MHz

static inline uint64_t get_counter()
{
#ifdef _riscv
    uint64_t counter;
    asm volatile("li t0, 0;"
                 "csrr t0, mcycle;"
                 "mv %0, t0"
                 : "=r"(counter)
                 :
                 : "t0");
    return counter;
#else
    return 0;
#endif
}

static void malloc_arrays()
{
    images = aligned_malloc(sizeof(uint16_t) * nRows * nCols);

    uint32_t img_num_pxls   = (M) * (N);
    uint32_t out_num_pxls   = img_num_pxls;
    uint32_t train_num_pxls = nModels * img_num_pxls;

    rgbtmpimg       = (rgb_pixel_t *)aligned_malloc(sizeof(rgb_pixel_t) * out_num_pxls);
    imgs            = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    grad_x          = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    grad_y          = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    IWxp_           = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    sub             = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    nabla_Ix        = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    nabla_Iy        = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    I_steepest      = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 6 * out_num_pxls);
    hess            = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 36);
    hess_inv        = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 36);
    sd_delta_p      = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 6);
    delta_p         = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 6);
    sd_delta_p_nxt  = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 6);
    affine_warp_nxt = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * 6);
    warp_iwxp       = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * out_num_pxls);
    foreground      = (uint8_t *)aligned_malloc(sizeof(uint8_t) * out_num_pxls);
    gmm_img         = (uint16_t *)aligned_malloc(sizeof(uint16_t) * out_num_pxls);
    mu_dump         = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * train_num_pxls);
    sigma_dump      = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * train_num_pxls);
    weight_dump     = (flt_pixel_t *)aligned_malloc(sizeof(flt_pixel_t) * train_num_pxls);

    // golden_rgb_imgs       = aligned_malloc(DEBAYER_TOTAL_OUTPUT_NUM_PXL * sizeof(rgb_pixel_t));
    golden_rgb_imgs       = aligned_malloc(DEBAYER_TOTAL_OUTPUT_NUM_PXL * sizeof(uint16_t) * 3);
    golden_gs_imgs        = aligned_malloc(GRAYSCALE_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_grad_x         = aligned_malloc(GRADIENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_grad_y         = aligned_malloc(GRADIENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_img       = aligned_malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    affine_warp           = aligned_malloc(6 * sizeof(flt_pixel_t));
    golden_sub            = aligned_malloc(SUBTRACT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    IWxp                  = aligned_malloc(SUBTRACT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_dx        = aligned_malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_dy        = aligned_malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_iwxp      = aligned_malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_I_steepest     = aligned_malloc(STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_hess           = aligned_malloc(HESSIAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_hess_inv       = aligned_malloc(INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    igj_workspace         = aligned_malloc(INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_sd_delta_p     = aligned_malloc(SD_UPDATE_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_delta_p        = aligned_malloc(MULT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_sd_delta_p_nxt = aligned_malloc(MULT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_affine_warp    = aligned_malloc(ADD_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_foreground     = aligned_malloc(CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL * sizeof(uint8_t));
    golden_gmm_img        = aligned_malloc(CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL * sizeof(uint16_t));
}

static void free_arrays()
{
    aligned_free(IWxp);
    aligned_free(mu);
    aligned_free(sigma);
    aligned_free(weight);
    aligned_free(images);
    aligned_free(results);

    aligned_free(rgbtmpimg);
    aligned_free(imgs);
    aligned_free(grad_x);
    aligned_free(grad_y);
    aligned_free(IWxp_);
    aligned_free(sub);
    aligned_free(nabla_Ix);
    aligned_free(nabla_Iy);
    aligned_free(I_steepest);
    aligned_free(hess);
    aligned_free(hess_inv);
    aligned_free(sd_delta_p);
    aligned_free(delta_p);
    aligned_free(sd_delta_p_nxt);
    aligned_free(affine_warp_nxt);
    aligned_free(warp_iwxp);
    aligned_free(foreground);
    aligned_free(gmm_img);
    aligned_free(mu_dump);
    aligned_free(sigma_dump);
    aligned_free(weight_dump);

    aligned_free(golden_rgb_imgs);
    aligned_free(golden_gs_imgs);
    aligned_free(golden_grad_x);
    aligned_free(golden_grad_y);
    aligned_free(golden_warp_img);
    aligned_free(affine_warp);
    aligned_free(golden_sub);
    aligned_free(golden_warp_dx);
    aligned_free(golden_warp_dy);
    aligned_free(golden_warp_iwxp);
    aligned_free(golden_I_steepest);
    aligned_free(golden_hess);
    aligned_free(golden_hess_inv);
    aligned_free(igj_workspace);
    aligned_free(golden_sd_delta_p);
    aligned_free(golden_delta_p);
    aligned_free(golden_sd_delta_p_nxt);
    aligned_free(golden_affine_warp);
    aligned_free(golden_foreground);
    aligned_free(golden_gmm_img);
}

static void init_buf_from_header()
{
    printf("-- init_buf_from_header()\n");
    nRows      = 132;
    nCols      = 132;
    M          = 128;
    N          = 128;
    padding_in = 2;

    //*IWxp = 0.0;
    nModels = 5;
    //*sigma = 0.0;
    //*weight = 0.0;
    nTestImages = 17;
#include "fish_bayer.h"
#include "fish_IWxp.h"
}

/*
static void validate_buf(token_t *buf)
{
    int x, y;
    // -- validate buffer
    int error_count = 0;
#if 1
    // validate output for debayer3
    for (x = 0; x < M * N; x++) {
        // y              = MEM_BASE_ADDR_WAMI_DEBAYER3 + x;
        y              = MEM_BASE_ADDR_WAMI_DEBAYER3 + x;
        uint64_t pixel = buf[y];
        uint16_t r     = (pixel >> 32) & 0xFFFF;
        uint16_t g     = (pixel >> 16) & 0xFFFF;
        uint16_t b     = (pixel >> 0) & 0xFFFF;

        if (golden_rgb_imgs[x].r != r || golden_rgb_imgs[x].g != g || golden_rgb_imgs[x].b != b) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout(%d, %d, %d)\tgold(%d, %d, %d)\n", x, r, g, b, golden_rgb_imgs[x].r,
                   golden_rgb_imgs[x].g, golden_rgb_imgs[x].b);
        }
    }
    if (error_count > 0) {
        printf("wami_debayer3: error: %d\n", error_count);
    } else {
        printf("wami_debayer3: Correct!!!\n");
    }

    // validate output for grayscale3
    error_count = 0;
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + x;
        double pixel = fx2float(buf[y], FX_IL_GRAYSCALE3);

        if (abs(pixel - golden_gs_imgs[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_gs_imgs[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_grayscale3: error: %d\n", error_count);
    } else {
        printf("wami_grayscale3: Correct!!!\n");
    }

    // validate output for gradient3
    error_count = 0;
    for (x = 0; x < M * N; x++) { // grad_x
        y           = MEM_BASE_ADDR_WAMI_GRADIENT3 + x;
        float pixel = fx2float(buf[y], FX_IL_GRADIENT3);

        if (abs(pixel - golden_grad_x[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_grad_x[x]);
        }
    }
    for (x = 0; x < M * N; x++) { // grad_y
        y           = MEM_BASE_ADDR_WAMI_GRADIENT3 + M * N + x;
        float pixel = fx2float(buf[y], FX_IL_GRADIENT3);

        if (abs(pixel - golden_grad_y[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_grad_y[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_gradient3: error: %d\n", error_count);
    } else {
        printf("wami_gradient3: Correct!!!\n");
    }

    // validate output for warp-img
    error_count = 0;
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_WARP_IMG3 + x;
        float pixel = fx2float(buf[y], FX_IL_WARP3);

        if (abs(pixel - golden_warp_img[x]) > 1) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_warp_img[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_warp-img3: error: %d\n", error_count);
    } else {
        printf("wami_warp-img3: Correct!!!\n");
    }

    // validate output for sub3
    error_count = 0;
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_SUB3 + x;
        float pixel = fx2float(buf[y], FX_IL_SUB3);

        if (abs(pixel - golden_sub[x]) > 1) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_sub[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_sub3: error: %d\n", error_count);
    } else {
        printf("wami_sub3: Correct!!!\n");
    }
    // ------------------------------------------
    #if 0
    printf("------------- wami_sub3: debug - input 1!!!\n");
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_SUB3 + M * N + x;
        float pixel = fx2float(buf[y], FX_IL_SUB3);

        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_sub[x]);
        }
    }
    printf("------------- wami_sub3: debug - input 2!!!\n");
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_SUB3 + M * N * 2 + x;
        float pixel = fx2float(buf[y], FX_IL_SUB3);

        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_sub[x]);
        }
    }
    printf("------------- wami_sub3: debug!!!\n");
    #endif
    // -------------------------------------

    // validate output for warp-x
    error_count = 0;
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_WARP_X3 + x;
        float pixel = fx2float(buf[y], FX_IL_WARP3);

        if (abs(pixel - golden_warp_dx[x]) > 1) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_warp_dx[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_warp-x3: error: %d\n", error_count);
    } else {
        printf("wami_warp-x3: Correct!!!\n");
    }

    // validate output for warp-y
    error_count = 0;
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_WARP_Y3 + x;
        float pixel = fx2float(buf[y], FX_IL_WARP3);

        if (abs(pixel - golden_warp_dy[x]) > 1) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_warp_dy[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_warp-y3: error: %d\n", error_count);
    } else {
        printf("wami_warp-y3: Correct!!!\n");
    }
#endif
    // validate output for steep_descent3
    error_count = 0;
    for (x = 0; x < 6 * M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + x;
        float pixel = fx2float(buf[y], FX_IL_STEEP_DESCENT3);

        if (abs(pixel - golden_I_steepest[x]) > 1) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_I_steepest[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_steep_descent3: error: %d\n", error_count);
    } else {
        printf("wami_steep_descent3: Correct!!!\n");
    }
#if 1
    // validate output for hessian3
    error_count = 0;
    for (x = 0; x < 36; x++) {
        y                = MEM_BASE_ADDR_WAMI_HESSIAN3 + x;
        float pixel      = fx2float(buf[y], FX_IL_HESSIAN3);
        float error_rate = abs((pixel - golden_hess[x]) / golden_hess[x]);
        if (error_rate > 0.01) {
            error_count++;
        }
        if (x < 6) {
            // printf("i = %d\tout= %f\tgold= %f\terror_rate= %f\n", x, pixel,
            // golden_hess[x], error_rate);
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_hess[x]);
        }
    }
    // printf("MEM_BASE_ADDR_WAMI_HESSIAN3 = %d\n", MEM_BASE_ADDR_WAMI_HESSIAN3);
    if (error_count > 0) {
        printf("wami_hessian3: error: %d\n", error_count);
    } else {
        printf("wami_hessian3: Correct!!!\n");
    }
#endif
    // validate output for inv3
    error_count = 0;
    for (x = 0; x < 36; x++) {
        y           = MEM_BASE_ADDR_WAMI_INV3 + x;
        float pixel = fx2float(buf[y], FX_IL_INV3);

        if (abs(pixel - golden_hess_inv[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_hess_inv[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_inv3: error: %d\n", error_count);
    } else {
        printf("wami_inv3: Correct!!!\n");
    }

    // validate output for sd_update3
    error_count = 0;
    for (x = 0; x < 6; x++) {
        y           = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + x;
        float pixel = fx2float(buf[y], FX_IL_SD_UPDATE3);

        if (abs(pixel - golden_sd_delta_p[x]) > 1000) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_sd_delta_p[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_sd_update3: error: %d\n", error_count);
    } else {
        printf("wami_sd_update3: Correct!!!\n");
    }

    // validate output for mult3
    error_count = 0;
    for (x = 0; x < 6; x++) {
        y           = MEM_BASE_ADDR_WAMI_MULT3 + x;
        float pixel = fx2float(buf[y], FX_IL_MULT3);

        if (abs(pixel - golden_delta_p[x]) > 0.02) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_delta_p[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_mult3: error: %d\n", error_count);
    } else {
        printf("wami_mult3: Correct!!!\n");
    }

    // validate output for reshape3
    error_count = 0;
    for (x = 0; x < 6; x++) {
        y           = MEM_BASE_ADDR_WAMI_RESHAPE3 + x;
        float pixel = fx2float(buf[y], FX_IL_RESHAPE3);

        if (abs(pixel - golden_sd_delta_p_nxt[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_sd_delta_p_nxt[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_reshape3: error: %d\n", error_count);
    } else {
        printf("wami_reshape3: Correct!!!\n");
    }

    // validate output for add3
    error_count = 0;
    for (x = 0; x < 6; x++) {
        y = MEM_BASE_ADDR_WAMI_ADD3 + x;
        // y = MEM_BASE_ADDR_WAMI_ADD3 + 12 + x;
        float pixel = fx2float(buf[y], FX_IL_ADD3);

        if (abs(pixel - golden_affine_warp[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_affine_warp[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_add3: error: %d\n", error_count);
    } else {
        printf("wami_add3: Correct!!!\n");
    }
    // ------------------------------------------
#if 0
    printf("------------- wami_add3: debug - input 1!!!\n");
    for (x = 0; x < 6; x++) {
        y           = MEM_BASE_ADDR_WAMI_ADD3 + 6 + x;
        float pixel = fx2float(buf[y], FX_IL_SUB3);

        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_affine_warp[x]);
        }
    }
    printf("------------- wami_add3: debug - input 2!!!\n");
    for (x = 0; x < 6; x++) {
        y           = MEM_BASE_ADDR_WAMI_ADD3 + 6 * 2 + x;
        float pixel = fx2float(buf[y], FX_IL_SUB3);

        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_affine_warp[x]);
        }
    }
    printf("------------- wami_add3: debug!!!\n");
#endif
    // -------------------------------------

#if 1
    // validate output for warp-iwxp
    error_count = 0;
    for (x = 0; x < M * N; x++) {
        y           = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + x;
        float pixel = fx2float(buf[y], FX_IL_WARP3);

        if (abs(pixel - golden_warp_iwxp[x]) > 1) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_warp_iwxp[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_warp-iwxp3: error: %d\n", error_count);
    } else {
        printf("wami_warp-iwxp3: Correct!!!\n");
    }

    // validate output for gradient3 dummy
    error_count = 0;
    for (x = 0; x < M * N; x++) { // grad_x
        y           = MEM_BASE_ADDR_WAMI_GRADIENT3 + x;
        float pixel = fx2float(buf[y], FX_IL_GRADIENT3);

        if (abs(pixel - golden_warp_iwxp[x]) > 0.001) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_warp_iwxp[x]);
        }
    }
    if (error_count > 0) {
        printf("wami_gradient3_dummy: error: %d\n", error_count);
    } else {
        printf("wami_gradient3_dummy: Correct!!!\n");
    }
#endif
}
*/

void run_pv(int batch)
{
    // printf("------- run_pv, start. test_batch: %d\n", test_batch);
    int i, x;

    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    // Debayer - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * nRows * nCols;
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
    }
    // printf("Debayer golden output succcesfully computed.\n");

    // Grayscale - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD, nCols - 2 * PAD);
    }
    // printf("Grayscale golden output succcesfully computed.\n");

    // Gradient - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                     golden_grad_y + out_offset);
    }
    // printf("Gradient golden output succcesfully computed.\n");

    // Warp-img - compute golden outputs
    for (i = 0; i < 6; i++)
        affine_warp[i] = 0.0;
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_img + out_offset);
    }
    // printf("Warp-img golden output succcesfully computed.\n");

    // Subtract - compute golden outputs
    for (img = 0; img < batch; img++) {
        //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

        __subtract(IWxp, golden_warp_img + out_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
        //__subtract(golden_warp_img, golden_warp_img + out_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_sub +
        // out_offset);
    }
    // printf("Subtract golden output succcesfully computed.\n");

    // Warp-dx - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dx + out_offset);
    }
    // printf("Warp-dx golden output succcesfully computed.\n");

    // Warp-dy - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dy + out_offset);
    }
    // printf("Warp-dy golden output succcesfully computed.\n");

    // Steepest descent - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                           golden_I_steepest + out_offset);
    }
    // printf("Steepest descent golden output succcesfully computed.\n");

    // Hessian - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
    }
    // printf("Hessian golden output succcesfully computed.\n");

    // Invert Gauss Jordan - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6, golden_hess_inv + out_offset);
    }
    // printf("Invert Gauss Jordan golden output succcesfully computed.\n");

    // SD update - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6;
        __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD, nCols - 2 * PAD,
                    golden_sd_delta_p + out_offset);
    }
    // printf("SD update golden output succcesfully computed.\n");

    // Mult - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset_1 = img * 36;
        in_offset_2 = img * 6;
        out_offset  = img * 6;
        __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6, golden_delta_p + out_offset);
    }
    // printf("Mult golden output succcesfully computed.\n");

    // Reshape - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
    }
    // printf("Reshape golden output succcesfully computed.\n");

    // Add - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
    }
    // printf("Add golden output succcesfully computed.\n");

    // Warp-iwxp - compute golden outputs
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                     golden_warp_iwxp + out_offset);
    }
    // printf("Warp-dy golden output succcesfully computed.\n");


    printf("can't print??? %f\n", 0.3);

    for (i = 0; i < 10; i++) {
        float xx = 32.662;
        printf("golden_warp_iwxp[%d] = %lf, %f, %d\n", i, golden_warp_iwxp[i], xx, (int)golden_warp_iwxp[i]);
    }

    // Change detection - compute golden outputs
    // printf("Change detection output: # %d pixels\n", CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);
    // cast from float to usigned short (16 bits)
    // for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
    //     golden_gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
    // }
    // for (img = 0; img < batch; img++) {
    //     in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
    //     out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
    //     __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
    //                golden_foreground + out_offset, golden_gmm_img + in_offset);
    // }
    // printf("Change detection output succcesfully computed.\n");

    printf("------- run_pv, finish\n");
    printf("-------------------------------------------------------\n");
}

int func_probe(struct esp_device *espdevs_x, struct esp_device *dev_x, const char *acc_name, unsigned int mem_size,
               unsigned SLD_X, const char *ACC_NAME_X)
{
    // [kuanlin]: this func is not working for some reason, need debugging

    printf("  Initialize %s app...\n", acc_name);
    int ndev;

    ndev = probe(&espdevs_x, VENDOR_SLD, SLD_X, ACC_NAME_X);
    if (ndev == 0) {
        printf("%s not found\n", acc_name);
        return 2;
    }
    dev_x = &espdevs_x[0];
    // Check DMA capabilities
    if (ioread32(dev_x, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 3;
    }
    if (ioread32(dev_x, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 4;
    }
}

int main(int argc, char *argv[])
{
    printf("  -----------------------------\n");
    printf("  --- Start baremetal test ----\n");
    printf("  -----------------------------\n");
    int      i;
    int      n;
    int      ndev;
    unsigned done;
    unsigned errors    = 0;
    unsigned coherence = ACC_COH_NONE;
    uint64_t time_acc_start, time_acc_stop;
    uint64_t time_cpu_start, time_cpu_stop;
    uint64_t time_acc_elapsed, time_cpu_elapsed;

    esp_monitor_args_t mon_args;
    const int          CPU_TILE_IDX = 1;
    mon_args.read_mode              = ESP_MON_READ_SINGLE;
    mon_args.tile_index             = CPU_TILE_IDX;
    mon_args.mon_index              = MON_DVFS_BASE_INDEX + 3;
    unsigned int cycles_start, cycles_end, cycles_diff;

    mem_size_debayer3       = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_GRAYSCALE3 - MEM_BASE_ADDR_WAMI_DEBAYER3);
    mem_size_grayscale3     = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_GRADIENT3 - MEM_BASE_ADDR_WAMI_GRAYSCALE3);
    mem_size_gradient3      = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_WARP_IMG3 - MEM_BASE_ADDR_WAMI_GRADIENT3);
    mem_size_warp_img3      = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_SUB3 - MEM_BASE_ADDR_WAMI_WARP_IMG3);
    mem_size_sub3           = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_WARP_X3 - MEM_BASE_ADDR_WAMI_SUB3);
    mem_size_warp_dx3       = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_WARP_Y3 - MEM_BASE_ADDR_WAMI_WARP_X3);
    mem_size_warp_dy3       = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 - MEM_BASE_ADDR_WAMI_WARP_Y3);
    mem_size_steep_descent3 = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_HESSIAN3 - MEM_BASE_ADDR_WAMI_STEEP_DESCENT3);
    mem_size_hessian3       = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_INV3 - MEM_BASE_ADDR_WAMI_HESSIAN3);
    mem_size_inv3           = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_SD_UPDATE3 - MEM_BASE_ADDR_WAMI_INV3);
    mem_size_sd_update3     = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_MULT3 - MEM_BASE_ADDR_WAMI_SD_UPDATE3);
    mem_size_mult3          = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_RESHAPE3 - MEM_BASE_ADDR_WAMI_MULT3);
    mem_size_reshape3       = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_ADD3 - MEM_BASE_ADDR_WAMI_RESHAPE3);
    mem_size_add3           = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_WARP_IWXP3 - MEM_BASE_ADDR_WAMI_ADD3);
    mem_size_warp_iwxp3     = sizeof(token_t) * (MEM_BASE_ADDR_WAMI_SUB3 - MEM_BASE_ADDR_WAMI_WARP_IMG3);
    // mem_size_warp_iwxp3 = sizeof(token_t) * (MEM_ONE_IMAGE_SIZE - MEM_BASE_ADDR_WAMI_WARP_IWXP3 + 128 * 128 * 2 + 6);

    // Allocate memory
    malloc_arrays();

    //////////////////////
    // Initialize
    init_buf_from_header();

    ///////////////////////////////////
    // Probing
    printf("  Probing...\n");
    // func_probe(espdevs_debayer3, dev_debayer3, "debayer3", mem_size_debayer3, SLD_DEBAYER3, DEV_NAME_DEBAYER3);
    // ---------------------------------------
    printf("  -- Probe debayer3...\n");
    ndev = probe(&espdevs_debayer3, VENDOR_SLD, SLD_DEBAYER3, DEV_NAME_DEBAYER3);
    if (ndev == 0) {
        printf("debayer3 not found\n");
        return 0;
    }
    dev_debayer3 = &espdevs_debayer3[0];
    // Check DMA capabilities
    if (ioread32(dev_debayer3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_debayer3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_debayer3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe grayscale3...\n");
    ndev = probe(&espdevs_grayscale3, VENDOR_SLD, SLD_GRAYSCALE3, DEV_NAME_GRAYSCALE3);
    if (ndev == 0) {
        printf("grayscale3 not found\n");
        return 0;
    }
    dev_grayscale3 = &espdevs_grayscale3[0];
    // Check DMA capabilities
    if (ioread32(dev_grayscale3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_grayscale3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_grayscale3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe gradient3...\n");
    ndev = probe(&espdevs_gradient3, VENDOR_SLD, SLD_GRADIENT3, DEV_NAME_GRADIENT3);
    if (ndev == 0) {
        printf("gradient3 not found\n");
        return 0;
    }
    dev_gradient3 = &espdevs_gradient3[0];
    // Check DMA capabilities
    if (ioread32(dev_gradient3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_gradient3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_gradient3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe warp_img3...\n");
    ndev = probe(&espdevs_warp_img3, VENDOR_SLD, SLD_WARP3, DEV_NAME_WARP3);
    if (ndev == 0) {
        printf("warp_img3 not found\n");
        return 0;
    }
    dev_warp_img3 = &espdevs_warp_img3[0];
    // Check DMA capabilities
    if (ioread32(dev_warp_img3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_warp_img3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_warp_img3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe sub3...\n");
    ndev = probe(&espdevs_sub3, VENDOR_SLD, SLD_SUB3, DEV_NAME_SUB3);
    if (ndev == 0) {
        printf("sub3 not found\n");
        return 0;
    }
    dev_sub3 = &espdevs_sub3[0];
    // Check DMA capabilities
    if (ioread32(dev_sub3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_sub3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_sub3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe warp_dx3...\n");
    ndev = probe(&espdevs_warp_dx3, VENDOR_SLD, SLD_WARP3, DEV_NAME_WARP3);
    if (ndev == 0) {
        printf("warp_dx3 not found\n");
        return 0;
    }
    dev_warp_dx3 = &espdevs_warp_dx3[1];
    // Check DMA capabilities
    if (ioread32(dev_warp_dx3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_warp_dx3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_warp_dx3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe warp_dy3...\n");
    ndev = probe(&espdevs_warp_dy3, VENDOR_SLD, SLD_WARP3, DEV_NAME_WARP3);
    if (ndev == 0) {
        printf("warp_dy3 not found\n");
        return 0;
    }
    dev_warp_dy3 = &espdevs_warp_dy3[2];
    // Check DMA capabilities
    if (ioread32(dev_warp_dy3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_warp_dy3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_warp_dy3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe steep_descent3...\n");
    ndev = probe(&espdevs_steep_descent3, VENDOR_SLD, SLD_STEEP_DESCENT3, DEV_NAME_STEEP_DESCENT3);
    if (ndev == 0) {
        printf("steep_descent3 not found\n");
        return 0;
    }
    dev_steep_descent3 = &espdevs_steep_descent3[0];
    // Check DMA capabilities
    if (ioread32(dev_steep_descent3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_steep_descent3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_steep_descent3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe hessian3...\n");
    ndev = probe(&espdevs_hessian3, VENDOR_SLD, SLD_HESSIAN3, DEV_NAME_HESSIAN3);
    if (ndev == 0) {
        printf("hessian3 not found\n");
        return 0;
    }
    dev_hessian3 = &espdevs_hessian3[0];
    // Check DMA capabilities
    if (ioread32(dev_hessian3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_hessian3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_hessian3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe inv3...\n");
    ndev = probe(&espdevs_inv3, VENDOR_SLD, SLD_INV3, DEV_NAME_INV3);
    if (ndev == 0) {
        printf("inv3 not found\n");
        return 0;
    }
    dev_inv3 = &espdevs_inv3[0];
    // Check DMA capabilities
    if (ioread32(dev_inv3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_inv3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_inv3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe sd_update3...\n");
    ndev = probe(&espdevs_sd_update3, VENDOR_SLD, SLD_SD_UPDATE3, DEV_NAME_SD_UPDATE3);
    if (ndev == 0) {
        printf("sd_update3 not found\n");
        return 0;
    }
    dev_sd_update3 = &espdevs_sd_update3[0];
    // Check DMA capabilities
    if (ioread32(dev_sd_update3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_sd_update3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_sd_update3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe mult3...\n");
    ndev = probe(&espdevs_mult3, VENDOR_SLD, SLD_MULT3, DEV_NAME_MULT3);
    if (ndev == 0) {
        printf("mult3 not found\n");
        return 0;
    }
    dev_mult3 = &espdevs_mult3[0];
    // Check DMA capabilities
    if (ioread32(dev_mult3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_mult3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_mult3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe reshape3...\n");
    ndev = probe(&espdevs_reshape3, VENDOR_SLD, SLD_RESHAPE3, DEV_NAME_RESHAPE3);
    if (ndev == 0) {
        printf("reshape3 not found\n");
        return 0;
    }
    dev_reshape3 = &espdevs_reshape3[0];
    // Check DMA capabilities
    if (ioread32(dev_reshape3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_reshape3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_reshape3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe add3...\n");
    ndev = probe(&espdevs_add3, VENDOR_SLD, SLD_ADD3, DEV_NAME_ADD3);
    if (ndev == 0) {
        printf("add3 not found\n");
        return 0;
    }
    dev_add3 = &espdevs_add3[0];
    // Check DMA capabilities
    if (ioread32(dev_add3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_add3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_add3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------
    printf("  -- Probe warp_iwxp3...\n");
    ndev = probe(&espdevs_warp_iwxp3, VENDOR_SLD, SLD_WARP3, DEV_NAME_WARP3);
    if (ndev == 0) {
        printf("warp_iwxp3 not found\n");
        return 0;
    }
    dev_warp_iwxp3 = &espdevs_warp_iwxp3[3];
    // Check DMA capabilities
    if (ioread32(dev_warp_iwxp3, PT_NCHUNK_MAX_REG) == 0) {
        printf("  -> scatter-gather DMA is disabled. Abort.\n");
        return 0;
    }
    if (ioread32(dev_warp_iwxp3, PT_NCHUNK_MAX_REG) < NCHUNK(mem_size_warp_iwxp3)) {
        printf("  -> Not enough TLB entries available. Abort.\n");
        return 0;
    }
    // ---------------------------------------

    ///////////////////////////////////
    // Allocation
    printf("  Allocation...\n");

    // ---------------------------------------
    mem_debayer3 = aligned_malloc(mem_size_debayer3);
    printf(" debayer3 memory buffer base-address = %p\n", mem_debayer3);
    printf(" debayer3 memory buffer size = %d\n", mem_size_debayer3);

    // allocate and populate page table
    ptable_debayer3 = aligned_malloc(NCHUNK(mem_size_debayer3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_debayer3); i++)
        ptable_debayer3[i] = (unsigned *)&mem_debayer3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf(" debayer3 ptable = %p\n", ptable_debayer3);
    printf(" debayer3 nchunk = %lu\n", NCHUNK(mem_size_debayer3));
    // ---------------------------------------
    mem_grayscale3 = aligned_malloc(mem_size_grayscale3);
    printf("  memory buffer base-address = %p\n", mem_grayscale3);
    printf("  memory buffer size = %d\n", mem_size_grayscale3);

    // allocate and populate page table
    ptable_grayscale3 = aligned_malloc(NCHUNK(mem_size_grayscale3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_grayscale3); i++)
        ptable_grayscale3[i] = (unsigned *)&mem_grayscale3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_grayscale3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_grayscale3));
    // ---------------------------------------
    mem_gradient3 = aligned_malloc(mem_size_gradient3);
    printf("  memory buffer base-address = %p\n", mem_gradient3);
    printf("  memory buffer size = %d\n", mem_size_gradient3);

    // allocate and populate page table
    ptable_gradient3 = aligned_malloc(NCHUNK(mem_size_gradient3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_gradient3); i++)
        ptable_gradient3[i] = (unsigned *)&mem_gradient3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_gradient3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_gradient3));
    // ---------------------------------------
    mem_warp_img3 = aligned_malloc(mem_size_warp_img3);
    printf("  memory buffer base-address = %p\n", mem_warp_img3);
    printf("  memory buffer size = %d\n", mem_size_warp_img3);

    // allocate and populate page table
    ptable_warp_img3 = aligned_malloc(NCHUNK(mem_size_warp_img3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_warp_img3); i++)
        ptable_warp_img3[i] = (unsigned *)&mem_warp_img3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_warp_img3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_warp_img3));
    // ---------------------------------------
    mem_sub3 = aligned_malloc(mem_size_sub3);
    printf("  memory buffer base-address = %p\n", mem_sub3);
    printf("  memory buffer size = %d\n", mem_size_sub3);

    // allocate and populate page table
    ptable_sub3 = aligned_malloc(NCHUNK(mem_size_sub3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_sub3); i++)
        ptable_sub3[i] = (unsigned *)&mem_sub3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_sub3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_sub3));
    // ---------------------------------------
    mem_warp_dx3 = aligned_malloc(mem_size_warp_dx3);
    printf("  memory buffer base-address = %p\n", mem_warp_dx3);
    printf("  memory buffer size = %d\n", mem_size_warp_dx3);

    // allocate and populate page table
    ptable_warp_dx3 = aligned_malloc(NCHUNK(mem_size_warp_dx3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_warp_dx3); i++)
        ptable_warp_dx3[i] = (unsigned *)&mem_warp_dx3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_warp_dx3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_warp_dx3));
    // ---------------------------------------
    mem_warp_dy3 = aligned_malloc(mem_size_warp_dy3);
    printf("  memory buffer base-address = %p\n", mem_warp_dy3);
    printf("  memory buffer size = %d\n", mem_size_warp_dy3);

    // allocate and populate page table
    ptable_warp_dy3 = aligned_malloc(NCHUNK(mem_size_warp_dy3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_warp_dy3); i++)
        ptable_warp_dy3[i] = (unsigned *)&mem_warp_dy3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_warp_dy3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_warp_dy3));
    // ---------------------------------------
    mem_steep_descent3 = aligned_malloc(mem_size_steep_descent3);
    printf("  memory buffer base-address = %p\n", mem_steep_descent3);
    printf("  memory buffer size = %d\n", mem_size_steep_descent3);

    // allocate and populate page table
    ptable_steep_descent3 = aligned_malloc(NCHUNK(mem_size_steep_descent3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_steep_descent3); i++)
        ptable_steep_descent3[i] = (unsigned *)&mem_steep_descent3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_steep_descent3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_steep_descent3));
    // ---------------------------------------
    mem_hessian3 = aligned_malloc(mem_size_hessian3);
    printf("  memory buffer base-address = %p\n", mem_hessian3);
    printf("  memory buffer size = %d\n", mem_size_hessian3);

    // allocate and populate page table
    ptable_hessian3 = aligned_malloc(NCHUNK(mem_size_hessian3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_hessian3); i++)
        ptable_hessian3[i] = (unsigned *)&mem_hessian3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_hessian3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_hessian3));
    // ---------------------------------------
    mem_inv3 = aligned_malloc(mem_size_inv3);
    printf("  memory buffer base-address = %p\n", mem_inv3);
    printf("  memory buffer size = %d\n", mem_size_inv3);

    // allocate and populate page table
    ptable_inv3 = aligned_malloc(NCHUNK(mem_size_inv3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_inv3); i++)
        ptable_inv3[i] = (unsigned *)&mem_inv3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_inv3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_inv3));
    // ---------------------------------------
    mem_sd_update3 = aligned_malloc(mem_size_sd_update3);
    printf("  memory buffer base-address = %p\n", mem_sd_update3);
    printf("  memory buffer size = %d\n", mem_size_sd_update3);

    // allocate and populate page table
    ptable_sd_update3 = aligned_malloc(NCHUNK(mem_size_sd_update3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_sd_update3); i++)
        ptable_sd_update3[i] = (unsigned *)&mem_sd_update3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_sd_update3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_sd_update3));
    // ---------------------------------------
    mem_mult3 = aligned_malloc(mem_size_mult3);
    printf("  memory buffer base-address = %p\n", mem_mult3);
    printf("  memory buffer size = %d\n", mem_size_mult3);

    // allocate and populate page table
    ptable_mult3 = aligned_malloc(NCHUNK(mem_size_mult3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_mult3); i++)
        ptable_mult3[i] = (unsigned *)&mem_mult3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_mult3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_mult3));
    // ---------------------------------------
    mem_reshape3 = aligned_malloc(mem_size_reshape3);
    printf("  memory buffer base-address = %p\n", mem_reshape3);
    printf("  memory buffer size = %d\n", mem_size_reshape3);

    // allocate and populate page table
    ptable_reshape3 = aligned_malloc(NCHUNK(mem_size_reshape3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_reshape3); i++)
        ptable_reshape3[i] = (unsigned *)&mem_reshape3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_reshape3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_reshape3));
    // ---------------------------------------
    mem_add3 = aligned_malloc(mem_size_add3);
    printf("  memory buffer base-address = %p\n", mem_add3);
    printf("  memory buffer size = %d\n", mem_size_add3);

    // allocate and populate page table
    ptable_add3 = aligned_malloc(NCHUNK(mem_size_add3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_add3); i++)
        ptable_add3[i] = (unsigned *)&mem_add3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_add3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_add3));
    // ---------------------------------------
    mem_warp_iwxp3 = aligned_malloc(mem_size_warp_iwxp3);
    printf("  memory buffer base-address = %p\n", mem_warp_iwxp3);
    printf("  memory buffer size = %d\n", mem_size_warp_iwxp3);

    // allocate and populate page table
    ptable_warp_iwxp3 = aligned_malloc(NCHUNK(mem_size_warp_iwxp3) * sizeof(unsigned *));
    for (i = 0; i < NCHUNK(mem_size_warp_iwxp3); i++)
        ptable_warp_iwxp3[i] = (unsigned *)&mem_warp_iwxp3[i * (CHUNK_SIZE / sizeof(token_t))];
    printf("  ptable = %p\n", ptable_warp_iwxp3);
    printf("  nchunk = %lu\n", NCHUNK(mem_size_warp_iwxp3));
    // ---------------------------------------

    ///////////////////////////////////
    // Data Initialization
    printf("  Data initialization...\n");
    // [kuanlin]: if init buf here, the pre-config got stuck
    // init_buf_from_header();

    ///////////////////////////////////
    // Pre-configuration
    printf("  Pre-configuration...\n");
    // ---------------------------------------
    iowrite32(dev_debayer3, SELECT_REG, ioread32(dev_debayer3, DEVID_REG));
    iowrite32(dev_debayer3, COHERENCE_REG, coherence);
    iowrite32(dev_debayer3, PT_ADDRESS_REG, (unsigned long)ptable_debayer3);
    iowrite32(dev_debayer3, PT_NCHUNK_REG, NCHUNK(mem_size_debayer3));
    iowrite32(dev_debayer3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_debayer3, SRC_OFFSET_REG, 0);
    iowrite32(dev_debayer3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_grayscale3, SELECT_REG, ioread32(dev_grayscale3, DEVID_REG));
    iowrite32(dev_grayscale3, COHERENCE_REG, coherence);
    iowrite32(dev_grayscale3, PT_ADDRESS_REG, (unsigned long)ptable_grayscale3);
    iowrite32(dev_grayscale3, PT_NCHUNK_REG, NCHUNK(mem_size_grayscale3));
    iowrite32(dev_grayscale3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_grayscale3, SRC_OFFSET_REG, 0);
    iowrite32(dev_grayscale3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_gradient3, SELECT_REG, ioread32(dev_gradient3, DEVID_REG));
    iowrite32(dev_gradient3, COHERENCE_REG, coherence);
    iowrite32(dev_gradient3, PT_ADDRESS_REG, (unsigned long)ptable_gradient3);
    iowrite32(dev_gradient3, PT_NCHUNK_REG, NCHUNK(mem_size_gradient3));
    iowrite32(dev_gradient3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_gradient3, SRC_OFFSET_REG, 0);
    iowrite32(dev_gradient3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_warp_img3, SELECT_REG, ioread32(dev_warp_img3, DEVID_REG));
    iowrite32(dev_warp_img3, COHERENCE_REG, coherence);
    iowrite32(dev_warp_img3, PT_ADDRESS_REG, (unsigned long)ptable_warp_img3);
    iowrite32(dev_warp_img3, PT_NCHUNK_REG, NCHUNK(mem_size_warp_img3));
    iowrite32(dev_warp_img3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_warp_img3, SRC_OFFSET_REG, 0);
    iowrite32(dev_warp_img3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_sub3, SELECT_REG, ioread32(dev_sub3, DEVID_REG));
    iowrite32(dev_sub3, COHERENCE_REG, coherence);
    iowrite32(dev_sub3, PT_ADDRESS_REG, (unsigned long)ptable_sub3);
    iowrite32(dev_sub3, PT_NCHUNK_REG, NCHUNK(mem_size_sub3));
    iowrite32(dev_sub3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_sub3, SRC_OFFSET_REG, 0);
    iowrite32(dev_sub3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_warp_dx3, SELECT_REG, ioread32(dev_warp_dx3, DEVID_REG));
    iowrite32(dev_warp_dx3, COHERENCE_REG, coherence);
    iowrite32(dev_warp_dx3, PT_ADDRESS_REG, (unsigned long)ptable_warp_dx3);
    iowrite32(dev_warp_dx3, PT_NCHUNK_REG, NCHUNK(mem_size_warp_dx3));
    iowrite32(dev_warp_dx3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_warp_dx3, SRC_OFFSET_REG, 0);
    iowrite32(dev_warp_dx3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_warp_dy3, SELECT_REG, ioread32(dev_warp_dy3, DEVID_REG));
    iowrite32(dev_warp_dy3, COHERENCE_REG, coherence);
    iowrite32(dev_warp_dy3, PT_ADDRESS_REG, (unsigned long)ptable_warp_dy3);
    iowrite32(dev_warp_dy3, PT_NCHUNK_REG, NCHUNK(mem_size_warp_dy3));
    iowrite32(dev_warp_dy3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_warp_dy3, SRC_OFFSET_REG, 0);
    iowrite32(dev_warp_dy3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_steep_descent3, SELECT_REG, ioread32(dev_steep_descent3, DEVID_REG));
    iowrite32(dev_steep_descent3, COHERENCE_REG, coherence);
    iowrite32(dev_steep_descent3, PT_ADDRESS_REG, (unsigned long)ptable_steep_descent3);
    iowrite32(dev_steep_descent3, PT_NCHUNK_REG, NCHUNK(mem_size_steep_descent3));
    iowrite32(dev_steep_descent3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_steep_descent3, SRC_OFFSET_REG, 0);
    iowrite32(dev_steep_descent3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_hessian3, SELECT_REG, ioread32(dev_hessian3, DEVID_REG));
    iowrite32(dev_hessian3, COHERENCE_REG, coherence);
    iowrite32(dev_hessian3, PT_ADDRESS_REG, (unsigned long)ptable_hessian3);
    iowrite32(dev_hessian3, PT_NCHUNK_REG, NCHUNK(mem_size_hessian3));
    iowrite32(dev_hessian3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_hessian3, SRC_OFFSET_REG, 0);
    iowrite32(dev_hessian3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_inv3, SELECT_REG, ioread32(dev_inv3, DEVID_REG));
    iowrite32(dev_inv3, COHERENCE_REG, coherence);
    iowrite32(dev_inv3, PT_ADDRESS_REG, (unsigned long)ptable_inv3);
    iowrite32(dev_inv3, PT_NCHUNK_REG, NCHUNK(mem_size_inv3));
    iowrite32(dev_inv3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_inv3, SRC_OFFSET_REG, 0);
    iowrite32(dev_inv3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_sd_update3, SELECT_REG, ioread32(dev_sd_update3, DEVID_REG));
    iowrite32(dev_sd_update3, COHERENCE_REG, coherence);
    iowrite32(dev_sd_update3, PT_ADDRESS_REG, (unsigned long)ptable_sd_update3);
    iowrite32(dev_sd_update3, PT_NCHUNK_REG, NCHUNK(mem_size_sd_update3));
    iowrite32(dev_sd_update3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_sd_update3, SRC_OFFSET_REG, 0);
    iowrite32(dev_sd_update3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_mult3, SELECT_REG, ioread32(dev_mult3, DEVID_REG));
    iowrite32(dev_mult3, COHERENCE_REG, coherence);
    iowrite32(dev_mult3, PT_ADDRESS_REG, (unsigned long)ptable_mult3);
    iowrite32(dev_mult3, PT_NCHUNK_REG, NCHUNK(mem_size_mult3));
    iowrite32(dev_mult3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_mult3, SRC_OFFSET_REG, 0);
    iowrite32(dev_mult3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_reshape3, SELECT_REG, ioread32(dev_reshape3, DEVID_REG));
    iowrite32(dev_reshape3, COHERENCE_REG, coherence);
    iowrite32(dev_reshape3, PT_ADDRESS_REG, (unsigned long)ptable_reshape3);
    iowrite32(dev_reshape3, PT_NCHUNK_REG, NCHUNK(mem_size_reshape3));
    iowrite32(dev_reshape3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_reshape3, SRC_OFFSET_REG, 0);
    iowrite32(dev_reshape3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_add3, SELECT_REG, ioread32(dev_add3, DEVID_REG));
    iowrite32(dev_add3, COHERENCE_REG, coherence);
    iowrite32(dev_add3, PT_ADDRESS_REG, (unsigned long)ptable_add3);
    iowrite32(dev_add3, PT_NCHUNK_REG, NCHUNK(mem_size_add3));
    iowrite32(dev_add3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_add3, SRC_OFFSET_REG, 0);
    iowrite32(dev_add3, DST_OFFSET_REG, 0);
    // ---------------------------------------
    iowrite32(dev_warp_iwxp3, SELECT_REG, ioread32(dev_warp_iwxp3, DEVID_REG));
    iowrite32(dev_warp_iwxp3, COHERENCE_REG, coherence);
    iowrite32(dev_warp_iwxp3, PT_ADDRESS_REG, (unsigned long)ptable_warp_iwxp3);
    iowrite32(dev_warp_iwxp3, PT_NCHUNK_REG, NCHUNK(mem_size_warp_iwxp3));
    iowrite32(dev_warp_iwxp3, PT_SHIFT_REG, CHUNK_SHIFT);
    iowrite32(dev_warp_iwxp3, SRC_OFFSET_REG, 0);
    iowrite32(dev_warp_iwxp3, DST_OFFSET_REG, 0);
    // ---------------------------------------

    ///////////////////////////////////
    // Execution on accelerators
    printf("  Execute on accelerators...\n");

    // start measuring accelerator execution time
    time_acc_start = get_counter();

    // Flush (customize coherence model here)
    esp_flush(coherence);

    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_debayer3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_debayer3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_debayer3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_debayer3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_debayer3, WAMI_PAD_REG, 2);
    iowrite32(dev_debayer3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_debayer3, WAMI_BATCH_REG, 1);
    iowrite32(dev_debayer3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_debayer3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_debayer3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_debayer3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_debayer3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_debayer3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_debayer3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for debayer3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_grayscale3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_grayscale3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_grayscale3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_grayscale3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_grayscale3, WAMI_PAD_REG, 2);
    iowrite32(dev_grayscale3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_grayscale3, WAMI_BATCH_REG, 1);
    iowrite32(dev_grayscale3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_grayscale3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_grayscale3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_grayscale3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_grayscale3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_grayscale3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_grayscale3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for grayscale3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_gradient3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_gradient3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_gradient3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_gradient3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_gradient3, WAMI_PAD_REG, 2);
    iowrite32(dev_gradient3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_gradient3, WAMI_BATCH_REG, 1);
    iowrite32(dev_gradient3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_gradient3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_gradient3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_gradient3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_gradient3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_gradient3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_gradient3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for gradient3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_warp_img3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_warp_img3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_warp_img3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_warp_img3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_warp_img3, WAMI_PAD_REG, 2);
    iowrite32(dev_warp_img3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_warp_img3, WAMI_BATCH_REG, 1);
    iowrite32(dev_warp_img3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_warp_img3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_warp_img3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_warp_img3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_warp_img3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_warp_img3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_warp_img3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for warp_img3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_sub3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_sub3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_sub3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_sub3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_sub3, WAMI_PAD_REG, 2);
    iowrite32(dev_sub3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_sub3, WAMI_BATCH_REG, 1);
    iowrite32(dev_sub3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_sub3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_sub3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_sub3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_sub3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_sub3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_sub3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for sub3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_warp_dx3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_warp_dx3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_warp_dx3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_warp_dx3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_warp_dx3, WAMI_PAD_REG, 2);
    iowrite32(dev_warp_dx3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_warp_dx3, WAMI_BATCH_REG, 1);
    iowrite32(dev_warp_dx3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_warp_dx3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_warp_dx3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_warp_dx3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_warp_dx3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_warp_dx3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_warp_dx3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for warp_dx3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_warp_dy3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_warp_dy3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_warp_dy3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_warp_dy3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_warp_dy3, WAMI_PAD_REG, 2);
    iowrite32(dev_warp_dy3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_warp_dy3, WAMI_BATCH_REG, 1);
    iowrite32(dev_warp_dy3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_warp_dy3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_warp_dy3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_warp_dy3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_warp_dy3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_warp_dy3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_warp_dy3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for warp_dy3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_steep_descent3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_steep_descent3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_steep_descent3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_steep_descent3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_steep_descent3, WAMI_PAD_REG, 2);
    iowrite32(dev_steep_descent3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_steep_descent3, WAMI_BATCH_REG, 1);
    iowrite32(dev_steep_descent3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_steep_descent3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_steep_descent3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_steep_descent3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_steep_descent3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_steep_descent3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_steep_descent3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for steep_descent3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_hessian3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_hessian3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_hessian3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_hessian3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_hessian3, WAMI_PAD_REG, 2);
    iowrite32(dev_hessian3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_hessian3, WAMI_BATCH_REG, 1);
    iowrite32(dev_hessian3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_hessian3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_hessian3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_hessian3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_hessian3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_hessian3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_hessian3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for hessian3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_inv3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_inv3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_inv3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_inv3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_inv3, WAMI_PAD_REG, 2);
    iowrite32(dev_inv3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_inv3, WAMI_BATCH_REG, 1);
    iowrite32(dev_inv3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_inv3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_inv3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_inv3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_inv3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_inv3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_inv3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for inv3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_sd_update3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_sd_update3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_sd_update3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_sd_update3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_sd_update3, WAMI_PAD_REG, 2);
    iowrite32(dev_sd_update3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_sd_update3, WAMI_BATCH_REG, 1);
    iowrite32(dev_sd_update3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_sd_update3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_sd_update3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_sd_update3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_sd_update3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_sd_update3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_sd_update3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for sd_update3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_mult3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_mult3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_mult3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_mult3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_mult3, WAMI_PAD_REG, 2);
    iowrite32(dev_mult3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_mult3, WAMI_BATCH_REG, 1);
    iowrite32(dev_mult3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_mult3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_mult3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_mult3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_mult3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_mult3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_mult3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for mult3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_reshape3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_reshape3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_reshape3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_reshape3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_reshape3, WAMI_PAD_REG, 2);
    iowrite32(dev_reshape3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_reshape3, WAMI_BATCH_REG, 1);
    iowrite32(dev_reshape3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_reshape3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_reshape3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_reshape3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_reshape3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_reshape3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_reshape3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for reshape3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_add3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_add3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_add3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_add3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_add3, WAMI_PAD_REG, 2);
    iowrite32(dev_add3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_add3, WAMI_BATCH_REG, 1);
    iowrite32(dev_add3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_add3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_add3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_add3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_add3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_add3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_add3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for add3 %d\n", cycles_diff);
    // ---------------------------------------
    cycles_start = esp_monitor(mon_args, NULL);
    iowrite32(dev_warp_iwxp3, WAMI_SRC_DST_OFFSET_0_REG, MEM_BASE_ADDR_WAMI_DEBAYER3);
    iowrite32(dev_warp_iwxp3, WAMI_NUM_IMG_REG, 1);
    iowrite32(dev_warp_iwxp3, WAMI_NUM_COL_REG, 128);
    iowrite32(dev_warp_iwxp3, WAMI_NUM_ROW_REG, 128);
    iowrite32(dev_warp_iwxp3, WAMI_PAD_REG, 2);
    iowrite32(dev_warp_iwxp3, WAMI_KERN_ID_REG, DEBAYER_KERN_ID);
    iowrite32(dev_warp_iwxp3, WAMI_BATCH_REG, 1);
    iowrite32(dev_warp_iwxp3, WAMI_SRC_DST_OFFSET_1_REG, MEM_BASE_ADDR_WAMI_DEBAYER3 + 17424);
    iowrite32(dev_warp_iwxp3, WAMI_SRC_DST_OFFSET_2_REG, 0);
    iowrite32(dev_warp_iwxp3, WAMI_SRC_DST_OFFSET_3_REG, 0);
    iowrite32(dev_warp_iwxp3, WAMI_SRC_DST_OFFSET_4_REG, 0);

    iowrite32(dev_warp_iwxp3, CMD_REG, CMD_MASK_START);
    done = 0;
    while (!done) {
        done = ioread32(dev_warp_iwxp3, STATUS_REG);
        done &= STATUS_MASK_DONE;
    }
    iowrite32(dev_warp_iwxp3, CMD_REG, 0x0);
    cycles_end  = esp_monitor(mon_args, NULL);
    cycles_diff = sub_monitor_vals(cycles_start, cycles_end);
    printf("===> cycles for warp_iwxp3 %d\n", cycles_diff);
    // ---------------------------------------

    // stop measuring accelerator execution time
    time_acc_stop = get_counter();

    ///////////////////////////////////
    // Execution on processor
    printf("  Execute on processor...\n");

    // start measuring processor execution time
    time_cpu_start = get_counter();

    // run the software now!
    run_pv(1);

    // stop measuring processor execution time
    time_cpu_stop = get_counter();

    ///////////////////////////////////
    // Validation
    printf("  Validation...\n");

    // gemm
    // errors = validate_buf_gemm(&mem_gemm[out_offset_gemm], &sw_buf_gemm[out_offset_gemm]);
    for (i = 0; i < 10; i++) {
        printf("mem_debayer3[%d] = %d\n", i, mem_debayer3[i]);
    }

    if (errors)
        printf("  ... debayer3 FAIL\n");
    else
        printf("  ... debayer3 PASS\n");

    ///////////////////////////////////
    // Free memory
    free_arrays();

    ///////////////////////////////////
    // Execution time
#ifdef _riscv
    time_acc_elapsed = (time_acc_stop - time_acc_start) * CLOCK_PERIOD;
    time_cpu_elapsed = (time_cpu_stop - time_cpu_start) * CLOCK_PERIOD;
    printf("  ------------------------\n");
    printf("    Accelerators execution time: %f sec\n", time_acc_elapsed);
    printf("    CPU execution time: %f sec\n", time_cpu_elapsed);
    printf("        --> Speedup: %f\n", time_cpu_elapsed / time_acc_elapsed);
    printf("  ------------------------\n");
#endif

    printf("  -----------------------------\n");
    printf("  --- Finish baremetal test ---\n");
    printf("  -----------------------------\n");
}
