// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <my_stringify.h>
#include <test/test.h>
#include <test/time.h>
#include "wami_run_stratus.h"

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define fx2float             fixed64_to_double
#define float2fx             double_to_fixed64
#define FX_IL_GRAYSCALE3     34
#define FX_IL_GRADIENT3      34
#define FX_IL_WARP3          34
#define FX_IL_SUB3           34
#define FX_IL_STEEP_DESCENT3 34
#define FX_IL_HESSIAN3       51
#define FX_IL_INV3           34
#define FX_IL_SD_UPDATE3     34
#define FX_IL_MULT3          34
#define FX_IL_RESHAPE3       34
#define FX_IL_ADD3           34

//-----------------------------------------------------------------------------
#define INPUT_SIZE INPUT_SIZE_132_17

#include "wami_C_data.h"
#include "wami_config_tb.h"
#include "wami_utils.h"

#include "../include/wami_debayer_pv.h"
#include "../include/wami_gmm_pv.h"
#include "../include/wami_gradient_pv.h"
#include "../include/wami_grayscale_pv.h"
#include "../include/wami_lucas_kanade_pv.h"
#include "../include/wami_matrix_ops.h"

#include "../include/fileio.h"

//--------------------------- below is for multi thread app -------------------
#include "kthreads.h"

#define KERNELS_NUM_00     4  // test_00
#define KERNELS_NUM_09     8  // test_09
#define KERNELS_NUM        16 // test_mtsm
#define KERNELS_NUM_COMBO1 15
#define KERNELS_NUM_COMBO2 15
#define KERNELS_NUM_COMBO3 15
#define KERNELS_NUM_COMBO4 14
#define FRAMES_NUM         1
#define QUEUE_CAPACITY     1

#ifndef FRAMES_NUM
    #warning "FRAMES_NUM is not defined! Set FRAMES_NUM to 3."
    #define FRAMES_NUM 3
#endif

#ifndef QUEUE_CAPACITY
    #warning "QUEUE_CAPACITY is not defined! Set QUEUE_CAPACITY to 1"
    #define QUEUE_CAPACITY 1
#endif

#define RAND        rand() % 100000
#define A_EXEC_TIME RAND
#define B_EXEC_TIME RAND
#define C_EXEC_TIME RAND
#define D_EXEC_TIME RAND
#define E_EXEC_TIME RAND
#define F_EXEC_TIME RAND
#define G_EXEC_TIME RAND
#define H_EXEC_TIME RAND

/* These values are from RTL simulation */

// ACC time (ms = ns/1000)
#define DEBAYER_EXEC_TIME             10119501 / 1000.0
#define GRAYSCALE_EXEC_TIME           3165824 / 1000.0
#define GRADIENT_EXEC_TIME            4402023 / 1000.0
#define WARP_GRAYSCALE_EXEC_TIME      9491635 / 1000.0
#define SUBTRACT_EXEC_TIME            3662233 / 1000.0
#define WARP_DX_EXEC_TIME             8505293 / 1000.0
#define WARP_DY_EXEC_TIME             8576665 / 1000.0
#define STEEPEST_DESCENT_EXEC_TIME    12864921 / 1000.0
#define HESSIAN_EXEC_TIME             29555507 / 1000.0
#define INVERT_GAUSS_JORDAN_EXEC_TIME 1001088 / 1000.0
#define SD_UPDATE_EXEC_TIME           12812724 / 1000.0
#define MULT_EXEC_TIME                1021056 / 1000.0
#define RESHAPE_EXEC_TIME             760883 / 1000.0
#define ADD_EXEC_TIME                 731034 / 1000.0
#define WARP_IWXP_EXEC_TIME           8521191 / 1000.0
#define CHANGE_DETECTION_EXEC_TIME    100

unsigned long long time_debayer;
unsigned long long time_grayscale;
unsigned long long time_gradient;
unsigned long long time_warp_img;
unsigned long long time_sub;
unsigned long long time_warp_dx;
unsigned long long time_warp_dy;
unsigned long long time_steep_descent;
unsigned long long time_hessian;
unsigned long long time_inv;
unsigned long long time_sd_update;
unsigned long long time_reshape;
unsigned long long time_add;
unsigned long long time_warp_iwxp;
unsigned long long time_change_detection;

static kthread_t ktreads[KERNELS_NUM]; // just use the max of all KERNELS_NUM here
pthread_mutex_t  printf_mutex;
//--------------------------------------------------------------------------------------

#include "cfg_independent.h"
#include "cfg_independent_batch.h"
#include "cfg_p2p.h"
#include "cfg_p2p_combo1.h"
#include "cfg_p2p_combo2.h"
#include "cfg_p2p_combo3.h"
#include "cfg_p2p_combo4.h"
#include "cfg_shared_memory.h"
// #include "cfg_multi_threads.h"

#include "monitors.h"

#define DEVNAME "/dev/wami_run_stratus.0"
#define NAME    "wami_run_stratus"

#define DBGMSG(MSG)                                         \
    {                                                       \
        printf("%s, line %d: %s", __FILE__, __LINE__, MSG); \
    }

#define CLIP_INRANGE(LOW, VAL, HIGH) ((VAL) < (LOW) ? (LOW) : ((VAL) > (HIGH) ? (HIGH) : (VAL)))

