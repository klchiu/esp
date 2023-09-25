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
#include "../include/wami_homo_stratus.h"

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

//--------------------------------------------------------------------------------------
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
//--------------------------------------------------------------------------------------

#include "cfg_independent.h"
#include "cfg_p2p_grayscale_case1.h"
#include "cfg_p2p_grayscale_case2.h"
#include "cfg_p2p_reshape_case1.h"
#include "cfg_p2p_reshape_case2.h"


#include "monitors.h"

#define DEVNAME "/dev/wami_homo_stratus.0"
#define NAME    "wami_homo_stratus"

#define DBGMSG(MSG)                                         \
    {                                                       \
        printf("%s, line %d: %s", __FILE__, __LINE__, MSG); \
    }

#define CLIP_INRANGE(LOW, VAL, HIGH) ((VAL) < (LOW) ? (LOW) : ((VAL) > (HIGH) ? (HIGH) : (VAL)))

// Supply "." for srcDir if files reside in current working directory

//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
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

void run_pv(int test_batch)
{
    // printf("------- run_pv, start. test_batch: %d\n", test_batch);

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
    printf("--> time of run_pv with batch = %d: %llu (ns)\n", test_batch, time_s);

    // printf("------- run_pv, finish\n");
    // printf("-------------------------------------------------------\n");
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
    printf("sw time of gradient_dummy      : %llu (ns)\n", time_s);
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
    printf("-------------------------------------------------------\n");
}