int read_debayer_input()
{
    unsigned flag = integrated_binary_read(input_filename, &nRows, &nCols, &padding_in, &M, &N, &IWxp, &nModels, &mu,
                                           &sigma, &weight, &nTestImages, &images, &results);

    printf("file: %s\n", input_filename);
    printf("nRows: %d\n", nRows);
    printf("nCols: %d\n", nCols);
    printf("padding_in: %d\n", padding_in);

    printf("M: %d\n", M);
    printf("N: %d\n", N);
    printf("IWxp: %f\n", *IWxp);
    printf("nModels: %d\n", nModels);
    printf("mu: %f\n", *mu);
    printf("sigma: %f\n", *sigma);
    printf("weight: %f\n", *weight);
    printf("nTestImages: %d\n", nTestImages);

    printf("-- INPUT_NUM_IMGS %d\n", INPUT_NUM_IMGS);
    printf("-- INPUT_IMG_NUM_ROWS %d\n", INPUT_IMG_NUM_ROWS);
    printf("-- INPUT_IMG_NUM_COLS %d\n", INPUT_IMG_NUM_COLS);
    printf("-- DEBAYER_INPUT_NUM_PXL %d\n", DEBAYER_INPUT_NUM_PXL);
    printf("-- DEBAYER_OUTPUT_NUM_PXL %d\n", DEBAYER_OUTPUT_NUM_PXL);
    printf("-- DEBAYER_TOTAL_INPUT_NUM_PXL %d\n", DEBAYER_TOTAL_INPUT_NUM_PXL);
    printf("-- DEBAYER_TOTAL_OUTPUT_NUM_PXL %d\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);

    nTestImages = 1; // [humu]: force to test only 1 images for now

    return flag;
}

void malloc_arrays()
{
    uint32_t img_num_pxls   = (M) * (N);
    uint32_t out_num_pxls   = img_num_pxls;
    uint32_t train_num_pxls = nModels * img_num_pxls;

    rgbtmpimg       = (rgb_pixel_t *)malloc(sizeof(rgb_pixel_t) * out_num_pxls);
    imgs            = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    grad_x          = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    grad_y          = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    IWxp_           = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    sub             = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    nabla_Ix        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    nabla_Iy        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    I_steepest      = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6 * out_num_pxls);
    hess            = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 36);
    hess_inv        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 36);
    sd_delta_p      = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    delta_p         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    sd_delta_p_nxt  = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    affine_warp_nxt = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    warp_iwxp       = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * out_num_pxls);
    foreground      = (uint8_t *)malloc(sizeof(uint8_t) * out_num_pxls);
    gmm_img         = (uint16_t *)malloc(sizeof(uint16_t) * out_num_pxls);
    mu_dump         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * train_num_pxls);
    sigma_dump      = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * train_num_pxls);
    weight_dump     = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * train_num_pxls);

    golden_rgb_imgs       = malloc(DEBAYER_TOTAL_OUTPUT_NUM_PXL * sizeof(rgb_pixel_t));
    golden_gs_imgs        = malloc(GRAYSCALE_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_grad_x         = malloc(GRADIENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_grad_y         = malloc(GRADIENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_img       = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    affine_warp           = malloc(6 * sizeof(flt_pixel_t));
    golden_sub            = malloc(SUBTRACT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_dx        = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_dy        = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_warp_iwxp      = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_I_steepest     = malloc(STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_hess           = malloc(HESSIAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_hess_inv       = malloc(INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    igj_workspace         = malloc(INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_sd_delta_p     = malloc(SD_UPDATE_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_delta_p        = malloc(MULT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_sd_delta_p_nxt = malloc(MULT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_affine_warp    = malloc(ADD_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    golden_foreground     = malloc(CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL * sizeof(uint8_t));
    golden_gmm_img        = malloc(CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL * sizeof(uint16_t));
}

void free_arrays()
{
    free(IWxp);
    free(mu);
    free(sigma);
    free(weight);
    free(images);
    free(results);

    free(rgbtmpimg);
    free(imgs);
    free(grad_x);
    free(grad_y);
    free(IWxp_);
    free(sub);
    free(nabla_Ix);
    free(nabla_Iy);
    free(I_steepest);
    free(hess);
    free(hess_inv);
    free(sd_delta_p);
    free(delta_p);
    free(sd_delta_p_nxt);
    free(affine_warp_nxt);
    free(warp_iwxp);
    free(foreground);
    free(gmm_img);
    free(mu_dump);
    free(sigma_dump);
    free(weight_dump);

    free(golden_rgb_imgs);
    free(golden_gs_imgs);
    free(golden_grad_x);
    free(golden_grad_y);
    free(golden_warp_img);
    free(affine_warp);
    free(golden_sub);
    free(golden_warp_dx);
    free(golden_warp_dy);
    free(golden_warp_iwxp);
    free(golden_I_steepest);
    free(golden_hess);
    free(golden_hess_inv);
    free(igj_workspace);
    free(golden_delta_p);
    free(golden_sd_delta_p);
    free(golden_sd_delta_p_nxt);
    free(golden_affine_warp);
}

void load_buf(token_t *buf)
{
    int x, y;

    printf("------- load_buf, start\n");

// -- load data to share memory
#if 1
    // load input for debayer3
    for (x = 0; x < nRows * nCols; x++) {
        y      = MEM_BASE_ADDR_WAMI_DEBAYER3 + nRows * nCols + x;
        buf[y] = images[x];
    }
    // load input for grayscale3
    for (x = 0; x < M * N; x++) {
        y                = MEM_BASE_ADDR_WAMI_GRAYSCALE3 + M * N + x;
        uint64_t pixel_r = golden_rgb_imgs[x].r;
        uint64_t pixel_g = golden_rgb_imgs[x].g;
        uint64_t pixel_b = golden_rgb_imgs[x].b;
        uint64_t pixel   = (pixel_r << 32) + (pixel_g << 16) + (pixel_b);
        buf[y]           = pixel;
    }
    // load input for gradient3
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_GRADIENT3 + (M * N) * 2 + x;
        double pixel = (double)golden_gs_imgs[x];
        buf[y]       = float2fx(pixel, FX_IL_GRADIENT3);
    }
    // load input for warp3-img
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_IMG3 + M * N + x;
        double pixel = (double)golden_gs_imgs[x];
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_IMG3 + M * N * 2 + x;
        double pixel = 0.0;
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    // load input for sub3
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_SUB3 + M * N + x;
        double pixel = (double)IWxp[x];
        buf[y]       = float2fx(pixel, FX_IL_SUB3);
    }
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_SUB3 + M * N * 2 + x;
        double pixel = (double)golden_warp_img[x];
        buf[y]       = float2fx(pixel, FX_IL_SUB3);
    }
    // load input for warp3-x
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_X3 + M * N + x;
        double pixel = (double)golden_grad_x[x];
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_X3 + M * N * 2 + x;
        double pixel = 0.0;
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    // load input for warp3-y
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_Y3 + M * N + x;
        double pixel = (double)golden_grad_y[x];
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_Y3 + M * N * 2 + x;
        double pixel = 0.0;
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
#endif
    // load input for steep_descent3
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + M * N * 6 + x;
        double pixel = (double)golden_warp_dx[x];
        buf[y]       = float2fx(pixel, FX_IL_STEEP_DESCENT3);
    }
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 + M * N * 7 + x;
        double pixel = (double)golden_warp_dy[x];
        buf[y]       = float2fx(pixel, FX_IL_STEEP_DESCENT3);
    }
#if 1
    // load input for hessian3
    for (x = 0; x < 6 * M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_HESSIAN3 + 36 + x;
        double pixel = (double)golden_I_steepest[x];
        buf[y]       = float2fx(pixel, FX_IL_HESSIAN3);
    }
#endif
    // load input for inv3
    for (x = 0; x < 36; x++) {
        y            = MEM_BASE_ADDR_WAMI_INV3 + 36 + x;
        double pixel = (double)golden_hess[x];
        buf[y]       = float2fx(pixel, FX_IL_INV3);
    }
#if 1
    // load input for sd_update3
    for (x = 0; x < 6 * M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + 6 + x;
        double pixel = (double)golden_I_steepest[x];
        buf[y]       = float2fx(pixel, FX_IL_SD_UPDATE3);
    }
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_SD_UPDATE3 + 6 + 6 * M * N + x;
        double pixel = (double)golden_sub[x];
        buf[y]       = float2fx(pixel, FX_IL_SD_UPDATE3);
    }
    // load input for mult3
    for (x = 0; x < 36; x++) {
        y            = MEM_BASE_ADDR_WAMI_MULT3 + 6 + x;
        double pixel = (double)golden_hess_inv[x];
        buf[y]       = float2fx(pixel, FX_IL_MULT3);
    }
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_MULT3 + 6 + 36 + x;
        double pixel = (double)golden_sd_delta_p[x];
        buf[y]       = float2fx(pixel, FX_IL_MULT3);
    }
    // load input for reshape3
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_RESHAPE3 + 6 + x;
        double pixel = (double)golden_delta_p[x];
        buf[y]       = float2fx(pixel, FX_IL_RESHAPE3);
    }
    // load input for add3
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_ADD3 + 6 + x;
        double pixel = 0.0;
        buf[y]       = float2fx(pixel, FX_IL_ADD3);
    }
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_ADD3 + 6 + 6 + x;
        double pixel = (double)golden_sd_delta_p_nxt[x];
        buf[y]       = float2fx(pixel, FX_IL_ADD3);
    }
#endif
#if 1
    // load input for warp3-iwxp
    for (x = 0; x < M * N; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + M * N + x;
        double pixel = (double)golden_gs_imgs[x];
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    for (x = 0; x < 6; x++) {
        y            = MEM_BASE_ADDR_WAMI_WARP_IWXP3 + M * N * 2 + x;
        double pixel = (double)golden_affine_warp[x];
        buf[y]       = float2fx(pixel, FX_IL_WARP3);
    }
    // load input for gradient3 dummy
    // printf("------- load_buf, gradient3 dummy\n");
    // for (x = 0; x < M * N; x++) {
    //     // y            = MEM_BASE_ADDR_WAMI_GRADIENT3_DUMMY + (M * N) + x; //
    //     <-- note this is different from gradient
    //             y            = MEM_BASE_ADDR_WAMI_GRADIENT3 + (M * N) * 2 + x;
    //     printf("x = %d\ty = %d\n", x,y);
    //     double pixel = (double)golden_gs_imgs[x];
    //     buf[y]       = float2fx(pixel, FX_IL_GRADIENT3);
    // }
    // printf("------- load_buf, finish\n");

#endif
    printf("------- load_buf, finish\n");
}

void reset_buf_to_zero(token_t *buf)
{
    printf("------- reset_buf_to_zero, MEM_ONE_IMAGE_SIZE = %d\n", MEM_ONE_IMAGE_SIZE);
    uint64_t x;
    // [humu]: note: max value for x is 655360, why?
    for (x = 0; x < MEM_ONE_IMAGE_SIZE; x++) {
        // printf("x = %d\n", x);
        buf[x] = 0;
    }
    printf("------- reset_buf_to_zero, finish\n");
}

void validate_buf(token_t *buf)
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
    printf("MEM_BASE_ADDR_WAMI_WARP_X3 = %d\n", MEM_BASE_ADDR_WAMI_WARP_X3);

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
    printf("MEM_BASE_ADDR_WAMI_WARP_Y3 = %d\n", MEM_BASE_ADDR_WAMI_WARP_Y3);

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

        if (abs(pixel - golden_I_steepest[x]) / golden_I_steepest[x] > 0.01) {
            error_count++;
        }
        if (x < 6) {
            printf("i = %d\tout= %f\tgold= %f\n", x, pixel, golden_I_steepest[x]);
        }
    }
    printf("MEM_BASE_ADDR_WAMI_STEEP_DESCENT3 = %d\n", MEM_BASE_ADDR_WAMI_STEEP_DESCENT3);

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
    printf("MEM_BASE_ADDR_WAMI_HESSIAN3 = %d\n", MEM_BASE_ADDR_WAMI_HESSIAN3);

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

void print_sw_time()
{
    unsigned long long time_s;
    time_s = ts_subtract(&t_debayer_1, &t_debayer_2);
    printf("sw time of debayer_3       : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_grayscale_1, &t_grayscale_2);
    printf("sw time of grayscale_3     : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_gradient_1, &t_gradient_2);
    printf("sw time of gradient_3      : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_warp_img_1, &t_warp_img_2);
    printf("sw time of warp_img_3      : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_subtract_1, &t_subtract_2);
    printf("sw time of subtract_3      : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_warp_x_1, &t_warp_x_2);
    printf("sw time of warp_x_3        : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_warp_y_1, &t_warp_y_2);
    printf("sw time of warp_y_3        : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_steep_descent_1, &t_steep_descent_2);
    printf("sw time of steep_descent_3 : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_hessian_1, &t_hessian_2);
    printf("sw time of hessian_3       : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_inv_1, &t_inv_2);
    printf("sw time of inv_3           : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_sd_update_1, &t_sd_update_2);
    printf("sw time of sd_update_3     : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_mult_1, &t_mult_2);
    printf("sw time of mult_3          : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_reshape_1, &t_reshape_2);
    printf("sw time of reshape_3       : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_add_1, &t_add_2);
    printf("sw time of add_3           : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_warp_iwxp_1, &t_warp_iwxp_2);
    printf("sw time of warp_iwxp_3     : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_gradient_dummy_1, &t_gradient_dummy_2);
    printf("sw time of gradient_dummy  : %llu (ns)\n", time_s);
    // time_s = ts_subtract(&t_reshape_add_1, &t_reshape_add_2);
    // printf("sw time of reshape_add           : %llu (ns)\n", time_s);
    // time_s = ts_subtract(&t_hessian_inv_1, &t_hessian_inv_2);
    // printf("sw time of hessian_inv           : %llu (ns)\n", time_s);
    // time_s = ts_subtract(&t_steep_descent_hessian_1, &t_steep_descent_hessian_2);
    // printf("sw time of steep_descent_hessian : %llu (ns)\n", time_s);
    time_s = ts_subtract(&t_gmm_1, &t_gmm_2);
    printf("sw time of gmm_3           : %llu (ns)\n", time_s);
}

//--------------------------------------------------------------------------------------
void run_debayer_hw(void *ptr)
{
    // printf("run_debayer_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_debayer3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_debayer3_indep, 1);
    esp_run_no_print(cfg_wami_debayer3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("debayer: HW exec time: %llu ns\n", hw_ns);
}
void run_grayscale_hw(void *ptr)
{
    // printf("run_grayscale_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_grayscale3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_grayscale3_indep, 1);
    esp_run_no_print(cfg_wami_grayscale3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("grayscale: HW exec time: %llu ns\n", hw_ns);
}
void run_gradient_hw(void *ptr)
{
    // printf("run_gradient_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_gradient3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_gradient3_indep, 1);
    esp_run_no_print(cfg_wami_gradient3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("gradient: HW exec time: %llu ns\n", hw_ns);
}
void run_warp_grayscale_hw(void *ptr)
{
    // printf("run_warp_grayscale_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_warp_img3_indep, 1);
    // // esp_run_1_no_thread(cfg_wami_warp_img3_indep, 1);
    esp_run_no_print(cfg_wami_warp_img3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("warp_grayscale: HW exec time: %llu ns\n", hw_ns);
}
void run_subtract_hw(void *ptr)
{
    // printf("run_subtract_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_sub3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_sub3_indep, 1);
    esp_run_no_print(cfg_wami_warp_img3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("subtract: HW exec time: %llu ns\n", hw_ns);
}
void run_warp_dx_hw(void *ptr)
{
    // printf("run_warp_dx_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_warp_x3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_x3_indep, 1);
    esp_run_no_print(cfg_wami_warp_x3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("warp_dx: HW exec time: %llu ns\n", hw_ns);
}
void run_warp_dy_hw(void *ptr)
{
    // printf("run_warp_dy_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_warp_y3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_y3_indep, 1);
    esp_run_no_print(cfg_wami_warp_y3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("warp_dy: HW exec time: %llu ns\n", hw_ns);
}
void run_steepest_descent_hw(void *ptr)
{
    // printf("run_steepest_descent_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_steep_descent3_indep, 1);
    // // esp_run_1_no_thread(cfg_wami_steep_descent3_indep, 1);
    esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("steep_descent: HW exec time: %llu ns\n", hw_ns);
}
void run_hessian_hw(void *ptr)
{
    // printf("run_hessian_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_hessian3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_hessian3_indep, 1);
    esp_run_no_print(cfg_wami_hessian3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("hessian: HW exec time: %llu ns\n", hw_ns);
}
void run_invert_gauss_jordan_hw(void *ptr)
{
    // printf("run_invert_gauss_jordan_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_inv3_indep, 1);
    // // esp_run_1_no_thread(cfg_wami_inv3_indep, 1);
    esp_run_no_print(cfg_wami_inv3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("invert_guass_jordan: HW exec time: %llu ns\n", hw_ns);
}
void run_sd_update_hw(void *ptr)
{
    // printf("run_sd_update_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_sd_update3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_sd_update3_indep, 1);
    esp_run_no_print(cfg_wami_sd_update3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("sd_update: HW exec time: %llu ns\n", hw_ns);
}
void run_mult_hw(void *ptr)
{
    // printf("run_mult_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_mult3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_mult3_indep, 1);
    esp_run_no_print(cfg_wami_mult3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("mult: HW exec time: %llu ns\n", hw_ns);
}
void run_reshape_hw(void *ptr)
{
    // printf("run_reshape_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_reshape3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_reshape3_indep, 1);
    esp_run_no_print(cfg_wami_reshape3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("reshape: HW exec time: %llu ns\n", hw_ns);
}
void run_add_hw(void *ptr)
{
    // printf("run_add_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    // unsigned queue_dst_0 = 0;
    // unsigned queue_src_0 = 0;
    // unsigned queue_src_1 = thread_data->fifo_get_index[0];

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(fd_add, WAMI_APP_IOC_ACCESS, desc_add);
    // esp_run(cfg_wami_add3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_add3_indep, 1);
    esp_run_no_print(cfg_wami_add3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("add: HW exec time: %llu ns\n", hw_ns);
}
void run_warp_iwxp_hw(void *ptr)
{
    // printf("run_warp_iwxp_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_warp_iwxp3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_iwxp3_indep, 1);
    esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("warp_iwxp: HW exec time: %llu ns\n", hw_ns);
}

void run_change_detection_hw(void *ptr)
{
    // printf("run_change_detection_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_gradient3_dummy_indep, 1);
    // esp_run_1_no_thread(cfg_wami_gradient3_dummy_indep, 1);

    // esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
    esp_run_no_print(cfg_wami_gmm3_indep, 1);

    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("change_detection: HW exec time: %llu ns\n", hw_ns);
}

void run_reshape_add_hw(void *ptr)
{
    // printf("run_reshape_add_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(fd_add, WAMI_APP_IOC_ACCESS, desc_add);
    // esp_run(cfg_wami_reshape_add_indep, 1);
    // esp_run_1_no_thread(cfg_wami_reshape_add_indep, 1);
    esp_run_no_print(cfg_wami_reshape_add_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("reshape_add: HW exec time: %llu ns\n", hw_ns);
}
void run_hessian_inv_hw(void *ptr)
{
    // printf("run_hessian_inv_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_hessian_inv_indep, 1);
    // esp_run_1_no_thread(cfg_wami_hessian_inv_indep, 1);
    esp_run_no_print(cfg_wami_hessian_inv_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("hessian_inv: HW exec time: %llu ns\n", hw_ns);
}
void run_steep_descent_hessian_hw(void *ptr)
{
    // printf("run_steep_descent_hessian_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(...)
    // esp_run(cfg_wami_steep_descent_hessian_indep, 1);
    // esp_run_1_no_thread(cfg_wami_steep_descent_hessian_indep, 1);
    esp_run_no_print(cfg_wami_steep_descent_hessian_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("steep_descent_hessian: HW exec time: %llu ns\n", hw_ns);
}
void run_mult_reshape_add_hw(void *ptr)
{
    // printf("run_mult_reshape_add_hw\n");
    // struct kthread_t *thread_data = (struct kthread_t *)ptr;

    // int                rc;
    // struct timespec    ts_start;
    // struct timespec    ts_end;
    // unsigned long long hw_ns = 0;

    /* TODO: is this the right point to increment it? */
    // thread_data->n_imgs++;

    // gettime(&ts_start);
    // rc = ioctl(fd_add, WAMI_APP_IOC_ACCESS, desc_add);
    // esp_run(cfg_wami_mult_reshape_add_indep, 1);
    // esp_run_1_no_thread(cfg_wami_mult_reshape_add_indep, 1);
    esp_run_no_print(cfg_wami_mult_reshape_add_indep, 1);
    // gettime(&ts_end);

    // hw_ns = ts_subtract(&ts_start, &ts_end);
    // thread_data->hw_ns += hw_ns;

    // printf("reshape_add: HW exec time: %llu ns\n", hw_ns);
}

int test_00()
{
    //
    // This is an example test for multi-thread shared memory
    //
    // A -> B -> C -> D
    //

    printf("-- test_00 start\n");

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", FRAMES_NUM);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_00);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));
    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 1; /* A */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].fifo.capacity    = FRAMES_NUM;
    ktreads[3].fifo.consumers   = 0; /* SINK! */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_00, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_00, FRAMES_NUM);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_00; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }
    //  for (unsigned i = 0; i < KERNELS_NUM_00; i++)
    //  { init_feedback(&ktreads[i]); }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_00);
  return 0;
#endif

    dump_dgraph(ktreads, KERNELS_NUM_00, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_00; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_00)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_00; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_00);

    printf("-- test_00 done\n");
    return 0;
}

int test_09()
{
    //
    // This is an example test with feedback loop for multi-thread shared memory
    //
    //      v---------,    v---------,
    // A -> B -> C -> D -> E -> F -> G -> H
    //
    //

    printf("-- test_09 start\n");

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", FRAMES_NUM);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_09);

    srand(0);

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));
    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "A";
    ktreads[0].execution_time   = A_EXEC_TIME;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* B */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "B";
    ktreads[1].execution_time   = B_EXEC_TIME;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 1; /* C */
    ktreads[1].incoming         = 2; /* A, D */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0], &ktreads[3]};
    ktreads[1].feedback         = (unsigned[]){0, 1};

    ktreads[2].id               = "C";
    ktreads[2].execution_time   = C_EXEC_TIME;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 1; /* D */
    ktreads[2].incoming         = 1; /* B */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "D";
    ktreads[3].execution_time   = D_EXEC_TIME;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 2; /* E, B */
    ktreads[3].incoming         = 1; /* C */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[2]};
    ktreads[3].feedback         = (unsigned[]){0};

    ktreads[4].id               = "E";
    ktreads[4].execution_time   = E_EXEC_TIME;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* F */
    ktreads[4].incoming         = 2; /* D, G */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[6]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "F";
    ktreads[5].execution_time   = F_EXEC_TIME;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* G */
    ktreads[5].incoming         = 1; /* E */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[4]};
    ktreads[5].feedback         = (unsigned[]){0};

    ktreads[6].id               = "G";
    ktreads[6].execution_time   = G_EXEC_TIME;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 2; /* H, E */
    ktreads[6].incoming         = 1; /* F */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[5]};
    ktreads[6].feedback         = (unsigned[]){0};

    ktreads[7].id               = "H";
    ktreads[7].execution_time   = H_EXEC_TIME;
    ktreads[7].fifo.capacity    = FRAMES_NUM;
    ktreads[7].fifo.consumers   = 0; /* SINK! */
    ktreads[7].incoming         = 1; /* G */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_09, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_09, FRAMES_NUM);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_09; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }
    //  for (i = 0; i < KERNELS_NUM_09; i++)
    //  { init_feedback(&ktreads[i]); }
    tcount = 0;
    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_09);
  return 0;
#endif

    dump_dgraph(ktreads, KERNELS_NUM_09, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_09; i++)
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);

    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_09)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_09; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_09);

    printf("-- test_09 done\n");
    return 0;
}

int test_mtsm(token_t *buf, int frames_num)
{
    //
    // This function tests multi-thread shared memory
    //

    cfg_wami_debayer3_indep[0].hw_buf        = buf;
    cfg_wami_grayscale3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_indep[0].hw_buf       = buf;
    cfg_wami_warp_img3_indep[0].hw_buf       = buf;
    cfg_wami_sub3_indep[0].hw_buf            = buf;
    cfg_wami_warp_x3_indep[0].hw_buf         = buf;
    cfg_wami_warp_y3_indep[0].hw_buf         = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;
    cfg_wami_gmm3_indep[0].hw_buf            = buf;

    struct wami_run_stratus_access *tmp;
    tmp = (struct wami_run_stratus_access *)cfg_wami_debayer3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_grayscale3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_gradient3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_warp_img3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_sub3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_warp_x3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_warp_y3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_steep_descent3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_hessian3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_inv3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_sd_update3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_mult3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_reshape3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_add3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_warp_iwxp3_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_gradient3_dummy_indep[0].esp_desc; tmp->wami_num_img = frames_num;
    tmp = (struct wami_run_stratus_access *)cfg_wami_gmm3_indep[0].esp_desc; tmp->wami_num_img = frames_num;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", frames_num);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "debayer";
    ktreads[0].execution_time   = DEBAYER_EXEC_TIME;
    ktreads[0].run_hw           = &run_debayer_hw;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* grayscale */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "grayscale";
    ktreads[1].execution_time   = GRAYSCALE_EXEC_TIME;
    ktreads[1].run_hw           = &run_grayscale_hw;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 3; /* gradient, warp_grayscale, warp_iwxp */
    ktreads[1].incoming         = 1; /* debayer */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "gradient";
    ktreads[2].execution_time   = GRADIENT_EXEC_TIME;
    ktreads[2].run_hw           = &run_gradient_hw;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 2; /* warp_dx, warp_dy */
    ktreads[2].incoming         = 1; /* grayscale */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "warp_grayscale";
    ktreads[3].execution_time   = WARP_GRAYSCALE_EXEC_TIME;
    ktreads[3].run_hw           = &run_warp_grayscale_hw;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* subtract */
    ktreads[3].incoming         = 2; /* grayscale, add */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[13]};
    ktreads[3].feedback         = (unsigned[]){0, 1};

    ktreads[4].id               = "subtract";
    ktreads[4].execution_time   = SUBTRACT_EXEC_TIME;
    ktreads[4].run_hw           = &run_subtract_hw;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* sd_update */
    ktreads[4].incoming         = 2; /* warp_grayscale, warp_iwxp */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[14]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "warp_dx";
    ktreads[5].execution_time   = WARP_DX_EXEC_TIME;
    ktreads[5].run_hw           = &run_warp_dx_hw;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* steepest_descent */
    ktreads[5].incoming         = 2; /* gradient, add */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[13]};
    ktreads[5].feedback         = (unsigned[]){0, 1};

    ktreads[6].id               = "warp_dy";
    ktreads[6].execution_time   = WARP_DY_EXEC_TIME;
    ktreads[6].run_hw           = &run_warp_dy_hw;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 1; /* steepest_descent */
    ktreads[6].incoming         = 2; /* gradient, add */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[13]};
    ktreads[6].feedback         = (unsigned[]){0, 1};

    ktreads[7].id               = "steepest_descent";
    ktreads[7].execution_time   = STEEPEST_DESCENT_EXEC_TIME;
    ktreads[7].run_hw           = &run_steepest_descent_hw;
    ktreads[7].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[7].fifo.consumers   = 2; /* sd_update, hessian */
    ktreads[7].incoming         = 2; /* warp_dx, warp_dy */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[5], &ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0, 0};

    ktreads[8].id               = "sd_update";
    ktreads[8].execution_time   = SD_UPDATE_EXEC_TIME;
    ktreads[8].run_hw           = &run_sd_update_hw;
    ktreads[8].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[8].fifo.consumers   = 1; /* mult */
    ktreads[8].incoming         = 2; /* subtract, steepest_descent */
    ktreads[8].incoming_threads = (kthread_t *[]){&ktreads[4], &ktreads[7]};
    ktreads[8].feedback         = (unsigned[]){0, 0};

    ktreads[9].id               = "hessian";
    ktreads[9].execution_time   = HESSIAN_EXEC_TIME;
    ktreads[9].run_hw           = &run_hessian_hw;
    ktreads[9].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[9].fifo.consumers   = 1; /* invert_gauss_jordan */
    ktreads[9].incoming         = 1; /* steepest_descent */
    ktreads[9].incoming_threads = (kthread_t *[]){&ktreads[7]};
    ktreads[9].feedback         = (unsigned[]){0};

    ktreads[10].id               = "invert_gauss_jordan";
    ktreads[10].execution_time   = INVERT_GAUSS_JORDAN_EXEC_TIME;
    ktreads[10].run_hw           = &run_invert_gauss_jordan_hw;
    ktreads[10].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[10].fifo.consumers   = 1; /* mult */
    ktreads[10].incoming         = 1; /* hessian */
    ktreads[10].incoming_threads = (kthread_t *[]){&ktreads[9]};
    ktreads[10].feedback         = (unsigned[]){0};

    ktreads[11].id               = "mult";
    ktreads[11].execution_time   = MULT_EXEC_TIME;
    ktreads[11].run_hw           = &run_mult_hw;
    ktreads[11].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[11].fifo.consumers   = 1; /* reshape */
    ktreads[11].incoming         = 2; /* sd_update, invert_gauss_jordan */
    ktreads[11].incoming_threads = (kthread_t *[]){&ktreads[8], &ktreads[10]};
    ktreads[11].feedback         = (unsigned[]){0, 0};

    ktreads[12].id               = "reshape";
    ktreads[12].execution_time   = RESHAPE_EXEC_TIME;
    ktreads[12].run_hw           = &run_reshape_hw;
    ktreads[12].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[12].fifo.consumers   = 1; /* add */
    ktreads[12].incoming         = 1; /* mult */
    ktreads[12].incoming_threads = (kthread_t *[]){&ktreads[11]};
    ktreads[12].feedback         = (unsigned[]){0};

    ktreads[13].id               = "add";
    ktreads[13].execution_time   = ADD_EXEC_TIME;
    ktreads[13].run_hw           = &run_add_hw;
    ktreads[13].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[13].fifo.consumers   = 4; /* warp_grayscale, warp_dx, warp_dy, warp_iwxp */
    ktreads[13].incoming         = 1; /* reshape */
    ktreads[13].incoming_threads = (kthread_t *[]){&ktreads[12]};
    ktreads[13].feedback         = (unsigned[]){0};

    ktreads[14].id               = "warp_iwxp";
    ktreads[14].execution_time   = WARP_IWXP_EXEC_TIME;
    ktreads[14].run_hw           = &run_warp_iwxp_hw;
    ktreads[14].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[14].fifo.consumers   = 2; /* subtract, change_detection */
    ktreads[14].incoming         = 2; /* grayscale, add */
    ktreads[14].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[13]};
    ktreads[14].feedback         = (unsigned[]){0, 0};

    ktreads[15].id               = "change_detection";
    ktreads[15].execution_time   = CHANGE_DETECTION_EXEC_TIME;
    ktreads[15].run_hw           = &run_change_detection_hw;
    ktreads[15].fifo.capacity    = frames_num;
    ktreads[15].fifo.consumers   = 0; /* SINK! */
    ktreads[15].incoming         = 1;
    ktreads[15].incoming_threads = (kthread_t *[]){&ktreads[14]};
    ktreads[15].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM, 0);

    /* init number of iterations */
    // set_iteration_counters(ktreads, KERNELS_NUM, frames_num);
    set_iteration_counters(ktreads, KERNELS_NUM, 1);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }

    for (i = 0; i < KERNELS_NUM; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;

    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM);
  return 0;
#endif

    // dump_dgraph(ktreads, KERNELS_NUM, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM; i++) {
        // fprintf(stderr, "..creating thread: %d\n", i);
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);
    }
    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    struct timespec time_start, time_end;
    uint64_t        time_diff;

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    const int          MEM_TILE_IDX = 0;
    // mon_args.read_mode = ESP_MON_READ_SINGLE;
    mon_args.read_mode  = ESP_MON_READ_ALL;
    mon_args.tile_index = MEM_TILE_IDX;
    mon_args.mon_index  = MON_DDR_WORD_TRANSFER_INDEX;

    unsigned int ddr_accesses_start, ddr_accesses_end;
    unsigned int ddr_accesses_diff;

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    esp_monitor(mon_args, &vals_start);

    gettime(&t_test_multi_thread_1);
    // clock_gettime(CLOCK_MONOTONIC, &time_start); /* mark start time */

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM; i++)
        pthread_join(ktreads[i].handle, NULL);

    // clock_gettime(CLOCK_MONOTONIC, &time_end); /* mark the end time */
    gettime(&t_test_multi_thread_2);
    // time_diff = BILLION * (time_end.tv_sec - time_start.tv_sec) + time_end.tv_nsec - time_start.tv_nsec;
    // fprintf(stderr, "multi-thread time: %llu (ns)\n", time_diff);

    wrap_up(ktreads, KERNELS_NUM);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_mtsm_esp_mon_all.txt", "w");
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // fclose(fp);

    // [humu]: disable validate for now
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_multi_thread_1, &t_test_multi_thread_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_mtsm x%d: %llu (ns)\n", frames_num, time_s);
    fprintf(log_file, "test_mtsm, %d, %llu\n", frames_num, time_s);
    printf("-------------------------------------------------------\n");

    return 0;
}

int test_mtsm_combo1(token_t *buf, int frames_num)
{
    //
    // This function tests multi-thread shared memory
    //

    cfg_wami_debayer3_indep[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep[0].hw_buf     = buf;
    cfg_wami_gradient3_indep[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep[0].hw_buf      = buf;
    cfg_wami_sub3_indep[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf = buf;
    cfg_wami_hessian3_indep[0].hw_buf       = buf;
    cfg_wami_inv3_indep[0].hw_buf           = buf;
    cfg_wami_sd_update3_indep[0].hw_buf     = buf;
    cfg_wami_mult3_indep[0].hw_buf          = buf;
    // cfg_wami_reshape3_indep[0].hw_buf        = buf;
    // cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_reshape_add_indep[0].hw_buf     = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;
    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", frames_num);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_COMBO1);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "debayer";
    ktreads[0].execution_time   = DEBAYER_EXEC_TIME;
    ktreads[0].run_hw           = &run_debayer_hw;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* grayscale */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "grayscale";
    ktreads[1].execution_time   = GRAYSCALE_EXEC_TIME;
    ktreads[1].run_hw           = &run_grayscale_hw;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 3; /* gradient, warp_grayscale, warp_iwxp */
    ktreads[1].incoming         = 1; /* debayer */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "gradient";
    ktreads[2].execution_time   = GRADIENT_EXEC_TIME;
    ktreads[2].run_hw           = &run_gradient_hw;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 2; /* warp_dx, warp_dy */
    ktreads[2].incoming         = 1; /* grayscale */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "warp_grayscale";
    ktreads[3].execution_time   = WARP_GRAYSCALE_EXEC_TIME;
    ktreads[3].run_hw           = &run_warp_grayscale_hw;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* subtract */
    ktreads[3].incoming         = 2; /* grayscale, add */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[12]};
    ktreads[3].feedback         = (unsigned[]){0, 1};

    ktreads[4].id               = "subtract";
    ktreads[4].execution_time   = SUBTRACT_EXEC_TIME;
    ktreads[4].run_hw           = &run_subtract_hw;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* sd_update */
    ktreads[4].incoming         = 2; /* warp_grayscale, warp_iwxp */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[13]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "warp_dx";
    ktreads[5].execution_time   = WARP_DX_EXEC_TIME;
    ktreads[5].run_hw           = &run_warp_dx_hw;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* steepest_descent */
    ktreads[5].incoming         = 2; /* gradient, add */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[12]};
    ktreads[5].feedback         = (unsigned[]){0, 1};

    ktreads[6].id               = "warp_dy";
    ktreads[6].execution_time   = WARP_DY_EXEC_TIME;
    ktreads[6].run_hw           = &run_warp_dy_hw;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 1; /* steepest_descent */
    ktreads[6].incoming         = 2; /* gradient, add */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[12]};
    ktreads[6].feedback         = (unsigned[]){0, 1};

    ktreads[7].id               = "steepest_descent";
    ktreads[7].execution_time   = STEEPEST_DESCENT_EXEC_TIME;
    ktreads[7].run_hw           = &run_steepest_descent_hw;
    ktreads[7].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[7].fifo.consumers   = 2; /* sd_update, hessian */
    ktreads[7].incoming         = 2; /* warp_dx, warp_dy */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[5], &ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0, 0};

    ktreads[8].id               = "sd_update";
    ktreads[8].execution_time   = SD_UPDATE_EXEC_TIME;
    ktreads[8].run_hw           = &run_sd_update_hw;
    ktreads[8].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[8].fifo.consumers   = 1; /* mult */
    ktreads[8].incoming         = 2; /* subtract, steepest_descent */
    ktreads[8].incoming_threads = (kthread_t *[]){&ktreads[4], &ktreads[7]};
    ktreads[8].feedback         = (unsigned[]){0, 0};

    ktreads[9].id               = "hessian";
    ktreads[9].execution_time   = HESSIAN_EXEC_TIME;
    ktreads[9].run_hw           = &run_hessian_hw;
    ktreads[9].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[9].fifo.consumers   = 1; /* invert_gauss_jordan */
    ktreads[9].incoming         = 1; /* steepest_descent */
    ktreads[9].incoming_threads = (kthread_t *[]){&ktreads[7]};
    ktreads[9].feedback         = (unsigned[]){0};

    ktreads[10].id               = "invert_gauss_jordan";
    ktreads[10].execution_time   = INVERT_GAUSS_JORDAN_EXEC_TIME;
    ktreads[10].run_hw           = &run_invert_gauss_jordan_hw;
    ktreads[10].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[10].fifo.consumers   = 1; /* mult */
    ktreads[10].incoming         = 1; /* hessian */
    ktreads[10].incoming_threads = (kthread_t *[]){&ktreads[9]};
    ktreads[10].feedback         = (unsigned[]){0};

    ktreads[11].id               = "mult";
    ktreads[11].execution_time   = MULT_EXEC_TIME;
    ktreads[11].run_hw           = &run_mult_hw;
    ktreads[11].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[11].fifo.consumers   = 1; /* reshape_add */
    ktreads[11].incoming         = 2; /* sd_update, invert_gauss_jordan */
    ktreads[11].incoming_threads = (kthread_t *[]){&ktreads[8], &ktreads[10]};
    ktreads[11].feedback         = (unsigned[]){0, 0};

    ktreads[12].id               = "reshape_add";
    ktreads[12].execution_time   = RESHAPE_EXEC_TIME + ADD_EXEC_TIME;
    ktreads[12].run_hw           = &run_reshape_add_hw;
    ktreads[12].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[12].fifo.consumers   = 4; /* warp_grayscale, warp_dx, warp_dy, warp_iwxp */
    ktreads[12].incoming         = 1; /* mult */
    ktreads[12].incoming_threads = (kthread_t *[]){&ktreads[11]};
    ktreads[12].feedback         = (unsigned[]){0};

    ktreads[13].id               = "warp_iwxp";
    ktreads[13].execution_time   = WARP_IWXP_EXEC_TIME;
    ktreads[13].run_hw           = &run_warp_iwxp_hw;
    ktreads[13].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[13].fifo.consumers   = 2; /* subtract, change_detection */
    ktreads[13].incoming         = 2; /* grayscale, add */
    ktreads[13].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[12]};
    ktreads[13].feedback         = (unsigned[]){0, 0};

    ktreads[14].id               = "change_detection";
    ktreads[14].execution_time   = CHANGE_DETECTION_EXEC_TIME;
    ktreads[14].run_hw           = &run_change_detection_hw;
    ktreads[14].fifo.capacity    = frames_num;
    ktreads[14].fifo.consumers   = 0; /* SINK! */
    ktreads[14].incoming         = 1;
    ktreads[14].incoming_threads = (kthread_t *[]){&ktreads[13]};
    ktreads[14].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_COMBO1, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_COMBO1, frames_num);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_COMBO1; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }

    for (i = 0; i < KERNELS_NUM_COMBO1; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;

    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_COMBO1);
  return 0;
#endif

    // dump_dgraph(ktreads, KERNELS_NUM_COMBO1, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_COMBO1; i++) {
        // fprintf(stderr, "..creating thread: %d\n", i);
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);
    }
    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_COMBO1)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_multi_thread_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_COMBO1; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_COMBO1);

    gettime(&t_test_multi_thread_2);

    // [humu]: disable validate for now
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_multi_thread_1, &t_test_multi_thread_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_mtsm_combo1 x%d: %llu (ns)\n", frames_num, time_s);
    fprintf(log_file_combo1, "test_mtsm_combo1, %d, %llu\n", frames_num, time_s);
    printf("-------------------------------------------------------\n");

    return 0;
}

int test_mtsm_combo2(token_t *buf, int frames_num)
{
    //
    // This function tests multi-thread shared memory
    //

    cfg_wami_debayer3_indep[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep[0].hw_buf     = buf;
    cfg_wami_gradient3_indep[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep[0].hw_buf      = buf;
    cfg_wami_sub3_indep[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf = buf;
    // cfg_wami_hessian3_indep[0].hw_buf        = buf;
    // cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_hessian_inv_indep[0].hw_buf     = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;
    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", frames_num);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_COMBO2);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "debayer";
    ktreads[0].execution_time   = DEBAYER_EXEC_TIME;
    ktreads[0].run_hw           = &run_debayer_hw;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* grayscale */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "grayscale";
    ktreads[1].execution_time   = GRAYSCALE_EXEC_TIME;
    ktreads[1].run_hw           = &run_grayscale_hw;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 3; /* gradient, warp_grayscale, warp_iwxp */
    ktreads[1].incoming         = 1; /* debayer */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "gradient";
    ktreads[2].execution_time   = GRADIENT_EXEC_TIME;
    ktreads[2].run_hw           = &run_gradient_hw;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 2; /* warp_dx, warp_dy */
    ktreads[2].incoming         = 1; /* grayscale */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "warp_grayscale";
    ktreads[3].execution_time   = WARP_GRAYSCALE_EXEC_TIME;
    ktreads[3].run_hw           = &run_warp_grayscale_hw;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* subtract */
    ktreads[3].incoming         = 2; /* grayscale, add */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[12]};
    ktreads[3].feedback         = (unsigned[]){0, 1};

    ktreads[4].id               = "subtract";
    ktreads[4].execution_time   = SUBTRACT_EXEC_TIME;
    ktreads[4].run_hw           = &run_subtract_hw;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* sd_update */
    ktreads[4].incoming         = 2; /* warp_grayscale, warp_iwxp */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[13]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "warp_dx";
    ktreads[5].execution_time   = WARP_DX_EXEC_TIME;
    ktreads[5].run_hw           = &run_warp_dx_hw;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* steepest_descent */
    ktreads[5].incoming         = 2; /* gradient, add */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[12]};
    ktreads[5].feedback         = (unsigned[]){0, 1};

    ktreads[6].id               = "warp_dy";
    ktreads[6].execution_time   = WARP_DY_EXEC_TIME;
    ktreads[6].run_hw           = &run_warp_dy_hw;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 1; /* steepest_descent */
    ktreads[6].incoming         = 2; /* gradient, add */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[12]};
    ktreads[6].feedback         = (unsigned[]){0, 1};

    ktreads[7].id               = "steepest_descent";
    ktreads[7].execution_time   = STEEPEST_DESCENT_EXEC_TIME;
    ktreads[7].run_hw           = &run_steepest_descent_hw;
    ktreads[7].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[7].fifo.consumers   = 2; /* sd_update, hessian */
    ktreads[7].incoming         = 2; /* warp_dx, warp_dy */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[5], &ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0, 0};

    ktreads[8].id               = "sd_update";
    ktreads[8].execution_time   = SD_UPDATE_EXEC_TIME;
    ktreads[8].run_hw           = &run_sd_update_hw;
    ktreads[8].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[8].fifo.consumers   = 1; /* mult */
    ktreads[8].incoming         = 2; /* subtract, steepest_descent */
    ktreads[8].incoming_threads = (kthread_t *[]){&ktreads[4], &ktreads[7]};
    ktreads[8].feedback         = (unsigned[]){0, 0};

    ktreads[9].id               = "hessian_inv";
    ktreads[9].execution_time   = HESSIAN_EXEC_TIME + INVERT_GAUSS_JORDAN_EXEC_TIME;
    ktreads[9].run_hw           = &run_hessian_inv_hw;
    ktreads[9].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[9].fifo.consumers   = 1; /* mult */
    ktreads[9].incoming         = 1; /* steepest_descent */
    ktreads[9].incoming_threads = (kthread_t *[]){&ktreads[7]};
    ktreads[9].feedback         = (unsigned[]){0};

    ktreads[10].id               = "mult";
    ktreads[10].execution_time   = MULT_EXEC_TIME;
    ktreads[10].run_hw           = &run_mult_hw;
    ktreads[10].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[10].fifo.consumers   = 1; /* reshape */
    ktreads[10].incoming         = 2; /* sd_update, hessian_inv */
    ktreads[10].incoming_threads = (kthread_t *[]){&ktreads[8], &ktreads[9]};
    ktreads[10].feedback         = (unsigned[]){0, 0};

    ktreads[11].id               = "reshape";
    ktreads[11].execution_time   = RESHAPE_EXEC_TIME;
    ktreads[11].run_hw           = &run_reshape_hw;
    ktreads[11].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[11].fifo.consumers   = 1; /* add */
    ktreads[11].incoming         = 1; /* mult */
    ktreads[11].incoming_threads = (kthread_t *[]){&ktreads[10]};
    ktreads[11].feedback         = (unsigned[]){0};

    ktreads[12].id               = "add";
    ktreads[12].execution_time   = ADD_EXEC_TIME;
    ktreads[12].run_hw           = &run_add_hw;
    ktreads[12].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[12].fifo.consumers   = 4; /* warp_grayscale, warp_dx, warp_dy, warp_iwxp */
    ktreads[12].incoming         = 1; /* reshape */
    ktreads[12].incoming_threads = (kthread_t *[]){&ktreads[11]};
    ktreads[12].feedback         = (unsigned[]){0};

    ktreads[13].id               = "warp_iwxp";
    ktreads[13].execution_time   = WARP_IWXP_EXEC_TIME;
    ktreads[13].run_hw           = &run_warp_iwxp_hw;
    ktreads[13].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[13].fifo.consumers   = 2; /* subtract, change_detection */
    ktreads[13].incoming         = 2; /* grayscale, add */
    ktreads[13].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[12]};
    ktreads[13].feedback         = (unsigned[]){0, 0};

    ktreads[14].id               = "change_detection";
    ktreads[14].execution_time   = CHANGE_DETECTION_EXEC_TIME;
    ktreads[14].run_hw           = &run_change_detection_hw;
    ktreads[14].fifo.capacity    = frames_num;
    ktreads[14].fifo.consumers   = 0; /* SINK! */
    ktreads[14].incoming         = 1;
    ktreads[14].incoming_threads = (kthread_t *[]){&ktreads[13]};
    ktreads[14].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_COMBO2, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_COMBO2, frames_num);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_COMBO2; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }

    for (i = 0; i < KERNELS_NUM_COMBO2; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;

    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_COMBO2);
  return 0;
#endif

    // dump_dgraph(ktreads, KERNELS_NUM_COMBO2, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_COMBO2; i++) {
        // fprintf(stderr, "..creating thread: %d\n", i);
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);
    }
    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_COMBO2)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    mon_args.read_mode = ESP_MON_READ_ALL;
    esp_monitor(mon_args, &vals_start);

    gettime(&t_test_multi_thread_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_COMBO2; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_COMBO2);

    gettime(&t_test_multi_thread_2);

    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_mtsm_combo2_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    fclose(fp);

    // [humu]: disable validate for now
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_multi_thread_1, &t_test_multi_thread_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_mtsm_combo2 x%d: %llu (ns)\n", frames_num, time_s);
    fprintf(log_file_combo2, "test_mtsm_combo2, %d, %llu\n", frames_num, time_s);
    printf("-------------------------------------------------------\n");

    return 0;
}