void test_p2p_grayscale3_case1(token_t *buf, int test_batch)
{
    cfg_wami_p2p_grayscale3_case1[0].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[1].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[2].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[3].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[4].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[5].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[6].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[7].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[8].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[9].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case1[10].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case1[11].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case1[12].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case1[13].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case1[14].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case1[15].hw_buf = buf;

    struct wami_homo_stratus_access *tmp;

    for (i = 0; i < 16; i++) {
        tmp               = (struct wami_homo_stratus_access *)cfg_wami_p2p_grayscale3_case1[i].esp_desc;
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
    gettime(&t_test_p2p_grayscale3_1);
    // esp_run(cfg_wami_p2p_grayscale3_case1, 16);
    esp_run_no_print(cfg_wami_p2p_grayscale3_case1, 16);
    gettime(&t_test_p2p_grayscale3_2);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    // FILE *fp = fopen("test_indep_mem_esp_mon_all_1.txt", "w");
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_grayscale3_1, &t_test_p2p_grayscale3_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_grayscale3_case1 x%d: %llu (ns)\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p_grayscale3_case2(token_t *buf, int test_batch)
{
    cfg_wami_p2p_grayscale3_case2[0].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[1].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[2].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[3].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[4].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[5].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[6].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[7].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[8].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[9].hw_buf  = buf;
    cfg_wami_p2p_grayscale3_case2[10].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case2[11].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case2[12].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case2[13].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case2[14].hw_buf = buf;
    cfg_wami_p2p_grayscale3_case2[15].hw_buf = buf;

    struct wami_homo_stratus_access *tmp;

    for (i = 0; i < 16; i++) {
        tmp               = (struct wami_homo_stratus_access *)cfg_wami_p2p_grayscale3_case2[i].esp_desc;
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
    gettime(&t_test_p2p_grayscale3_1);
    // esp_run(cfg_wami_p2p_grayscale3_case2, 16);
    esp_run_no_print(cfg_wami_p2p_grayscale3_case2, 16);
    gettime(&t_test_p2p_grayscale3_2);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    // FILE *fp = fopen("test_indep_mem_esp_mon_all_1.txt", "w");
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_grayscale3_1, &t_test_p2p_grayscale3_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_grayscale3_case2 x%d: %llu (ns)\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}



void test_p2p_reshape3_case1(token_t *buf, int test_batch)
{
    cfg_wami_p2p_reshape3_case1[0].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[1].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[2].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[3].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[4].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[5].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[6].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[7].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[8].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[9].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case1[10].hw_buf = buf;
    cfg_wami_p2p_reshape3_case1[11].hw_buf = buf;
    cfg_wami_p2p_reshape3_case1[12].hw_buf = buf;
    cfg_wami_p2p_reshape3_case1[13].hw_buf = buf;
    cfg_wami_p2p_reshape3_case1[14].hw_buf = buf;
    cfg_wami_p2p_reshape3_case1[15].hw_buf = buf;

    struct wami_homo_stratus_access *tmp;

    for (i = 0; i < 16; i++) {
        tmp               = (struct wami_homo_stratus_access *)cfg_wami_p2p_reshape3_case1[i].esp_desc;
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
    gettime(&t_test_p2p_reshape3_1);
    // esp_run(cfg_wami_p2p_reshape3_case1, 16);
    esp_run_no_print(cfg_wami_p2p_reshape3_case1, 16);
    gettime(&t_test_p2p_reshape3_2);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    // FILE *fp = fopen("test_indep_mem_esp_mon_all_1.txt", "w");
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_reshape3_1, &t_test_p2p_reshape3_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_reshape3_case1 x%d: %llu (ns)\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}

void test_p2p_reshape3_case2(token_t *buf, int test_batch)
{
    cfg_wami_p2p_reshape3_case2[0].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[1].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[2].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[3].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[4].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[5].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[6].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[7].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[8].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[9].hw_buf  = buf;
    cfg_wami_p2p_reshape3_case2[10].hw_buf = buf;
    cfg_wami_p2p_reshape3_case2[11].hw_buf = buf;
    cfg_wami_p2p_reshape3_case2[12].hw_buf = buf;
    cfg_wami_p2p_reshape3_case2[13].hw_buf = buf;
    cfg_wami_p2p_reshape3_case2[14].hw_buf = buf;
    cfg_wami_p2p_reshape3_case2[15].hw_buf = buf;

    struct wami_homo_stratus_access *tmp;

    for (i = 0; i < 16; i++) {
        tmp               = (struct wami_homo_stratus_access *)cfg_wami_p2p_reshape3_case2[i].esp_desc;
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
    gettime(&t_test_p2p_reshape3_1);
    // esp_run(cfg_wami_p2p_reshape3_case2, 16);
    esp_run_no_print(cfg_wami_p2p_reshape3_case2, 16);
    gettime(&t_test_p2p_reshape3_2);

    // ddr_accesses_end = esp_monitor(mon_args, NULL);
    // ddr_accesses_diff = sub_monitor_vals(ddr_accesses_start, ddr_accesses_end);
    // printf("\tOff-chip memory accesses: %d\n", ddr_accesses_diff);
    esp_monitor(mon_args, &vals_end);
    vals_diff = esp_monitor_diff(vals_start, vals_end);
    // FILE *fp = fopen("test_indep_mem_esp_mon_all_1.txt", "w");
    esp_monitor_print_mem_access_only(mon_args, vals_diff);
    // fclose(fp);

    // [humu]: comment this validation for now, to disable the print
    // validate_buf(buf);

    time_s = ts_subtract(&t_test_p2p_reshape3_1, &t_test_p2p_reshape3_2);
    printf("-------------------------------------------------------\n");
    printf("Finish testing. time of test_p2p_reshape3_case2 x%d: %llu (ns)\n", test_batch, time_s);
    printf("-------------------------------------------------------\n");
}


int main(int argc, char **argv)
{
    printf("-------------------------------------------------------\n");
    printf("-- WAMI HOMO: START\n");
    printf("-------------------------------------------------------\n");

    token_t *buf;

    read_debayer_input();
    malloc_arrays();

    printf("-------------------------------------------------------\n");

    run_pv(1);

    // run_pv_no_individual_gettime_no_batchloop(1);
    printf("---- end of run_pv() ----------------------------------\n");

    buf = (token_t *)esp_alloc(5000000); // MEM_ONE_IMAGE_SIZE

    //--  Each test includes reset_buf, load_buf, compute, and validate_buf

    // Test all accelerators with there own memory space
    // test_independent_memory(buf);

    //============
    printf("     start to test single-thread shared memory\n");

//    test_p2p_grayscale3_case1(buf, 1);
//    test_p2p_grayscale3_case1(buf, 10);
//    test_p2p_grayscale3_case1(buf, 100);
//    test_p2p_grayscale3_case1(buf, 1000);
//    test_p2p_grayscale3_case1(buf, 10000);
//
//    test_p2p_grayscale3_case2(buf, 1);
//    test_p2p_grayscale3_case2(buf, 10);
//    test_p2p_grayscale3_case2(buf, 100);
//    test_p2p_grayscale3_case2(buf, 1000);
//    test_p2p_grayscale3_case2(buf, 10000);


    test_p2p_reshape3_case1(buf, 1);
    test_p2p_reshape3_case1(buf, 10);
    test_p2p_reshape3_case1(buf, 100);
    test_p2p_reshape3_case1(buf, 1000);
    test_p2p_reshape3_case1(buf, 10000);

    test_p2p_reshape3_case2(buf, 1);
    test_p2p_reshape3_case2(buf, 10);
    test_p2p_reshape3_case2(buf, 100);
    test_p2p_reshape3_case2(buf, 1000);
    test_p2p_reshape3_case2(buf, 10000);


    // Test all accelerators with multi-thread shared memory

    esp_free(buf);

    print_sw_time();

    free_arrays();

    printf("-------------------------------------------------------\n");
    printf("-- WAMI HOMO: FINISH\n");
    printf("-------------------------------------------------------\n");

    return 0;
}