int test_mtsm_combo3(token_t *buf, int frames_num)
{
    //
    // This function tests multi-thread shared memory
    //

    cfg_wami_debayer3_indep[0].hw_buf   = buf;
    cfg_wami_grayscale3_indep[0].hw_buf = buf;
    cfg_wami_gradient3_indep[0].hw_buf  = buf;
    cfg_wami_warp_img3_indep[0].hw_buf  = buf;
    cfg_wami_sub3_indep[0].hw_buf       = buf;
    cfg_wami_warp_x3_indep[0].hw_buf    = buf;
    cfg_wami_warp_y3_indep[0].hw_buf    = buf;
    // cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    // cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent_hessian_indep[0].hw_buf = buf;
    cfg_wami_inv3_indep[0].hw_buf                  = buf;
    cfg_wami_sd_update3_indep[0].hw_buf            = buf;
    cfg_wami_mult3_indep[0].hw_buf                 = buf;
    cfg_wami_reshape3_indep[0].hw_buf              = buf;
    cfg_wami_add3_indep[0].hw_buf                  = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf            = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf       = buf;
    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", frames_num);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_COMBO3);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "debayer";
    ktreads[0].execution_time   = DEBAYER_EXEC_TIME;
    ktreads[0].run_hw           = &run_debayer_hw;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* grayscale */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "grayscale";
    ktreads[1].execution_time   = GRAYSCALE_EXEC_TIME;
    ktreads[1].run_hw           = &run_grayscale_hw;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 3; /* gradient, warp_grayscale, warp_iwxp */
    ktreads[1].incoming         = 1; /* debayer */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "gradient";
    ktreads[2].execution_time   = GRADIENT_EXEC_TIME;
    ktreads[2].run_hw           = &run_gradient_hw;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 2; /* warp_dx, warp_dy */
    ktreads[2].incoming         = 1; /* grayscale */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "warp_grayscale";
    ktreads[3].execution_time   = WARP_GRAYSCALE_EXEC_TIME;
    ktreads[3].run_hw           = &run_warp_grayscale_hw;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* subtract */
    ktreads[3].incoming         = 2; /* grayscale, add */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[12]};
    ktreads[3].feedback         = (unsigned[]){0, 1};

    ktreads[4].id               = "subtract";
    ktreads[4].execution_time   = SUBTRACT_EXEC_TIME;
    ktreads[4].run_hw           = &run_subtract_hw;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* sd_update */
    ktreads[4].incoming         = 2; /* warp_grayscale, warp_iwxp */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[13]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "warp_dx";
    ktreads[5].execution_time   = WARP_DX_EXEC_TIME;
    ktreads[5].run_hw           = &run_warp_dx_hw;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* steepest_descent */
    ktreads[5].incoming         = 2; /* gradient, add */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[12]};
    ktreads[5].feedback         = (unsigned[]){0, 1};

    ktreads[6].id               = "warp_dy";
    ktreads[6].execution_time   = WARP_DY_EXEC_TIME;
    ktreads[6].run_hw           = &run_warp_dy_hw;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 1; /* steepest_descent */
    ktreads[6].incoming         = 2; /* gradient, add */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[12]};
    ktreads[6].feedback         = (unsigned[]){0, 1};

    ktreads[7].id               = "steep_descent_hessian";
    ktreads[7].execution_time   = STEEPEST_DESCENT_EXEC_TIME + HESSIAN_EXEC_TIME;
    ktreads[7].run_hw           = &run_steep_descent_hessian_hw;
    ktreads[7].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[7].fifo.consumers   = 2; /* sd_update, hessian */
    ktreads[7].incoming         = 2; /* warp_dx, warp_dy */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[5], &ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0, 0};

    ktreads[8].id               = "sd_update";
    ktreads[8].execution_time   = SD_UPDATE_EXEC_TIME;
    ktreads[8].run_hw           = &run_sd_update_hw;
    ktreads[8].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[8].fifo.consumers   = 1; /* mult */
    ktreads[8].incoming         = 2; /* subtract, steep_descent_hessian */
    ktreads[8].incoming_threads = (kthread_t *[]){&ktreads[4], &ktreads[7]};
    ktreads[8].feedback         = (unsigned[]){0, 0};

    ktreads[9].id               = "invert_gauss_jordan";
    ktreads[9].execution_time   = INVERT_GAUSS_JORDAN_EXEC_TIME;
    ktreads[9].run_hw           = &run_invert_gauss_jordan_hw;
    ktreads[9].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[9].fifo.consumers   = 1; /* mult */
    ktreads[9].incoming         = 1; /* steep_descent_hessian */
    ktreads[9].incoming_threads = (kthread_t *[]){&ktreads[7]};
    ktreads[9].feedback         = (unsigned[]){0};

    ktreads[10].id               = "mult";
    ktreads[10].execution_time   = MULT_EXEC_TIME;
    ktreads[10].run_hw           = &run_mult_hw;
    ktreads[10].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[10].fifo.consumers   = 1; /* reshape */
    ktreads[10].incoming         = 2; /* sd_update, invert_gauss_jordan */
    ktreads[10].incoming_threads = (kthread_t *[]){&ktreads[8], &ktreads[9]};
    ktreads[10].feedback         = (unsigned[]){0, 0};

    ktreads[11].id               = "reshape";
    ktreads[11].execution_time   = RESHAPE_EXEC_TIME;
    ktreads[11].run_hw           = &run_reshape_hw;
    ktreads[11].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[11].fifo.consumers   = 1; /* add */
    ktreads[11].incoming         = 1; /* mult */
    ktreads[11].incoming_threads = (kthread_t *[]){&ktreads[10]};
    ktreads[11].feedback         = (unsigned[]){0};

    ktreads[12].id               = "add";
    ktreads[12].execution_time   = ADD_EXEC_TIME;
    ktreads[12].run_hw           = &run_add_hw;
    ktreads[12].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[12].fifo.consumers   = 4; /* warp_grayscale, warp_dx, warp_dy, warp_iwxp */
    ktreads[12].incoming         = 1; /* reshape */
    ktreads[12].incoming_threads = (kthread_t *[]){&ktreads[11]};
    ktreads[12].feedback         = (unsigned[]){0};

    ktreads[13].id               = "warp_iwxp";
    ktreads[13].execution_time   = WARP_IWXP_EXEC_TIME;
    ktreads[13].run_hw           = &run_warp_iwxp_hw;
    ktreads[13].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[13].fifo.consumers   = 2; /* subtract, change_detection */
    ktreads[13].incoming         = 2; /* grayscale, add */
    ktreads[13].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[12]};
    ktreads[13].feedback         = (unsigned[]){0, 0};

    ktreads[14].id               = "change_detection";
    ktreads[14].execution_time   = CHANGE_DETECTION_EXEC_TIME;
    ktreads[14].run_hw           = &run_change_detection_hw;
    ktreads[14].fifo.capacity    = frames_num;
    ktreads[14].fifo.consumers   = 0; /* SINK! */
    ktreads[14].incoming         = 1;
    ktreads[14].incoming_threads = (kthread_t *[]){&ktreads[13]};
    ktreads[14].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_COMBO3, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_COMBO3, frames_num);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_COMBO3; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }

    for (i = 0; i < KERNELS_NUM_COMBO3; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;

    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_COMBO3);
  return 0;
#endif

    // dump_dgraph(ktreads, KERNELS_NUM_COMBO3, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_COMBO3; i++) {
        // fprintf(stderr, "..creating thread: %d\n", i);
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);
    }
    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_COMBO3)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_multi_thread_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_COMBO3; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_COMBO3);

    gettime(&t_test_multi_thread_2);

    // [humu]: disable validate for now
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_multi_thread_1, &t_test_multi_thread_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_mtsm_combo3 x%d: %llu (ns)\n", frames_num, time_s);
    fprintf(log_file_combo3, "test_mtsm_combo3, %d, %llu\n", frames_num, time_s);
    printf("-------------------------------------------------------\n");

    return 0;
}

int test_mtsm_combo4(token_t *buf, int frames_num)
{
    //
    // This function tests multi-thread shared memory
    //

    cfg_wami_debayer3_indep[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep[0].hw_buf     = buf;
    cfg_wami_gradient3_indep[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep[0].hw_buf      = buf;
    cfg_wami_sub3_indep[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf = buf;
    cfg_wami_hessian3_indep[0].hw_buf       = buf;
    cfg_wami_inv3_indep[0].hw_buf           = buf;
    cfg_wami_sd_update3_indep[0].hw_buf     = buf;
    // cfg_wami_mult3_indep[0].hw_buf           = buf;
    // cfg_wami_reshape3_indep[0].hw_buf        = buf;
    // cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_mult_reshape_add_indep[0].hw_buf = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf       = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf  = buf;
    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    unsigned i;

    pthread_mutex_init(&printf_mutex, NULL);

    setbuf(stdout, NULL); /* Disable stdout buffering (immediate print) */
    printf("Frame count # %d\n", frames_num);
    printf("Queue capacity # %d\n", QUEUE_CAPACITY);
    printf("Thread number # %d\n", KERNELS_NUM_COMBO4);

    srand(time(NULL));

    printf("KERNEL SIZE %zu\n", sizeof(ktreads));

    memset(ktreads, 0, sizeof(ktreads));

    ktreads[0].id               = "debayer";
    ktreads[0].execution_time   = DEBAYER_EXEC_TIME;
    ktreads[0].run_hw           = &run_debayer_hw;
    ktreads[0].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[0].fifo.consumers   = 1; /* grayscale */
    ktreads[0].incoming         = 0; /* SOURCE! */
    ktreads[0].incoming_threads = (kthread_t *[]){};
    ktreads[0].feedback         = (unsigned[]){};

    ktreads[1].id               = "grayscale";
    ktreads[1].execution_time   = GRAYSCALE_EXEC_TIME;
    ktreads[1].run_hw           = &run_grayscale_hw;
    ktreads[1].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[1].fifo.consumers   = 3; /* gradient, warp_grayscale, warp_iwxp */
    ktreads[1].incoming         = 1; /* debayer */
    ktreads[1].incoming_threads = (kthread_t *[]){&ktreads[0]};
    ktreads[1].feedback         = (unsigned[]){0};

    ktreads[2].id               = "gradient";
    ktreads[2].execution_time   = GRADIENT_EXEC_TIME;
    ktreads[2].run_hw           = &run_gradient_hw;
    ktreads[2].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[2].fifo.consumers   = 2; /* warp_dx, warp_dy */
    ktreads[2].incoming         = 1; /* grayscale */
    ktreads[2].incoming_threads = (kthread_t *[]){&ktreads[1]};
    ktreads[2].feedback         = (unsigned[]){0};

    ktreads[3].id               = "warp_grayscale";
    ktreads[3].execution_time   = WARP_GRAYSCALE_EXEC_TIME;
    ktreads[3].run_hw           = &run_warp_grayscale_hw;
    ktreads[3].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[3].fifo.consumers   = 1; /* subtract */
    ktreads[3].incoming         = 2; /* grayscale, mult_reshape_add */
    ktreads[3].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[11]};
    ktreads[3].feedback         = (unsigned[]){0, 1};

    ktreads[4].id               = "subtract";
    ktreads[4].execution_time   = SUBTRACT_EXEC_TIME;
    ktreads[4].run_hw           = &run_subtract_hw;
    ktreads[4].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[4].fifo.consumers   = 1; /* sd_update */
    ktreads[4].incoming         = 2; /* warp_grayscale, warp_iwxp */
    ktreads[4].incoming_threads = (kthread_t *[]){&ktreads[3], &ktreads[12]};
    ktreads[4].feedback         = (unsigned[]){0, 1};

    ktreads[5].id               = "warp_dx";
    ktreads[5].execution_time   = WARP_DX_EXEC_TIME;
    ktreads[5].run_hw           = &run_warp_dx_hw;
    ktreads[5].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[5].fifo.consumers   = 1; /* steepest_descent */
    ktreads[5].incoming         = 2; /* gradient, mult_reshape_add */
    ktreads[5].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[11]};
    ktreads[5].feedback         = (unsigned[]){0, 1};

    ktreads[6].id               = "warp_dy";
    ktreads[6].execution_time   = WARP_DY_EXEC_TIME;
    ktreads[6].run_hw           = &run_warp_dy_hw;
    ktreads[6].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[6].fifo.consumers   = 1; /* steepest_descent */
    ktreads[6].incoming         = 2; /* gradient, mult_reshape_add */
    ktreads[6].incoming_threads = (kthread_t *[]){&ktreads[2], &ktreads[11]};
    ktreads[6].feedback         = (unsigned[]){0, 1};

    ktreads[7].id               = "steepest_descent";
    ktreads[7].execution_time   = STEEPEST_DESCENT_EXEC_TIME;
    ktreads[7].run_hw           = &run_steepest_descent_hw;
    ktreads[7].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[7].fifo.consumers   = 2; /* sd_update, hessian */
    ktreads[7].incoming         = 2; /* warp_dx, warp_dy */
    ktreads[7].incoming_threads = (kthread_t *[]){&ktreads[5], &ktreads[6]};
    ktreads[7].feedback         = (unsigned[]){0, 0};

    ktreads[8].id               = "sd_update";
    ktreads[8].execution_time   = SD_UPDATE_EXEC_TIME;
    ktreads[8].run_hw           = &run_sd_update_hw;
    ktreads[8].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[8].fifo.consumers   = 1; /* mult */
    ktreads[8].incoming         = 2; /* subtract, steepest_descent */
    ktreads[8].incoming_threads = (kthread_t *[]){&ktreads[4], &ktreads[7]};
    ktreads[8].feedback         = (unsigned[]){0, 0};

    ktreads[9].id               = "hessian";
    ktreads[9].execution_time   = HESSIAN_EXEC_TIME;
    ktreads[9].run_hw           = &run_hessian_hw;
    ktreads[9].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[9].fifo.consumers   = 1; /* invert_gauss_jordan */
    ktreads[9].incoming         = 1; /* steepest_descent */
    ktreads[9].incoming_threads = (kthread_t *[]){&ktreads[7]};
    ktreads[9].feedback         = (unsigned[]){0};

    ktreads[10].id               = "invert_gauss_jordan";
    ktreads[10].execution_time   = INVERT_GAUSS_JORDAN_EXEC_TIME;
    ktreads[10].run_hw           = &run_invert_gauss_jordan_hw;
    ktreads[10].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[10].fifo.consumers   = 1; /* mult */
    ktreads[10].incoming         = 1; /* hessian */
    ktreads[10].incoming_threads = (kthread_t *[]){&ktreads[9]};
    ktreads[10].feedback         = (unsigned[]){0};

    ktreads[11].id               = "mult_reshape_add";
    ktreads[11].execution_time   = MULT_EXEC_TIME + RESHAPE_EXEC_TIME + ADD_EXEC_TIME;
    ktreads[11].run_hw           = &run_mult_reshape_add_hw;
    ktreads[11].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[11].fifo.consumers   = 4; /* warp_grayscale, warp_dx, warp_dy, warp_iwxp */
    ktreads[11].incoming         = 2; /* sd_update, invert_gauss_jordan */
    ktreads[11].incoming_threads = (kthread_t *[]){&ktreads[8], &ktreads[10]};
    ktreads[11].feedback         = (unsigned[]){0, 0};

    ktreads[12].id               = "warp_iwxp";
    ktreads[12].execution_time   = WARP_IWXP_EXEC_TIME;
    ktreads[12].run_hw           = &run_warp_iwxp_hw;
    ktreads[12].fifo.capacity    = QUEUE_CAPACITY;
    ktreads[12].fifo.consumers   = 2; /* subtract, change_detection */
    ktreads[12].incoming         = 2; /* grayscale, mult_reshape_add */
    ktreads[12].incoming_threads = (kthread_t *[]){&ktreads[1], &ktreads[11]};
    ktreads[12].feedback         = (unsigned[]){0, 0};

    ktreads[13].id               = "change_detection";
    ktreads[13].execution_time   = CHANGE_DETECTION_EXEC_TIME;
    ktreads[13].run_hw           = &run_change_detection_hw;
    ktreads[13].fifo.capacity    = frames_num;
    ktreads[13].fifo.consumers   = 0; /* SINK! */
    ktreads[13].incoming         = 1;
    ktreads[13].incoming_threads = (kthread_t *[]){&ktreads[12]};
    ktreads[13].feedback         = (unsigned[]){0};

    /* ***** DO NOT TOUCH UNDER THIS LINE! ***** */

    /* init reset kernel */
    set_reset_kernel(ktreads, KERNELS_NUM_COMBO4, 0);

    /* init number of iterations */
    set_iteration_counters(ktreads, KERNELS_NUM_COMBO4, frames_num);

    /* init counters, mutexes and conditional variables */
    for (i = 0; i < KERNELS_NUM_COMBO4; i++) {
        int                 err;
        pthread_mutexattr_t mta;

        err = pthread_mutexattr_init(&mta);
        if (err) {
            exit(1);
        }
        err = pthread_mutexattr_settype(&mta, PTHREAD_MUTEX_ERRORCHECK);
        if (err) {
            exit(1);
        }

        pthread_mutex_init(&ktreads[i].mutex, &mta);
        pthread_cond_init(&ktreads[i].ready, NULL);
        init_fifo(&ktreads[i]);
        reset_fifo_prev_element_ids(&ktreads[i]);
        pthread_mutexattr_destroy(&mta);
    }

    for (i = 0; i < KERNELS_NUM_COMBO4; i++) {
        init_feedback(&ktreads[i]);
    }
    tcount = 0;

    pthread_mutex_init(&tcount_mutex, NULL);
    pthread_cond_init(&tcount_ready, NULL);

#if 0
  dump_topology(ktreads, KERNELS_NUM_COMBO4);
  return 0;
#endif

    // dump_dgraph(ktreads, KERNELS_NUM_COMBO4, "graph.dot");

    /* create threads */
    for (i = 0; i < KERNELS_NUM_COMBO4; i++) {
        // fprintf(stderr, "..creating thread: %d\n", i);
        pthread_create(&ktreads[i].handle, NULL, thread_function, &ktreads[i]);
    }
    /* wait that all of the threads are up and running */
    pthread_mutex_lock(&tcount_mutex);
    while (tcount < KERNELS_NUM_COMBO4)
        pthread_cond_wait(&tcount_ready, &tcount_mutex);
    tcount = 0;
    pthread_mutex_unlock(&tcount_mutex);

    gettime(&t_test_multi_thread_1);

    /* run! */
    start(ktreads, 0);

    /* wait and dispose of all of the threads */
    for (i = 0; i < KERNELS_NUM_COMBO4; i++)
        pthread_join(ktreads[i].handle, NULL);

    wrap_up(ktreads, KERNELS_NUM_COMBO4);

    gettime(&t_test_multi_thread_2);

    // [humu]: disable validate for now
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_multi_thread_1, &t_test_multi_thread_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_mtsm_combo4 x%d: %llu (ns)\n", frames_num, time_s);
    fprintf(log_file_combo4, "test_mtsm_combo4, %d, %llu\n", frames_num, time_s);
    printf("-------------------------------------------------------\n");

    return 0;
}

//--------------------------------------------------------------------------------------

void run_pv(int test_batch)
{
    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    gettime(&t_sw_1);
    for (int bb = 0; bb < test_batch; bb++) {

        // Debayer - compute golden outputs
        // printf("Debayer output: # %d pixels\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_debayer_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * nRows * nCols;
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
        }
        gettime(&t_debayer_2);
        // printf("Debayer golden output succcesfully computed.\n");

        // Grayscale - compute golden outputs
        // printf("Grayscale output: # %d pixels\n",
        // GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_grayscale_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD,
                               nCols - 2 * PAD);
        }
        gettime(&t_grayscale_2);
        // printf("Grayscale golden output succcesfully computed.\n");

        // Gradient - compute golden outputs
        // printf("Gradient output: #  %d pixels\n", GRADIENT_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_gradient_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                         golden_grad_y + out_offset);
        }
        gettime(&t_gradient_2);
        // printf("Gradient golden output succcesfully computed.\n");

        // Warp-img - compute golden outputs
        // printf("Warp-img output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_warp_img_1);
        for (i = 0; i < 6; i++)
            affine_warp[i] = 0.0;
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_img + out_offset);
        }
        gettime(&t_warp_img_2);
        // printf("Warp-img golden output succcesfully computed.\n");

        // Subtract - compute golden outputs
        // printf("Subtract output: # %d pixels\n", SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_subtract_1);
        for (img = 0; img < batch; img++) {
            //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

            __subtract(IWxp,
                       golden_warp_img + out_offset, // TODO????
                       nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
        }
        gettime(&t_subtract_2);
        // printf("Subtract golden output succcesfully computed.\n");

        // Warp-dx - compute golden outputs
        // printf("Warp-dx output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_warp_x_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_dx + out_offset);
        }
        gettime(&t_warp_x_2);
        // printf("Warp-dx golden output succcesfully computed.\n");

        // Warp-dy - compute golden outputs
        // printf("Warp-dy output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_warp_y_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_dy + out_offset);
        }
        gettime(&t_warp_y_2);
        // printf("Warp-dy golden output succcesfully computed.\n");

        // Steepest descent - compute golden outputs
        // printf("Steepest descent output: # %d pixels\n",
        // STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_steep_descent_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                               golden_I_steepest + out_offset);
        }
        gettime(&t_steep_descent_2);
        // printf("Steepest descent golden output succcesfully computed.\n");

        // Hessian - compute golden outputs
        // printf("Hessian output: # %d pixels\n", HESSIAN_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_hessian_inv_1);
        gettime(&t_hessian_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 36;
            __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
        }
        gettime(&t_hessian_2);
        // printf("Hessian golden output succcesfully computed.\n");

        // Invert Gauss Jordan - compute golden outputs
        // printf("Invert Gauss Jordan output: # %d pixels\n",
        // INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_inv_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 36;
            __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6,
                                  golden_hess_inv + out_offset);
        }
        gettime(&t_inv_2);
        // gettime(&t_hessian_inv_2);
        // printf("Invert Gauss Jordan golden output succcesfully computed.\n");

        // SD update - compute golden outputs
        // printf("SD update output: # %d pixels\n",
        // SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_sd_update_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 6;
            __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD,
                        nCols - 2 * PAD, golden_sd_delta_p + out_offset);
        }
        gettime(&t_sd_update_2);
        // printf("SD update golden output succcesfully computed.\n");

        // Mult - compute golden outputs
        // printf("Mult output: # %d pixels\n", MULT_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_mult_1);
        for (img = 0; img < batch; img++) {
            in_offset_1 = img * 36;
            in_offset_2 = img * 6;
            out_offset  = img * 6;
            __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6,
                   golden_delta_p + out_offset);
        }
        gettime(&t_mult_2);
        // printf("Mult golden output succcesfully computed.\n");

        // Reshape - compute golden outputs
        // printf("Reshape output: # %d pixels\n", RESHAPE_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_reshape_add_1);
        gettime(&t_reshape_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * 6;
            out_offset = img * 6;
            __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
        }
        gettime(&t_reshape_2);
        //  printf("Reshape golden output succcesfully computed.\n");

        // Add - compute golden outputs
        // printf("Add output: # %d pixels\n", ADD_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_add_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * 6;
            out_offset = img * 6;
            __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
        }
        gettime(&t_add_2);
        // gettime(&t_reshape_add_2);
        // printf("Add golden output succcesfully computed.\n");

        // Warp-iwxp - compute golden outputs
        // printf("Warp-iwxp output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        gettime(&t_warp_iwxp_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                         golden_warp_iwxp + out_offset);
        }
        gettime(&t_warp_iwxp_2);
        // printf("Warp-dy golden output succcesfully computed.\n");

        // Change detection - compute golden outputs
        // printf("Change detection output: # %d pixels\n",
        // CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);

        // cast from float to usigned short (16 bits)
        gettime(&t_gmm_1);
        for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
            golden_gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
        }
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                       golden_foreground + out_offset, golden_gmm_img + in_offset);
        }
        gettime(&t_gmm_2);
        // printf("Change detection output succcesfully computed.\n");

    } // test_batch forloop
    gettime(&t_sw_2);

    time_s = ts_subtract(&t_sw_1, &t_sw_2);
    printf("--> time of run_pv with batch = %d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_pv, "run_pv, %d, %llu\n", test_batch, time_s);
}

void run_pv_no_batchloop(int test_batch)
{
    // printf("------- run_pv_no_batchloop, start. test_batch: %d\n", test_batch);

    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    gettime(&t_sw_1);
    // for (int bb = 0; bb < test_batch; bb++) {

    // Debayer - compute golden outputs
    // printf("Debayer output: # %d pixels\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_debayer_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * nRows * nCols;
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
    }
    gettime(&t_debayer_2);
    // printf("Debayer golden output succcesfully computed.\n");

    // Grayscale - compute golden outputs
    // printf("Grayscale output: # %d pixels\n",
    // GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_grayscale_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD, nCols - 2 * PAD);
    }
    gettime(&t_grayscale_2);
    // printf("Grayscale golden output succcesfully computed.\n");

    // Gradient - compute golden outputs
    // printf("Gradient output: #  %d pixels\n", GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_gradient_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                     golden_grad_y + out_offset);
    }
    gettime(&t_gradient_2);
    // printf("Gradient golden output succcesfully computed.\n");

    // Warp-img - compute golden outputs
    // printf("Warp-img output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_warp_img_1);
    for (i = 0; i < 6; i++)
        affine_warp[i] = 0.0;
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_img + out_offset);
    }
    gettime(&t_warp_img_2);
    // printf("Warp-img golden output succcesfully computed.\n");

    // Subtract - compute golden outputs
    // printf("Subtract output: # %d pixels\n", SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_subtract_1);
    for (img = 0; img < batch; img++) {
        //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

        __subtract(IWxp,
                   golden_warp_img + out_offset, // TODO????
                   nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
    }
    gettime(&t_subtract_2);
    // printf("Subtract golden output succcesfully computed.\n");

    // Warp-dx - compute golden outputs
    // printf("Warp-dx output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_warp_x_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dx + out_offset);
    }
    gettime(&t_warp_x_2);
    // printf("Warp-dx golden output succcesfully computed.\n");

    // Warp-dy - compute golden outputs
    // printf("Warp-dy output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_warp_y_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dy + out_offset);
    }
    gettime(&t_warp_y_2);
    // printf("Warp-dy golden output succcesfully computed.\n");

    // Steepest descent - compute golden outputs
    // printf("Steepest descent output: # %d pixels\n",
    // STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_steep_descent_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                           golden_I_steepest + out_offset);
    }
    gettime(&t_steep_descent_2);
    // printf("Steepest descent golden output succcesfully computed.\n");

    // Hessian - compute golden outputs
    // printf("Hessian output: # %d pixels\n", HESSIAN_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_hessian_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
    }
    gettime(&t_hessian_2);
    // printf("Hessian golden output succcesfully computed.\n");

    // Invert Gauss Jordan - compute golden outputs
    // printf("Invert Gauss Jordan output: # %d pixels\n",
    // INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_inv_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6, golden_hess_inv + out_offset);
    }
    gettime(&t_inv_2);
    // printf("Invert Gauss Jordan golden output succcesfully computed.\n");

    // SD update - compute golden outputs
    // printf("SD update output: # %d pixels\n",
    // SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_sd_update_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6;
        __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD, nCols - 2 * PAD,
                    golden_sd_delta_p + out_offset);
    }
    gettime(&t_sd_update_2);
    // printf("SD update golden output succcesfully computed.\n");

    // Mult - compute golden outputs
    // printf("Mult output: # %d pixels\n", MULT_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_mult_1);
    for (img = 0; img < batch; img++) {
        in_offset_1 = img * 36;
        in_offset_2 = img * 6;
        out_offset  = img * 6;
        __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6, golden_delta_p + out_offset);
    }
    gettime(&t_mult_2);
    // printf("Mult golden output succcesfully computed.\n");

    // Reshape - compute golden outputs
    // printf("Reshape output: # %d pixels\n", RESHAPE_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_reshape_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
    }
    gettime(&t_reshape_2);
    // printf("Reshape golden output succcesfully computed.\n");

    // Add - compute golden outputs
    // printf("Add output: # %d pixels\n", ADD_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_add_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
    }
    gettime(&t_add_2);
    // printf("Add golden output succcesfully computed.\n");

    // Warp-iwxp - compute golden outputs
    // printf("Warp-iwxp output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    gettime(&t_warp_iwxp_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                     golden_warp_iwxp + out_offset);
    }
    gettime(&t_warp_iwxp_2);
    // printf("Warp-dy golden output succcesfully computed.\n");

    //} // batch forloop
    gettime(&t_sw_2);

    // Change detection - compute golden outputs
    // printf("Change detection output: # %d pixels\n",
    // CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);

    // cast from float to usigned short (16 bits)
    for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
        golden_gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
    }
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                   golden_foreground + out_offset, golden_gmm_img + in_offset);
    }
    // printf("Change detection output succcesfully computed.\n");

    time_s = ts_subtract(&t_sw_1, &t_sw_2);
    printf("--> time of run_pv_no_batchloop x1: %llu (ns)\n", time_s);
    fprintf(log_file_pv, "run_pv_no_batchloop, %d, %llu\n", test_batch, time_s);
}

void run_pv_no_individual_gettime(int test_batch)
{
    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    gettime(&t_sw_1);
    for (int bb = 0; bb < test_batch; bb++) {

        // Debayer - compute golden outputs
        // printf("Debayer output: # %d pixels\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_debayer_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * nRows * nCols;
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
        }
        // gettime(&t_debayer_2);
        // printf("Debayer golden output succcesfully computed.\n");

        // Grayscale - compute golden outputs
        // printf("Grayscale output: # %d pixels\n",
        // GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_grayscale_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD,
                               nCols - 2 * PAD);
        }
        // gettime(&t_grayscale_2);
        // printf("Grayscale golden output succcesfully computed.\n");

        // Gradient - compute golden outputs
        // printf("Gradient output: #  %d pixels\n", GRADIENT_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_gradient_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                         golden_grad_y + out_offset);
        }
        // gettime(&t_gradient_2);
        // printf("Gradient golden output succcesfully computed.\n");

        // Warp-img - compute golden outputs
        // printf("Warp-img output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_warp_img_1);
        for (i = 0; i < 6; i++)
            affine_warp[i] = 0.0;
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_img + out_offset);
        }
        // gettime(&t_warp_img_2);
        // printf("Warp-img golden output succcesfully computed.\n");

        // Subtract - compute golden outputs
        // printf("Subtract output: # %d pixels\n", SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_subtract_1);
        for (img = 0; img < batch; img++) {
            //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

            __subtract(IWxp,
                       golden_warp_img + out_offset, // TODO????
                       nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
        }
        // gettime(&t_subtract_2);
        // printf("Subtract golden output succcesfully computed.\n");

        // Warp-dx - compute golden outputs
        // printf("Warp-dx output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_warp_x_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_dx + out_offset);
        }
        // gettime(&t_warp_x_2);
        // printf("Warp-dx golden output succcesfully computed.\n");

        // Warp-dy - compute golden outputs
        // printf("Warp-dy output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_warp_y_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_dy + out_offset);
        }
        // gettime(&t_warp_y_2);
        // printf("Warp-dy golden output succcesfully computed.\n");

        // Steepest descent - compute golden outputs
        // printf("Steepest descent output: # %d pixels\n",
        // STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_steep_descent_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                               golden_I_steepest + out_offset);
        }
        // gettime(&t_steep_descent_2);
        // printf("Steepest descent golden output succcesfully computed.\n");

        // Hessian - compute golden outputs
        // printf("Hessian output: # %d pixels\n", HESSIAN_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_hessian_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 36;
            __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
        }
        // gettime(&t_hessian_2);
        // printf("Hessian golden output succcesfully computed.\n");

        // Invert Gauss Jordan - compute golden outputs
        // printf("Invert Gauss Jordan output: # %d pixels\n",
        // INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_inv_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 36;
            __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6,
                                  golden_hess_inv + out_offset);
        }
        // gettime(&t_inv_2);
        // printf("Invert Gauss Jordan golden output succcesfully computed.\n");

        // SD update - compute golden outputs
        // printf("SD update output: # %d pixels\n",
        // SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_sd_update_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 6;
            __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD,
                        nCols - 2 * PAD, golden_sd_delta_p + out_offset);
        }
        // gettime(&t_sd_update_2);
        // printf("SD update golden output succcesfully computed.\n");

        // Mult - compute golden outputs
        // printf("Mult output: # %d pixels\n", MULT_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_mult_1);
        for (img = 0; img < batch; img++) {
            in_offset_1 = img * 36;
            in_offset_2 = img * 6;
            out_offset  = img * 6;
            __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6,
                   golden_delta_p + out_offset);
        }
        // gettime(&t_mult_2);
        // printf("Mult golden output succcesfully computed.\n");

        // Reshape - compute golden outputs
        // printf("Reshape output: # %d pixels\n", RESHAPE_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_reshape_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * 6;
            out_offset = img * 6;
            __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
        }
        // gettime(&t_reshape_2);
        // printf("Reshape golden output succcesfully computed.\n");

        // Add - compute golden outputs
        // printf("Add output: # %d pixels\n", ADD_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_add_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * 6;
            out_offset = img * 6;
            __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
        }
        // gettime(&t_add_2);
        // printf("Add golden output succcesfully computed.\n");

        // Warp-iwxp - compute golden outputs
        // printf("Warp-iwxp output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
        // gettime(&t_warp_iwxp_1);
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                         golden_warp_iwxp + out_offset);
        }
        // gettime(&t_warp_iwxp_2);
        // printf("Warp-dy golden output succcesfully computed.\n");

    } // test_batch forloop
    gettime(&t_sw_2);

    // Change detection - compute golden outputs
    // printf("Change detection output: # %d pixels\n",
    // CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);

    // cast from float to usigned short (16 bits)
    for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
        golden_gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
    }
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                   golden_foreground + out_offset, golden_gmm_img + in_offset);
    }
    // printf("Change detection output succcesfully computed.\n");

    time_s = ts_subtract(&t_sw_1, &t_sw_2);
    printf("--> time of run_pv_no_individual_gettime x%d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_pv, "run_pv_no_individual_gettime, %d, %llu\n", test_batch, time_s);
}

void run_pv_no_individual_gettime_no_batchloop(int test_batch)
{
    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    gettime(&t_sw_1);
    // for (int bb = 0; bb < test_batch; bb++) {

    // Debayer - compute golden outputs
    // printf("Debayer output: # %d pixels\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_debayer_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * nRows * nCols;
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
    }
    // gettime(&t_debayer_2);
    // printf("Debayer golden output succcesfully computed.\n");

    // Grayscale - compute golden outputs
    // printf("Grayscale output: # %d pixels\n",
    // GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_grayscale_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD, nCols - 2 * PAD);
    }
    // gettime(&t_grayscale_2);
    // printf("Grayscale golden output succcesfully computed.\n");

    // Gradient - compute golden outputs
    // printf("Gradient output: #  %d pixels\n", GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_gradient_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                     golden_grad_y + out_offset);
    }
    // gettime(&t_gradient_2);
    // printf("Gradient golden output succcesfully computed.\n");

    // Warp-img - compute golden outputs
    // printf("Warp-img output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_warp_img_1);
    for (i = 0; i < 6; i++)
        affine_warp[i] = 0.0;
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_img + out_offset);
    }
    // gettime(&t_warp_img_2);
    // printf("Warp-img golden output succcesfully computed.\n");

    // Subtract - compute golden outputs
    // printf("Subtract output: # %d pixels\n", SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_subtract_1);
    for (img = 0; img < batch; img++) {
        //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

        __subtract(IWxp,
                   golden_warp_img + out_offset, // TODO????
                   nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
    }
    // gettime(&t_subtract_2);
    // printf("Subtract golden output succcesfully computed.\n");

    // Warp-dx - compute golden outputs
    // printf("Warp-dx output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_warp_x_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dx + out_offset);
    }
    // gettime(&t_warp_x_2);
    // printf("Warp-dx golden output succcesfully computed.\n");

    // Warp-dy - compute golden outputs
    // printf("Warp-dy output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_warp_y_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dy + out_offset);
    }
    // gettime(&t_warp_y_2);
    // printf("Warp-dy golden output succcesfully computed.\n");

    // Steepest descent - compute golden outputs
    // printf("Steepest descent output: # %d pixels\n",
    // STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_steep_descent_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                           golden_I_steepest + out_offset);
    }
    // gettime(&t_steep_descent_2);
    // printf("Steepest descent golden output succcesfully computed.\n");

    // Hessian - compute golden outputs
    // printf("Hessian output: # %d pixels\n", HESSIAN_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_hessian_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
    }
    // gettime(&t_hessian_2);
    // printf("Hessian golden output succcesfully computed.\n");

    // Invert Gauss Jordan - compute golden outputs
    // printf("Invert Gauss Jordan output: # %d pixels\n",
    // INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_inv_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6, golden_hess_inv + out_offset);
    }
    // gettime(&t_inv_2);
    // printf("Invert Gauss Jordan golden output succcesfully computed.\n");

    // SD update - compute golden outputs
    // printf("SD update output: # %d pixels\n",
    // SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_sd_update_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6;
        __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD, nCols - 2 * PAD,
                    golden_sd_delta_p + out_offset);
    }
    // gettime(&t_sd_update_2);
    // printf("SD update golden output succcesfully computed.\n");

    // Mult - compute golden outputs
    // printf("Mult output: # %d pixels\n", MULT_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_mult_1);
    for (img = 0; img < batch; img++) {
        in_offset_1 = img * 36;
        in_offset_2 = img * 6;
        out_offset  = img * 6;
        __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6, golden_delta_p + out_offset);
    }
    // gettime(&t_mult_2);
    // printf("Mult golden output succcesfully computed.\n");

    // Reshape - compute golden outputs
    // printf("Reshape output: # %d pixels\n", RESHAPE_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_reshape_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
    }
    // gettime(&t_reshape_2);
    // printf("Reshape golden output succcesfully computed.\n");

    // Add - compute golden outputs
    // printf("Add output: # %d pixels\n", ADD_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_add_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
    }
    // gettime(&t_add_2);
    // printf("Add golden output succcesfully computed.\n");

    // Warp-iwxp - compute golden outputs
    // printf("Warp-iwxp output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    // gettime(&t_warp_iwxp_1);
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                     golden_warp_iwxp + out_offset);
    }
    // gettime(&t_warp_iwxp_2);
    // printf("Warp-dy golden output succcesfully computed.\n");

    //} // batch forloop
    gettime(&t_sw_2);

    // Change detection - compute golden outputs
    // printf("Change detection output: # %d pixels\n",
    // CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);

    // cast from float to usigned short (16 bits)
    for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
        golden_gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
    }
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                   golden_foreground + out_offset, golden_gmm_img + in_offset);
    }
    // printf("Change detection output succcesfully computed.\n");

    time_s = ts_subtract(&t_sw_1, &t_sw_2);
    printf("--> time of run_pv_no_individual_gettime_no_batchloop x1: %llu (ns)\n", time_s);
    fprintf(log_file_pv, "run_pv_no_individual_gettime_no_batchloop, %d, %llu\n", test_batch, time_s);
}

void test_independent_memory(token_t *buf)
{
    cfg_wami_debayer3_indep[0].hw_buf        = buf;
    cfg_wami_grayscale3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_indep[0].hw_buf       = buf;
    cfg_wami_warp_img3_indep[0].hw_buf       = buf;
    cfg_wami_sub3_indep[0].hw_buf            = buf;
    cfg_wami_warp_x3_indep[0].hw_buf         = buf;
    cfg_wami_warp_y3_indep[0].hw_buf         = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    const int          MEM_TILE_IDX = 0;
    // mon_args.read_mode = ESP_MON_READ_SINGLE;
    mon_args.read_mode  = ESP_MON_READ_ALL;
    mon_args.tile_index = MEM_TILE_IDX;
    mon_args.mon_index  = MON_DDR_WORD_TRANSFER_INDEX;

    unsigned int ddr_accesses_start, ddr_accesses_end;
    unsigned int ddr_accesses_diff;

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // esp_monitor(mon_args, &vals_start);

    gettime(&t_test_indep_mem_1);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_debayer3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_debayer3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_grayscale3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_grayscale3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_gradient3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_gradient3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_warp_img3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_warp_img3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_sub3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_sub3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_warp_x3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_warp_x3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_warp_y3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_warp_y3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_steep_descent3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_steep_descent3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_hessian3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_hessian3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_inv3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_inv3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_sd_update3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_sd_update3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_mult3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_mult3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_reshape3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_reshape3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_add3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_add3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_warp_iwxp3 indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_warp_iwxp3_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    // printf("--- wami_gradient3_dummy indep --- \n");
    esp_monitor(mon_args, &vals_start);
    esp_run(cfg_wami_gradient3_dummy_indep, 1);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);

    gettime(&t_test_indep_mem_2);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    // esp_monitor(mon_args, &vals_end);
    // vals_diff = esp_monitor_diff(vals_start, vals_end);
    // FILE *fp  = fopen("test_indep_mem_esp_mon_all_1.txt", "w");
    // esp_monitor_print(mon_args, vals_diff, fp);
    // fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_indep_mem_1, &t_test_indep_mem_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory: %llu (ns)\n", time_s);
    // fprintf(log_file, "test_independent_memory, %d, %llu\n", 1, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_1_no_thread(token_t *buf)
{
    cfg_wami_debayer3_indep[0].hw_buf        = buf;
    cfg_wami_grayscale3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_indep[0].hw_buf       = buf;
    cfg_wami_warp_img3_indep[0].hw_buf       = buf;
    cfg_wami_sub3_indep[0].hw_buf            = buf;
    cfg_wami_warp_x3_indep[0].hw_buf         = buf;
    cfg_wami_warp_y3_indep[0].hw_buf         = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;
    cfg_wami_gmm3_indep[0].hw_buf            = buf;

    cfg_wami_reshape_add_indep[0].hw_buf           = buf;
    cfg_wami_hessian_inv_indep[0].hw_buf           = buf;
    cfg_wami_steep_descent_hessian_indep[0].hw_buf = buf;
    cfg_wami_mult_reshape_add_indep[0].hw_buf      = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_indep_mem_1);
    // esp_run_1_no_thread(cfg_wami_debayer3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_grayscale3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_gradient3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_img3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_sub3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_x3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_y3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_steep_descent3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_hessian3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_inv3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_sd_update3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_mult3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_reshape3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_add3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_warp_iwxp3_indep, 1);
    // esp_run_1_no_thread(cfg_wami_gradient3_dummy_indep, 1);
    // esp_run_1_no_thread(cfg_wami_gmm3_indep, 1);

    esp_run(cfg_wami_debayer3_indep, 1);
    esp_run(cfg_wami_grayscale3_indep, 1);
    esp_run(cfg_wami_gradient3_indep, 1);
    esp_run(cfg_wami_warp_img3_indep, 1);
    esp_run(cfg_wami_sub3_indep, 1);
    esp_run(cfg_wami_warp_x3_indep, 1);
    esp_run(cfg_wami_warp_y3_indep, 1);
    esp_run(cfg_wami_steep_descent3_indep, 1);
    esp_run(cfg_wami_hessian3_indep, 1);
    esp_run(cfg_wami_inv3_indep, 1);
    esp_run(cfg_wami_sd_update3_indep, 1);
    esp_run(cfg_wami_mult3_indep, 1);
    esp_run(cfg_wami_reshape3_indep, 1);
    esp_run(cfg_wami_add3_indep, 1);
    esp_run(cfg_wami_warp_iwxp3_indep, 1);
    esp_run(cfg_wami_gradient3_dummy_indep, 1);

    printf("-------------------------------------------------------\n");
    esp_run(cfg_wami_gmm3_indep, 1);
    printf("-------------------------------------------------------\n");

    gettime(&t_test_indep_mem_2);

    // esp_run_1_no_thread(cfg_wami_reshape_add_indep, 1);
    // esp_run_1_no_thread(cfg_wami_hessian_inv_indep, 1);
    // esp_run_1_no_thread(cfg_wami_steep_descent_hessian_indep, 1);
    // esp_run_1_no_thread(cfg_wami_mult_reshape_add_indep, 1);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_indep_mem_1, &t_test_indep_mem_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_1_no_thread: %llu (ns)\n", time_s);
    // fprintf(log_file, "test_independent_memory_1_no_thread, %d, %llu\n", 1, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_no_print(token_t *buf)
{
    cfg_wami_debayer3_indep[0].hw_buf        = buf;
    cfg_wami_grayscale3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_indep[0].hw_buf       = buf;
    cfg_wami_warp_img3_indep[0].hw_buf       = buf;
    cfg_wami_sub3_indep[0].hw_buf            = buf;
    cfg_wami_warp_x3_indep[0].hw_buf         = buf;
    cfg_wami_warp_y3_indep[0].hw_buf         = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_indep_mem_1);
    esp_run_no_print(cfg_wami_debayer3_indep, 1);
    esp_run_no_print(cfg_wami_grayscale3_indep, 1);
    esp_run_no_print(cfg_wami_gradient3_indep, 1);
    esp_run_no_print(cfg_wami_warp_img3_indep, 1);
    esp_run_no_print(cfg_wami_sub3_indep, 1);
    esp_run_no_print(cfg_wami_warp_x3_indep, 1);
    esp_run_no_print(cfg_wami_warp_y3_indep, 1);
    esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
    esp_run_no_print(cfg_wami_hessian3_indep, 1);
    esp_run_no_print(cfg_wami_inv3_indep, 1);
    esp_run_no_print(cfg_wami_sd_update3_indep, 1);
    esp_run_no_print(cfg_wami_mult3_indep, 1);
    esp_run_no_print(cfg_wami_reshape3_indep, 1);
    esp_run_no_print(cfg_wami_add3_indep, 1);
    esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
    esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
    gettime(&t_test_indep_mem_2);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_indep_mem_1, &t_test_indep_mem_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_no_print: %llu (ns)\n", time_s);
    // fprintf(log_file, "test_independent_memory_no_print, %d, %llu\n", 1, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_real_batch(token_t *buf, int test_batch)
{
    cfg_wami_debayer3_indep[0].hw_buf        = buf;
    cfg_wami_grayscale3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_indep[0].hw_buf       = buf;
    cfg_wami_warp_img3_indep[0].hw_buf       = buf;
    cfg_wami_sub3_indep[0].hw_buf            = buf;
    cfg_wami_warp_x3_indep[0].hw_buf         = buf;
    cfg_wami_warp_y3_indep[0].hw_buf         = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;
    cfg_wami_gmm3_indep[0].hw_buf            = buf;

   
    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    // const int MEM_TILE_IDX = 0;
    // mon_args.read_mode = ESP_MON_READ_SINGLE;
    mon_args.read_mode = ESP_MON_READ_ALL;
    // mon_args.tile_index = MEM_TILE_IDX;
    // mon_args.mon_index = MON_DDR_WORD_TRANSFER_INDEX;

    // unsigned int ddr_accesses_start, ddr_accesses_end;
    // unsigned int ddr_accesses_diff;

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    esp_monitor(mon_args, &vals_start);

    gettime(&t_test_indep_mem_real_batch_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_wami_debayer3_indep, 1);
        esp_run_no_print(cfg_wami_grayscale3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_indep, 1);
        esp_run_no_print(cfg_wami_warp_img3_indep, 1);
        esp_run_no_print(cfg_wami_sub3_indep, 1);
        esp_run_no_print(cfg_wami_warp_x3_indep, 1);
        esp_run_no_print(cfg_wami_warp_y3_indep, 1);
        esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
        esp_run_no_print(cfg_wami_hessian3_indep, 1);
        esp_run_no_print(cfg_wami_inv3_indep, 1);
        esp_run_no_print(cfg_wami_sd_update3_indep, 1);
        esp_run_no_print(cfg_wami_mult3_indep, 1);
        esp_run_no_print(cfg_wami_reshape3_indep, 1);
        esp_run_no_print(cfg_wami_add3_indep, 1);
        esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
        esp_run_no_print(cfg_wami_gmm3_indep, 1);
    }
    gettime(&t_test_indep_mem_real_batch_2);

    // printf("--- wami_gradient3_dummy indep --- \n");

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_indep_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    // esp_monitor_print_mem_access_only(mon_args, vals_diff);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_indep_mem_real_batch_1, &t_test_indep_mem_real_batch_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_real_batch = %d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file, "test_independent_memory_real_batch, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_real_batch_combo1(token_t *buf, int test_batch)
{
    cfg_wami_debayer3_indep[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep[0].hw_buf     = buf;
    cfg_wami_gradient3_indep[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep[0].hw_buf      = buf;
    cfg_wami_sub3_indep[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf = buf;
    cfg_wami_hessian3_indep[0].hw_buf       = buf;
    cfg_wami_inv3_indep[0].hw_buf           = buf;
    cfg_wami_sd_update3_indep[0].hw_buf     = buf;
    cfg_wami_mult3_indep[0].hw_buf          = buf;
    // cfg_wami_reshape3_indep[0].hw_buf        = buf;
    // cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_reshape_add_indep[0].hw_buf     = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_wami_debayer3_indep, 1);
        esp_run_no_print(cfg_wami_grayscale3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_indep, 1);
        esp_run_no_print(cfg_wami_warp_img3_indep, 1);
        esp_run_no_print(cfg_wami_sub3_indep, 1);
        esp_run_no_print(cfg_wami_warp_x3_indep, 1);
        esp_run_no_print(cfg_wami_warp_y3_indep, 1);
        esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
        esp_run_no_print(cfg_wami_hessian3_indep, 1);
        esp_run_no_print(cfg_wami_inv3_indep, 1);
        esp_run_no_print(cfg_wami_sd_update3_indep, 1);
        esp_run_no_print(cfg_wami_mult3_indep, 1);
        // esp_run_no_print(cfg_wami_reshape3_indep, 1);
        // esp_run_no_print(cfg_wami_add3_indep, 1);
        esp_run_no_print(cfg_wami_reshape_add_indep, 1);
        esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
    }
    gettime(&t_test_2);

    // printf("--- wami_gradient3_dummy indep --- \n");

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_real_batch_combo1 = %d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo1, "test_independent_memory_real_batch_combo1, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_real_batch_combo2(token_t *buf, int test_batch)
{
    cfg_wami_debayer3_indep[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep[0].hw_buf     = buf;
    cfg_wami_gradient3_indep[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep[0].hw_buf      = buf;
    cfg_wami_sub3_indep[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf = buf;
    // cfg_wami_hessian3_indep[0].hw_buf       = buf;
    // cfg_wami_inv3_indep[0].hw_buf           = buf;
    cfg_wami_hessian_inv_indep[0].hw_buf     = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    mon_args.read_mode = ESP_MON_READ_ALL;
    esp_monitor(mon_args, &vals_start);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_wami_debayer3_indep, 1);
        esp_run_no_print(cfg_wami_grayscale3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_indep, 1);
        esp_run_no_print(cfg_wami_warp_img3_indep, 1);
        esp_run_no_print(cfg_wami_sub3_indep, 1);
        esp_run_no_print(cfg_wami_warp_x3_indep, 1);
        esp_run_no_print(cfg_wami_warp_y3_indep, 1);
        esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
        // esp_run_no_print(cfg_wami_hessian3_indep, 1);
        // esp_run_no_print(cfg_wami_inv3_indep, 1);
        esp_run_no_print(cfg_wami_hessian_inv_indep, 1);
        esp_run_no_print(cfg_wami_sd_update3_indep, 1);
        esp_run_no_print(cfg_wami_mult3_indep, 1);
        esp_run_no_print(cfg_wami_reshape3_indep, 1);
        esp_run_no_print(cfg_wami_add3_indep, 1);
        esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
    }
    gettime(&t_test_2);

    // printf("--- wami_gradient3_dummy indep --- \n");

    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_indep_combo2_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_real_batch_combo2 = %d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo2, "test_independent_memory_real_batch_combo2, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_real_batch_combo3(token_t *buf, int test_batch)
{
    cfg_wami_debayer3_indep[0].hw_buf   = buf;
    cfg_wami_grayscale3_indep[0].hw_buf = buf;
    cfg_wami_gradient3_indep[0].hw_buf  = buf;
    cfg_wami_warp_img3_indep[0].hw_buf  = buf;
    cfg_wami_sub3_indep[0].hw_buf       = buf;
    cfg_wami_warp_x3_indep[0].hw_buf    = buf;
    cfg_wami_warp_y3_indep[0].hw_buf    = buf;
    // cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    // cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent_hessian_indep[0].hw_buf = buf;
    cfg_wami_inv3_indep[0].hw_buf                  = buf;
    cfg_wami_sd_update3_indep[0].hw_buf            = buf;
    cfg_wami_mult3_indep[0].hw_buf                 = buf;
    cfg_wami_reshape3_indep[0].hw_buf              = buf;
    cfg_wami_add3_indep[0].hw_buf                  = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf            = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf       = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_wami_debayer3_indep, 1);
        esp_run_no_print(cfg_wami_grayscale3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_indep, 1);
        esp_run_no_print(cfg_wami_warp_img3_indep, 1);
        esp_run_no_print(cfg_wami_sub3_indep, 1);
        esp_run_no_print(cfg_wami_warp_x3_indep, 1);
        esp_run_no_print(cfg_wami_warp_y3_indep, 1);
        // esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
        // esp_run_no_print(cfg_wami_hessian3_indep, 1);
        esp_run_no_print(cfg_wami_steep_descent_hessian_indep, 1);
        esp_run_no_print(cfg_wami_inv3_indep, 1);
        esp_run_no_print(cfg_wami_sd_update3_indep, 1);
        esp_run_no_print(cfg_wami_mult3_indep, 1);
        esp_run_no_print(cfg_wami_reshape3_indep, 1);
        esp_run_no_print(cfg_wami_add3_indep, 1);
        esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
    }
    gettime(&t_test_2);

    // printf("--- wami_gradient3_dummy indep --- \n");

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_1, &t_test_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_real_batch_combo3 = %d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo3, "test_independent_memory_real_batch_combo3, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_real_batch_combo4(token_t *buf, int test_batch)
{
    cfg_wami_debayer3_indep[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep[0].hw_buf     = buf;
    cfg_wami_gradient3_indep[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep[0].hw_buf      = buf;
    cfg_wami_sub3_indep[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf = buf;
    cfg_wami_hessian3_indep[0].hw_buf       = buf;
    cfg_wami_inv3_indep[0].hw_buf           = buf;
    cfg_wami_sd_update3_indep[0].hw_buf     = buf;
    // cfg_wami_mult3_indep[0].hw_buf           = buf;
    // cfg_wami_reshape3_indep[0].hw_buf        = buf;
    // cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_mult_reshape_add_indep[0].hw_buf = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf       = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf  = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_indep_mem_real_batch_1);
    for (i = 0; i < test_batch; i++) {
        esp_run_no_print(cfg_wami_debayer3_indep, 1);
        esp_run_no_print(cfg_wami_grayscale3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_indep, 1);
        esp_run_no_print(cfg_wami_warp_img3_indep, 1);
        esp_run_no_print(cfg_wami_sub3_indep, 1);
        esp_run_no_print(cfg_wami_warp_x3_indep, 1);
        esp_run_no_print(cfg_wami_warp_y3_indep, 1);
        esp_run_no_print(cfg_wami_steep_descent3_indep, 1);
        esp_run_no_print(cfg_wami_hessian3_indep, 1);
        esp_run_no_print(cfg_wami_inv3_indep, 1);
        esp_run_no_print(cfg_wami_sd_update3_indep, 1);
        // esp_run_no_print(cfg_wami_mult3_indep, 1);
        // esp_run_no_print(cfg_wami_reshape3_indep, 1);
        // esp_run_no_print(cfg_wami_add3_indep, 1);
        esp_run_no_print(cfg_wami_mult_reshape_add_indep, 1);
        esp_run_no_print(cfg_wami_warp_iwxp3_indep, 1);
        esp_run_no_print(cfg_wami_gradient3_dummy_indep, 1);
    }
    gettime(&t_test_indep_mem_real_batch_2);

    // printf("--- wami_gradient3_dummy indep --- \n");

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_indep_mem_real_batch_1, &t_test_indep_mem_real_batch_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_independent_memory_real_batch_combo4 = %d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo4, "test_independent_memory_real_batch_combo4, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_batch(token_t *buf, int test_batch)
{
    cfg_wami_debayer3_indep_batch[0].hw_buf       = buf;
    cfg_wami_grayscale3_indep_batch[0].hw_buf     = buf;
    cfg_wami_gradient3_indep_batch[0].hw_buf      = buf;
    cfg_wami_warp_img3_indep_batch[0].hw_buf      = buf;
    cfg_wami_sub3_indep_batch[0].hw_buf           = buf;
    cfg_wami_warp_x3_indep_batch[0].hw_buf        = buf;
    cfg_wami_warp_y3_indep_batch[0].hw_buf        = buf;
    cfg_wami_steep_descent3_indep_batch[0].hw_buf = buf;
    cfg_wami_hessian3_indep_batch[0].hw_buf       = buf;
    cfg_wami_inv3_indep_batch[0].hw_buf           = buf;
    cfg_wami_sd_update3_indep_batch[0].hw_buf     = buf;
    cfg_wami_mult3_indep_batch[0].hw_buf          = buf;
    cfg_wami_reshape3_indep_batch[0].hw_buf       = buf;
    cfg_wami_add3_indep_batch[0].hw_buf           = buf;
    cfg_wami_warp_iwxp3_indep_batch[0].hw_buf     = buf;

    struct wami_run_stratus_access *tmp;

    tmp               = (struct wami_run_stratus_access *)cfg_wami_grayscale3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_gradient3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_warp_img3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_sub3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_warp_x3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_warp_y3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_steep_descent3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_hessian3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_inv3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_sd_update3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_mult3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_reshape3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_add3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;
    tmp               = (struct wami_run_stratus_access *)cfg_wami_warp_iwxp3_indep_batch[0].esp_desc;
    tmp->wami_num_img = test_batch;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    // [humu]: this doesn't work since it needs to be constant
    // set the global variable for the batch number
    // indep_batch_num = batch;

    gettime(&t_test_indep_mem_batch_1);
    esp_run(cfg_wami_debayer3_indep_batch, 1);
    esp_run(cfg_wami_grayscale3_indep_batch, 1);
    esp_run(cfg_wami_gradient3_indep_batch, 1);
    esp_run(cfg_wami_warp_img3_indep_batch, 1);
    esp_run(cfg_wami_sub3_indep_batch, 1);
    esp_run(cfg_wami_warp_x3_indep_batch, 1);
    esp_run(cfg_wami_warp_y3_indep_batch, 1);
    esp_run(cfg_wami_steep_descent3_indep_batch, 1);
    esp_run(cfg_wami_hessian3_indep_batch, 1);
    esp_run(cfg_wami_inv3_indep_batch, 1);
    esp_run(cfg_wami_sd_update3_indep_batch, 1);
    esp_run(cfg_wami_mult3_indep_batch, 1);
    esp_run(cfg_wami_reshape3_indep_batch, 1);
    esp_run(cfg_wami_add3_indep_batch, 1);
    esp_run(cfg_wami_warp_iwxp3_indep_batch, 1);
    gettime(&t_test_indep_mem_batch_2);

    validate_buf(buf);

    time_s = ts_subtract(&t_test_indep_mem_batch_1, &t_test_indep_mem_batch_2);
    printf("-------------------------------------------------------\n");
    printf("   (optimized)   \n");
    printf("Finish testing. time of test_independent_memory with batch = %d: %llu (ns)\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_independent_memory_unit_test(token_t *buf, char *acc_name, char *validatema)
{
    // unit tests
    reset_buf_to_zero(buf);
    load_buf(buf);

    printf("--- wami_%s (unit test) --- \n", acc_name);

    if (strcmp(acc_name, "debayer") == 0) {
        cfg_wami_debayer3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_debayer3_indep, 1);
    }

    if (strcmp(acc_name, "grayscale") == 0) {
        cfg_wami_grayscale3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_grayscale3_indep, 1);
    }
    if (strcmp(acc_name, "gradient") == 0) {
        cfg_wami_gradient3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_gradient3_indep, 1);
    }
    if (strcmp(acc_name, "warp_img") == 0) {
        cfg_wami_warp_img3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_warp_img3_indep, 1);
    }
    if (strcmp(acc_name, "sub") == 0) {
        cfg_wami_sub3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_sub3_indep, 1);
    }
    if (strcmp(acc_name, "warp_dx") == 0) {
        cfg_wami_warp_x3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_warp_x3_indep, 1);
    }
    if (strcmp(acc_name, "warp_dy") == 0) {
        cfg_wami_warp_y3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_warp_y3_indep, 1);
    }
    if (strcmp(acc_name, "steep_descent") == 0) {
        cfg_wami_steep_descent3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_steep_descent3_indep, 1);
    }
    if (strcmp(acc_name, "hessian") == 0) {
        cfg_wami_hessian3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_hessian3_indep, 1);
    }
    if (strcmp(acc_name, "inv") == 0) {
        cfg_wami_inv3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_inv3_indep, 1);
    }
    if (strcmp(acc_name, "sd_update") == 0) {
        cfg_wami_sd_update3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_sd_update3_indep, 1);
    }
    if (strcmp(acc_name, "mult") == 0) {
        cfg_wami_mult3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_mult3_indep, 1);
    }
    if (strcmp(acc_name, "reshape") == 0) {
        cfg_wami_reshape3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_reshape3_indep, 1);
    }
    if (strcmp(acc_name, "add") == 0) {
        cfg_wami_add3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_add3_indep, 1);
    }
    if (strcmp(acc_name, "warp_iwxp") == 0) {
        cfg_wami_warp_iwxp3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_warp_iwxp3_indep, 1);
    }
    if (strcmp(acc_name, "gradient_dummy") == 0) {
        cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_gradient3_dummy_indep, 1);
    }
    if (strcmp(acc_name, "gmm") == 0) {
        cfg_wami_gmm3_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_gmm3_indep, 1);
    }

    if (strcmp(acc_name, "reshape_add") == 0) { // combo1
        cfg_wami_reshape_add_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_reshape_add_indep, 1);
    }
    if (strcmp(acc_name, "hessian_inv") == 0) { // combo 2
        cfg_wami_hessian_inv_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_hessian_inv_indep, 1);
    }
    if (strcmp(acc_name, "steep_descent_hessian") == 0) { // combo 3
        cfg_wami_steep_descent_hessian_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_steep_descent_hessian_indep, 1);
    }
    if (strcmp(acc_name, "mult_reshape_add") == 0) { // combo 4
        cfg_wami_mult_reshape_add_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_mult_reshape_add_indep, 1);
    }
    if (strcmp(acc_name, "warp_xy") == 0) {
        cfg_wami_warp_xy_indep[0].hw_buf = buf;
        esp_run_1_no_thread(cfg_wami_warp_xy_indep, 1);
    }

    if (strcmp(validatema, "true") == 0 || strcmp(validatema, "1") == 0) {
        validate_buf(buf);
    }
}

void test_shared_memory(token_t *buf)
{
    cfg_wami_debayer3_sharemem[0].hw_buf        = buf;
    cfg_wami_grayscale3_sharemem[0].hw_buf      = buf;
    cfg_wami_gradient3_sharemem[0].hw_buf       = buf;
    cfg_wami_warp_img3_sharemem[0].hw_buf       = buf;
    cfg_wami_sub3_sharemem[0].hw_buf            = buf;
    cfg_wami_warp_x3_sharemem[0].hw_buf         = buf;
    cfg_wami_warp_y3_sharemem[0].hw_buf         = buf;
    cfg_wami_steep_descent3_sharemem[0].hw_buf  = buf;
    cfg_wami_hessian3_sharemem[0].hw_buf        = buf;
    cfg_wami_inv3_sharemem[0].hw_buf            = buf;
    cfg_wami_sd_update3_sharemem[0].hw_buf      = buf;
    cfg_wami_mult3_sharemem[0].hw_buf           = buf;
    cfg_wami_reshape3_sharemem[0].hw_buf        = buf;
    cfg_wami_add3_sharemem[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_sharemem[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_sharemem[0].hw_buf = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    gettime(&t_test_shared_mem_1);
    esp_run(cfg_wami_debayer3_sharemem, 1);
    esp_run(cfg_wami_grayscale3_sharemem, 1);
    esp_run(cfg_wami_gradient3_sharemem, 1);
    esp_run(cfg_wami_warp_img3_sharemem, 1);
    esp_run(cfg_wami_sub3_sharemem, 1);
    esp_run(cfg_wami_warp_x3_sharemem, 1);
    esp_run(cfg_wami_warp_y3_sharemem, 1);
    esp_run(cfg_wami_steep_descent3_sharemem, 1);
    esp_run(cfg_wami_hessian3_sharemem, 1);
    esp_run(cfg_wami_inv3_sharemem, 1);
    esp_run(cfg_wami_sd_update3_sharemem, 1);
    esp_run(cfg_wami_mult3_sharemem, 1);
    esp_run(cfg_wami_reshape3_sharemem, 1);
    esp_run(cfg_wami_add3_sharemem, 1);
    esp_run(cfg_wami_warp_iwxp3_sharemem, 1);
    esp_run(cfg_wami_gradient3_dummy_sharemem, 1);
    gettime(&t_test_shared_mem_2);

    validate_buf(buf);

    time_s = ts_subtract(&t_test_shared_mem_1, &t_test_shared_mem_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. test_shared_memory: %llu (ns)\n", time_s);
    fprintf(log_file, "test_shared_memory, %d, %llu\n", 1, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p(token_t *buf, int test_batch)
{
    cfg_wami_p2p[0].hw_buf  = buf;
    cfg_wami_p2p[1].hw_buf  = buf;
    cfg_wami_p2p[2].hw_buf  = buf;
    cfg_wami_p2p[3].hw_buf  = buf;
    cfg_wami_p2p[4].hw_buf  = buf;
    cfg_wami_p2p[5].hw_buf  = buf;
    cfg_wami_p2p[6].hw_buf  = buf;
    cfg_wami_p2p[7].hw_buf  = buf;
    cfg_wami_p2p[8].hw_buf  = buf;
    cfg_wami_p2p[9].hw_buf  = buf;
    cfg_wami_p2p[10].hw_buf = buf;
    cfg_wami_p2p[11].hw_buf = buf;
    cfg_wami_p2p[12].hw_buf = buf;
    cfg_wami_p2p[13].hw_buf = buf;
    cfg_wami_p2p[14].hw_buf = buf;
    cfg_wami_p2p[15].hw_buf = buf;

    struct wami_run_stratus_access *tmp;

    for (i = 0; i < 16; i++) {
        tmp               = (struct wami_run_stratus_access *)cfg_wami_p2p[i].esp_desc;
        tmp->wami_num_img = test_batch;
    }

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    // const int MEM_TILE_IDX = 0;
    // mon_args.read_mode = ESP_MON_READ_SINGLE;
    mon_args.read_mode = ESP_MON_READ_ALL;
    // mon_args.tile_index = MEM_TILE_IDX;
    // mon_args.mon_index = MON_DDR_WORD_TRANSFER_INDEX;

    // unsigned int ddr_accesses_start, ddr_accesses_end;
    // unsigned int ddr_accesses_diff;

    // ddr_accesses_start = esp_monitor(mon_args, NULL);
    esp_monitor(mon_args, &vals_start);

    printf("--- wami p2p x%d--- \n", test_batch);
    gettime(&t_test_p2p_1);
    // esp_run(cfg_wami_p2p, 16);
    esp_run_no_print(cfg_wami_p2p, 16);
    gettime(&t_test_p2p_2);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_p2p_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    // esp_monitor_print_mem_access_only(mon_args, vals_diff);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_1, &t_test_p2p_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p x%d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file, "test_p2p, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p_combo1(token_t *buf, int test_batch)
{
    cfg_wami_p2p_combo1[0].hw_buf  = buf;
    cfg_wami_p2p_combo1[1].hw_buf  = buf;
    cfg_wami_p2p_combo1[2].hw_buf  = buf;
    cfg_wami_p2p_combo1[3].hw_buf  = buf;
    cfg_wami_p2p_combo1[4].hw_buf  = buf;
    cfg_wami_p2p_combo1[5].hw_buf  = buf;
    cfg_wami_p2p_combo1[6].hw_buf  = buf;
    cfg_wami_p2p_combo1[7].hw_buf  = buf;
    cfg_wami_p2p_combo1[8].hw_buf  = buf;
    cfg_wami_p2p_combo1[9].hw_buf  = buf;
    cfg_wami_p2p_combo1[10].hw_buf = buf;
    cfg_wami_p2p_combo1[11].hw_buf = buf;
    cfg_wami_p2p_combo1[12].hw_buf = buf;
    cfg_wami_p2p_combo1[13].hw_buf = buf;
    cfg_wami_p2p_combo1[14].hw_buf = buf;

    struct wami_run_stratus_access *tmp;

    for (i = 0; i < 15; i++) {
        tmp               = (struct wami_run_stratus_access *)cfg_wami_p2p_combo1[i].esp_desc;
        tmp->wami_num_img = test_batch;
    }

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    mon_args.read_mode = ESP_MON_READ_ALL;
    esp_monitor(mon_args, &vals_start);

    printf("--- wami p2p_combo1 x%d--- \n", test_batch);
    gettime(&t_test_p2p_1);
    // esp_run(cfg_wami_p2p_combo1, 15);
    esp_run_no_print(cfg_wami_p2p_combo1, 15);
    gettime(&t_test_p2p_2);

    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_p2p_combo1_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_1, &t_test_p2p_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_combo1 x%d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo1, "test_p2p_combo1, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p_combo2(token_t *buf, int test_batch)
{
    cfg_wami_p2p_combo2[0].hw_buf  = buf;
    cfg_wami_p2p_combo2[1].hw_buf  = buf;
    cfg_wami_p2p_combo2[2].hw_buf  = buf;
    cfg_wami_p2p_combo2[3].hw_buf  = buf;
    cfg_wami_p2p_combo2[4].hw_buf  = buf;
    cfg_wami_p2p_combo2[5].hw_buf  = buf;
    cfg_wami_p2p_combo2[6].hw_buf  = buf;
    cfg_wami_p2p_combo2[7].hw_buf  = buf;
    cfg_wami_p2p_combo2[8].hw_buf  = buf;
    cfg_wami_p2p_combo2[9].hw_buf  = buf;
    cfg_wami_p2p_combo2[10].hw_buf = buf;
    cfg_wami_p2p_combo2[11].hw_buf = buf;
    cfg_wami_p2p_combo2[12].hw_buf = buf;
    cfg_wami_p2p_combo2[13].hw_buf = buf;
    cfg_wami_p2p_combo2[14].hw_buf = buf;

    struct wami_run_stratus_access *tmp;

    for (i = 0; i < 15; i++) {
        tmp               = (struct wami_run_stratus_access *)cfg_wami_p2p_combo2[i].esp_desc;
        tmp->wami_num_img = test_batch;
    }

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    mon_args.read_mode = ESP_MON_READ_ALL;
    esp_monitor(mon_args, &vals_start);

    printf("--- wami p2p_combo2 x%d--- \n", test_batch);
    gettime(&t_test_p2p_1);
    // esp_run(cfg_wami_p2p_combo2, 15);
    esp_run_no_print(cfg_wami_p2p_combo2, 15);
    gettime(&t_test_p2p_2);

    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_p2p_combo2_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_1, &t_test_p2p_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_combo2 x%d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo2, "test_p2p_combo2, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p_combo3(token_t *buf, int test_batch)
{
    cfg_wami_p2p_combo3[0].hw_buf  = buf;
    cfg_wami_p2p_combo3[1].hw_buf  = buf;
    cfg_wami_p2p_combo3[2].hw_buf  = buf;
    cfg_wami_p2p_combo3[3].hw_buf  = buf;
    cfg_wami_p2p_combo3[4].hw_buf  = buf;
    cfg_wami_p2p_combo3[5].hw_buf  = buf;
    cfg_wami_p2p_combo3[6].hw_buf  = buf;
    cfg_wami_p2p_combo3[7].hw_buf  = buf;
    cfg_wami_p2p_combo3[8].hw_buf  = buf;
    cfg_wami_p2p_combo3[9].hw_buf  = buf;
    cfg_wami_p2p_combo3[10].hw_buf = buf;
    cfg_wami_p2p_combo3[11].hw_buf = buf;
    cfg_wami_p2p_combo3[12].hw_buf = buf;
    cfg_wami_p2p_combo3[13].hw_buf = buf;
    cfg_wami_p2p_combo3[14].hw_buf = buf;

    struct wami_run_stratus_access *tmp;

    for (i = 0; i < 15; i++) {
        tmp               = (struct wami_run_stratus_access *)cfg_wami_p2p_combo3[i].esp_desc;
        tmp->wami_num_img = test_batch;
    }

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    mon_args.read_mode = ESP_MON_READ_ALL;
    esp_monitor(mon_args, &vals_start);

    printf("--- wami p2p_combo3 x%d--- \n", test_batch);
    gettime(&t_test_p2p_1);
    // esp_run(cfg_wami_p2p_combo3, 15);
    esp_run_no_print(cfg_wami_p2p_combo3, 15);
    gettime(&t_test_p2p_2);

    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_p2p_combo3_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_1, &t_test_p2p_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_combo3 x%d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo3, "test_p2p_combo3, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p_combo4(token_t *buf, int test_batch)
{
    cfg_wami_p2p_combo4[0].hw_buf  = buf;
    cfg_wami_p2p_combo4[1].hw_buf  = buf;
    cfg_wami_p2p_combo4[2].hw_buf  = buf;
    cfg_wami_p2p_combo4[3].hw_buf  = buf;
    cfg_wami_p2p_combo4[4].hw_buf  = buf;
    cfg_wami_p2p_combo4[5].hw_buf  = buf;
    cfg_wami_p2p_combo4[6].hw_buf  = buf;
    cfg_wami_p2p_combo4[7].hw_buf  = buf;
    cfg_wami_p2p_combo4[8].hw_buf  = buf;
    cfg_wami_p2p_combo4[9].hw_buf  = buf;
    cfg_wami_p2p_combo4[10].hw_buf = buf;
    cfg_wami_p2p_combo4[11].hw_buf = buf;
    cfg_wami_p2p_combo4[12].hw_buf = buf;
    cfg_wami_p2p_combo4[13].hw_buf = buf;

    struct wami_run_stratus_access *tmp;

    for (i = 0; i < 14; i++) {
        tmp               = (struct wami_run_stratus_access *)cfg_wami_p2p_combo4[i].esp_desc;
        tmp->wami_num_img = test_batch;
    }

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    esp_monitor_args_t mon_args;
    esp_monitor_vals_t vals_start, vals_end, vals_diff;
    mon_args.read_mode = ESP_MON_READ_ALL;
    esp_monitor(mon_args, &vals_start);

    fprintf(stderr, "--- wami p2p_combo4 x%d--- \n", test_batch);
    gettime(&t_test_p2p_1);
    // esp_run(cfg_wami_p2p_combo4, 14);
    esp_run_no_print(cfg_wami_p2p_combo4, 14);
    gettime(&t_test_p2p_2);

    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    FILE *fp  = fopen("test_p2p_combo4_esp_mon_all.txt", "w");
    esp_monitor_print(mon_args, vals_diff, fp);
    fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_1, &t_test_p2p_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_combo4 x%d: %llu (ns)\n", test_batch, time_s);
    fprintf(log_file_combo4, "test_p2p_combo4, %d, %llu\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_app_level_speedup(token_t *buf, int acc_num)
{
    // this is a mix of run_pv and independent test
    // printf("------- test_app_level_speedup, start\n");

    cfg_wami_debayer3_indep[0].hw_buf        = buf;
    cfg_wami_grayscale3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_indep[0].hw_buf       = buf;
    cfg_wami_warp_img3_indep[0].hw_buf       = buf;
    cfg_wami_sub3_indep[0].hw_buf            = buf;
    cfg_wami_warp_x3_indep[0].hw_buf         = buf;
    cfg_wami_warp_y3_indep[0].hw_buf         = buf;
    cfg_wami_steep_descent3_indep[0].hw_buf  = buf;
    cfg_wami_hessian3_indep[0].hw_buf        = buf;
    cfg_wami_inv3_indep[0].hw_buf            = buf;
    cfg_wami_sd_update3_indep[0].hw_buf      = buf;
    cfg_wami_mult3_indep[0].hw_buf           = buf;
    cfg_wami_reshape3_indep[0].hw_buf        = buf;
    cfg_wami_add3_indep[0].hw_buf            = buf;
    cfg_wami_warp_iwxp3_indep[0].hw_buf      = buf;
    cfg_wami_gradient3_dummy_indep[0].hw_buf = buf;

    // -- load inputs to the memory
    reset_buf_to_zero(buf);
    load_buf(buf);

    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    // Debayer - compute golden outputs
    gettime(&t_test_app_level_speedup_1);
    if (acc_num == 1) {
        esp_run(cfg_wami_debayer3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * nRows * nCols;
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
        }
    }
    // Grayscale - compute golden outputs
    if (acc_num == 2) {
        esp_run(cfg_wami_grayscale3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD,
                               nCols - 2 * PAD);
        }
    }
    // Gradient - compute golden outputs
    if (acc_num == 3) {
        esp_run(cfg_wami_gradient3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                         golden_grad_y + out_offset);
        }
    }
    // Warp-img - compute golden outputs
    if (acc_num == 4) {
        esp_run(cfg_wami_warp_img3_indep, 1);
    } else {
        for (i = 0; i < 6; i++)
            affine_warp[i] = 0.0;
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_img + out_offset);
        }
    }
    // Subtract - compute golden outputs
    if (acc_num == 5) {
        esp_run(cfg_wami_sub3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __subtract(IWxp,
                       golden_warp_img + out_offset, // TODO????
                       nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
        }
    }
    // Warp-dx - compute golden outputs
    if (acc_num == 6) {
        esp_run(cfg_wami_warp_x3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_dx + out_offset);
        }
    }
    // Warp-dy - compute golden outputs
    if (acc_num == 7) {
        esp_run(cfg_wami_warp_y3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                         golden_warp_dy + out_offset);
        }
    }
    // Steepest descent - compute golden outputs
    if (acc_num == 8) {
        esp_run(cfg_wami_steep_descent3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                               golden_I_steepest + out_offset);
        }
    }
    // Hessian - compute golden outputs
    if (acc_num == 9) {
        esp_run(cfg_wami_hessian3_indep, 1);
    } else {

        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 36;
            __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
        }
    }
    // Invert Gauss Jordan - compute golden outputs
    if (acc_num == 10) {
        esp_run(cfg_wami_inv3_indep, 1);
    } else {

        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 36;
            __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6,
                                  golden_hess_inv + out_offset);
        }
    }
    // SD update - compute golden outputs
    if (acc_num == 11) {
        esp_run(cfg_wami_sd_update3_indep, 1);
    } else {

        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * 6;
            __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD,
                        nCols - 2 * PAD, golden_sd_delta_p + out_offset);
        }
    }
    // Mult - compute golden outputs
    if (acc_num == 12) {
        esp_run(cfg_wami_mult3_indep, 1);
    } else {

        for (img = 0; img < batch; img++) {
            in_offset_1 = img * 36;
            in_offset_2 = img * 6;
            out_offset  = img * 6;
            __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6,
                   golden_delta_p + out_offset);
        }
    }
    // Reshape - compute golden outputs
    if (acc_num == 13) {
        esp_run(cfg_wami_reshape3_indep, 1);
    } else {

        for (img = 0; img < batch; img++) {
            in_offset  = img * 6;
            out_offset = img * 6;
            __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
        }
    }
    // Add - compute golden outputs
    if (acc_num == 14) {
        esp_run(cfg_wami_add3_indep, 1);
    } else {

        for (img = 0; img < batch; img++) {
            in_offset  = img * 6;
            out_offset = img * 6;
            __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
        }
    }
    // Warp-iwxp - compute golden outputs
    if (acc_num == 15) {
        esp_run(cfg_wami_warp_iwxp3_indep, 1);
    } else {
        for (img = 0; img < batch; img++) {
            in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
            __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                         golden_warp_iwxp + out_offset);
        }
    }
    gettime(&t_test_app_level_speedup_2);

    // Change detection - compute golden outputs
    // printf("Change detection output: # %d pixels\n",
    // CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);

    // cast from float to usigned short (16 bits)
    for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
        golden_gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
    }
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                   golden_foreground + out_offset, golden_gmm_img + in_offset);
    }

    // printf("Change detection output succcesfully computed.\n");
    // printf("------- test_app_level_speedup, finish\n");

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);
}

int main(int argc, char **argv)
{
    printf("-------------------------------------------------------\n");
    printf("-- WAMI RUN: START yo\n");
    printf("-------------------------------------------------------\n");

    int x, y;

    log_file_pv     = fopen("log_pv.txt", "w");
    log_file        = fopen("log.txt", "w");
    log_file_combo1 = fopen("log_combo1.txt", "w");
    log_file_combo2 = fopen("log_combo2.txt", "w");
    log_file_combo3 = fopen("log_combo3.txt", "w");
    log_file_combo4 = fopen("log_combo4.txt", "w");

    // test_00();

    // test_09();

    token_t *buf;
    buf = (token_t *)esp_alloc(5000000); // MEM_ONE_IMAGE_SIZE

    read_debayer_input();
    malloc_arrays();

    // this run_pv is to get the golden value
    run_pv(1);

    if (argc > 1) {
        if (argc == 3) { // unit tests

            printf("unit test: argc = %d, argv = %s, validatema = %s\n", argc, argv[1], argv[2]);

            test_independent_memory_unit_test(buf, argv[1], argv[2]);
        }
        goto CLEANUP;
    }

    //-- Test all accelerators with multi-thread shared memory
    printf("------- start to test multi-thread shared memory -------\n");

    for (x = 1; x < 20; x++) {
        test_mtsm(buf, x);
        // test_mtsm_combo1(buf, x);
        // test_mtsm_combo2(buf, x);
        // test_mtsm_combo3(buf, x);
        // test_mtsm_combo4(buf, x);
    }

    for (x = 2; x <= 10; x++) {
        // test_mtsm(buf, 10 * x);
        // test_mtsm_combo1(buf, 10 * x);
        // test_mtsm_combo2(buf, 10 * x);
        // test_mtsm_combo3(buf, 10 * x);
        // test_mtsm_combo4(buf, 10 * x);
    }

    // test_mtsm(buf, 1);
    // test_mtsm(buf, 10);
    // test_mtsm(buf, 100);
    test_mtsm(buf, 1000);
    // test_mtsm_combo1(buf, 1);
    // test_mtsm_combo1(buf, 10);
    // test_mtsm_combo1(buf, 100);
    // test_mtsm_combo2(buf, 1);
    // test_mtsm_combo2(buf, 10);
    // test_mtsm_combo2(buf, 100);
    // test_mtsm_combo3(buf, 1);
    // test_mtsm_combo3(buf, 10);
    // test_mtsm_combo3(buf, 100);
    // test_mtsm_combo4(buf, 1);
    // test_mtsm_combo4(buf, 10);
    // test_mtsm_combo4(buf, 100);

    printf("-------------------------------------------------------\n");

    // fprintf(log_file_pv, "------- run_pv()\n");

    for (x = 1; x < 20; x++) {
        run_pv(x);
    }

    // for (x = 2; x <= 10; x++) {
    //     run_pv(10 * x);
    // }

    // run_pv(1);
    // run_pv(10);
    // run_pv(100);
    // run_pv(1000);

    // run_pv_no_batchloop(1);

    // run_pv_no_individual_gettime(1);

    // run_pv_no_individual_gettime_no_batchloop(1);

    print_sw_time();

    printf("---- end of run_pv() ----------------------------------\n");

    // struct contig_alloc_params params;
    // params.policy = CONTIG_ALLOC_BALANCED;
    // params.policy = CONTIG_ALLOC_LEAST_LOADED;
    // buf = (token_t *)esp_alloc_policy(params, 5000000);
    // printf("=======>> CONTIG_ALLOC_LEAST_LOADED <<========\n");

    //--  Each test includes reset_buf, load_buf, compute, and validate_buf

    // Test all accelerators with there own memory space
    // test_independent_memory(buf);

    test_independent_memory_1_no_thread(buf);

    // test_independent_memory_no_print(buf);

    //============
    // [humu]: these optimized batch tests are running the 1st acc multiple times,
    // then the 2nd acc multiple times, and so on
    //
    // test_independent_memory_batch(buf, 1);
    // test_independent_memory_batch(buf, 10);
    // test_independent_memory_batch(buf, 100);
    // test_independent_memory_batch(buf, 1000);
    //============

    printf("------- start to test single-thread shared memory -------\n");

    for (x = 1; x < 20; x++) {
        test_independent_memory_real_batch(buf, x);
        // test_independent_memory_real_batch_combo1(buf, x);
        // test_independent_memory_real_batch_combo2(buf, x);
        // test_independent_memory_real_batch_combo3(buf, x);
        // test_independent_memory_real_batch_combo4(buf, x);
    }

    for (x = 2; x <= 10; x++) {
        // test_independent_memory_real_batch(buf, 10 * x);
        // test_independent_memory_real_batch_combo1(buf, 10 * x);
        // test_independent_memory_real_batch_combo2(buf, 10 * x);
        // test_independent_memory_real_batch_combo3(buf, 10 * x);
        // test_independent_memory_real_batch_combo4(buf, 10 * x);
    }

    // test_independent_memory_real_batch(buf, 1);
    // test_independent_memory_real_batch(buf, 10);
    // test_independent_memory_real_batch(buf, 100);
    // test_independent_memory_real_batch_combo1(buf, 1);
    // test_independent_memory_real_batch_combo1(buf, 10);
    // test_independent_memory_real_batch_combo1(buf, 100);
    // test_independent_memory_real_batch_combo2(buf, 1);
    // test_independent_memory_real_batch_combo2(buf, 10);
    // test_independent_memory_real_batch_combo2(buf, 100);
    // test_independent_memory_real_batch_combo3(buf, 1);
    // test_independent_memory_real_batch_combo3(buf, 10);
    // test_independent_memory_real_batch_combo3(buf, 100);
    // test_independent_memory_real_batch_combo4(buf, 1);
    // test_independent_memory_real_batch_combo4(buf, 10);
    // test_independent_memory_real_batch_combo4(buf, 100);

    //-- Test all accelerators with shared memory
    // test_shared_memory(buf);

    printf("------- start to test p2p -------\n");

    for (x = 1; x < 20; x++) {
        test_p2p(buf, x);
        // test_p2p_combo1(buf, x);
        // test_p2p_combo2(buf, x);
        // test_p2p_combo3(buf, x);
        // test_p2p_combo4(buf, x);
    }

    for (x = 2; x <= 10; x++) {
        // test_p2p(buf, 10 * x);
        // test_p2p_combo1(buf, 10 * x);
        // test_p2p_combo2(buf, 10 * x);
        // test_p2p_combo3(buf, 10 * x);
        // test_p2p_combo4(buf, 10 * x);
    }

    // test_p2p(buf, 1);
    // test_p2p(buf, 10);
    // test_p2p(buf, 100);
    test_p2p(buf, 1000);
    // test_p2p_combo1(buf, 1);
    // test_p2p_combo1(buf, 10);
    // test_p2p_combo1(buf, 100);
    // test_p2p_combo2(buf, 1);
    // test_p2p_combo2(buf, 10);
    // test_p2p_combo2(buf, 100);
    // test_p2p_combo3(buf, 1);
    // test_p2p_combo3(buf, 10);
    // test_p2p_combo3(buf, 100);
    // test_p2p_combo4(buf, 1);
    // test_p2p_combo4(buf, 10);
    // test_p2p_combo4(buf, 100);

    // printf("------- start to test application level speedup -------\n");
    // for (int i = 1; i <= 15; i++) {
    //     test_app_level_speedup(buf, i);
    //     time_s = ts_subtract(&t_test_app_level_speedup_1, &t_test_app_level_speedup_2);
    //     printf("-------------------------------------------------------\n");
    //     printf("     Finish testing application level speedup: %d\n", i);
    //     printf("--> time of app level speedup: %llu (ns)\n", time_s);
    //     printf("-------------------------------------------------------\n");
    // }

    /*
        printf("Test buf to zero\n");
        reset_buf_to_zero(buf);
        for (x = 0; x < 100; x++) {
            if (buf[x] != 0) {
                printf("%d: %ld\n", x, buf[x]);
            }
        }
    */

CLEANUP:
    esp_free(buf);

    free_arrays();
    fclose(log_file_pv);
    fclose(log_file);
    fclose(log_file_combo1);
    fclose(log_file_combo2);
    fclose(log_file_combo3);
    fclose(log_file_combo4);

    printf("-------------------------------------------------------\n");
    printf("-- WAMI RUN: FINISH\n");
    printf("-------------------------------------------------------\n");

    return 0;
}
