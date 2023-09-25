// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <my_stringify.h>
#include <wami_warp_xy_stratus.h>
#include <test/test.h>
#include <test/time.h>

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//--------------------------------------------------------------------------------------
#define INPUT_SIZE INPUT_SIZE_132_17

#include "wami_utils.h"
#include "wami_config_tb.h"
#include "wami_C_data.h"

#include "../include/wami_debayer_pv.h"
#include "../include/wami_grayscale_pv.h"
#include "../include/wami_matrix_ops.h"
#include "../include/wami_lucas_kanade_pv.h"
#include "../include/wami_gradient_pv.h"
#include "../include/wami_gmm_pv.h"

#include "../include/fileio.h"
//--------------------------------------------------------------------------------------

#include "cfg_wami_warp_xy.h"

#define DEVNAME "/dev/wami_warp_xy_stratus.0"
#define NAME    "wami_warp_xy_stratus"

#define DBGMSG(MSG)                                         \
    {                                                       \
        printf("%s, line %d: %s", __FILE__, __LINE__, MSG); \
    }

#define CLIP_INRANGE(LOW, VAL, HIGH) ((VAL) < (LOW) ? (LOW) : ((VAL) > (HIGH) ? (HIGH) : (VAL)))

// Supply "." for srcDir if files reside in current working directory

static void init_buf(struct wami_warp_xy_test *t)
{
    printf("init buffers\n");
    

    //  ========================  ^
    //  |  in/out image (int)  |  | img_size (in bytes)
    //  ========================  v

    

    // allocate buffers
}

int main(int argc, char **argv)
{
    printf("=== Helloo from wami_warp_xy\n");

    token_t *buf;

    printf("-------------------------------------------------------\n");
    printf("-------------------------------------------------------\n");
    //-- struct timespec t_debayer_1, t_debayer_2;
    //-- struct timespec t_grayscale_1, t_grayscale_2;
    //-- struct timespec t_gradient_1, t_gradient_2;
    //-- struct timespec t_warp_img_1, t_warp_img_2;
    //-- struct timespec t_subtract_1, t_subtract_2;
    //-- struct timespec t_warp_x_1, t_warp_x_2;
    //-- struct timespec t_warp_y_1, t_warp_y_2;
    //-- struct timespec t_steep_descent_1, t_steep_descent_2;
    //-- struct timespec t_hessian_1, t_hessian_2;
    //-- struct timespec t_inv_1, t_inv_2;
    //-- struct timespec t_sd_update_1, t_sd_update_2;
    //-- struct timespec t_mult_1, t_mult_2;
    //-- struct timespec t_reshape_1, t_reshape_2;
    //-- struct timespec t_add_1, t_add_2;
    //-- struct timespec t_warp_iwxp_1, t_warp_iwxp_2;

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

    // Input images
    uint32_t     nTestImages;
    uint16_t *   u16tmpimg;
    uint16_t *   images;
    uint8_t *    results;
    rgb_pixel_t *rgbtmpimg;
    uint32_t     nRows;
    uint32_t     nCols;

    // Warp and registration - k2 lucas kanade
    flt_pixel_t *imgs;
    dbl_pixel_t *grad_x;
    dbl_pixel_t *grad_y;
    dbl_pixel_t *IWxp_; // TODO
    dbl_pixel_t *sub;
    dbl_pixel_t *nabla_Ix;
    dbl_pixel_t *nabla_Iy;
    dbl_pixel_t *I_steepest;
    dbl_pixel_t *hess;
    dbl_pixel_t *hess_inv;
    dbl_pixel_t *sd_delta_p;
    dbl_pixel_t *delta_p;
    dbl_pixel_t *sd_delta_p_nxt;
    dbl_pixel_t *affine_warp_nxt;
    dbl_pixel_t *warp_iwxp;

    flt_pixel_t *tmplt;
    flt_pixel_t *IWxp;
    flt_pixel_t *swap;

    uint32_t M, N;

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
    printf("--- before read binary\n");
    unsigned flag = integrated_binary_read(input_filename, &nRows, &nCols, &padding_in, &M, &N, &IWxp, &nModels, &mu,
                                           &sigma, &weight, &nTestImages, &images, &results);

    uint32_t img, in_offset, out_offset;
    uint32_t in_offset_1, in_offset_2;

    printf("file: %s\n", input_filename);
    printf("nRows: %d\n", nRows);
    printf("nCols: %d\n", nCols);
    printf("padding_in: %d\n", padding_in);

    printf("M: %d\n", M);
    printf("N: %d\n", N);
    printf("IWxp: %f\n", IWxp);
    printf("nModels: %d\n", nModels);
    printf("mu: %f\n", mu);
    printf("sigma: %f\n", sigma);
    printf("weight: %f\n", weight);
    printf("nTestImages: %d\n", nTestImages);

    printf("-- INPUT_NUM_IMGS %d\n", INPUT_NUM_IMGS);
    printf("-- INPUT_IMG_NUM_ROWS %d\n", INPUT_IMG_NUM_ROWS);
    printf("-- INPUT_IMG_NUM_COLS %d\n", INPUT_IMG_NUM_COLS);
    printf("-- DEBAYER_INPUT_NUM_PXL %d\n", DEBAYER_INPUT_NUM_PXL);
    printf("-- DEBAYER_OUTPUT_NUM_PXL %d\n", DEBAYER_OUTPUT_NUM_PXL);
    printf("-- DEBAYER_TOTAL_INPUT_NUM_PXL %d\n", DEBAYER_TOTAL_INPUT_NUM_PXL);
    printf("-- DEBAYER_TOTAL_OUTPUT_NUM_PXL %d\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);

    // Debayer - compute golden outputs
    printf("Debayer output: # %d pixels\n", DEBAYER_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_debayer_1);
    rgb_pixel_t *golden_rgb_imgs = malloc(DEBAYER_TOTAL_OUTPUT_NUM_PXL * sizeof(rgb_pixel_t));
    printf("After debayer malloc\n");
    for (img = 0; img < batch; img++) {
        in_offset  = img * nRows * nCols;
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
    }
    //-- gettime(&t_debayer_2);
    printf("Debayer golden output succcesfully computed.\n");

    // Grayscale - compute golden outputs
    printf("Grayscale output: # %d pixels\n", GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_grayscale_1);
    flt_pixel_t *golden_gs_imgs = malloc(GRAYSCALE_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD, nCols - 2 * PAD);
    }
    //-- gettime(&t_grayscale_2);
    printf("Grayscale golden output succcesfully computed.\n");

    // Gradient - compute golden outputs
    printf("Gradient output: #  %d pixels\n", GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_gradient_1);
    flt_pixel_t *golden_grad_x = malloc(GRADIENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    flt_pixel_t *golden_grad_y = malloc(GRADIENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                     golden_grad_y + out_offset);
    }
    //-- gettime(&t_gradient_2);
    printf("Gradient golden output succcesfully computed.\n");

    // Warp-img - compute golden outputs
    printf("Warp-img output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_warp_img_1);
    flt_pixel_t *golden_warp_img = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    flt_pixel_t *affine_warp     = malloc(6 * sizeof(flt_pixel_t));
    for (i = 0; i < 6; i++)
        affine_warp[i] = 0.0;
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_img + out_offset);
    }
    //-- gettime(&t_warp_img_2);
    printf("Warp-img golden output succcesfully computed.\n");

    // Subtract - compute golden outputs
    printf("Subtract output: # %d pixels\n", SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_subtract_1);
    flt_pixel_t *golden_sub = malloc(SUBTRACT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

        __subtract(IWxp,
                   golden_warp_img + out_offset, // TODO????
                   nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
    }
    //-- gettime(&t_subtract_2);
    printf("Subtract golden output succcesfully computed.\n");

    // Warp-dx - compute golden outputs
    printf("Warp-dx output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_warp_x_1);
    flt_pixel_t *golden_warp_dx = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dx + out_offset);
    }
    //-- gettime(&t_warp_x_2);
    printf("Warp-dx golden output succcesfully computed.\n");

    // Warp-dy - compute golden outputs
    printf("Warp-dy output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_warp_y_1);
    flt_pixel_t *golden_warp_dy = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dy + out_offset);
    }
    //-- gettime(&t_warp_y_2);
    printf("Warp-dy golden output succcesfully computed.\n");

    // Steepest descent - compute golden outputs
    printf("Steepest descent output: # %d pixels\n", STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_steep_descent_1);
    flt_pixel_t *golden_I_steepest = malloc(STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                           golden_I_steepest + out_offset);
    }
    //-- gettime(&t_steep_descent_2);
    printf("Steepest descent golden output succcesfully computed.\n");

    // Hessian - compute golden outputs
    printf("Hessian output: # %d pixels\n", HESSIAN_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_hessian_1);
    flt_pixel_t *golden_hess = malloc(HESSIAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
    }
    //-- gettime(&t_hessian_2);
    printf("Hessian golden output succcesfully computed.\n");

    // Invert Gauss Jordan - compute golden outputs
    printf("Invert Gauss Jordan output: # %d pixels\n", INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_inv_1);
    flt_pixel_t *golden_hess_inv = malloc(INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    flt_pixel_t *igj_workspace   = malloc(INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 36;
        __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6, golden_hess_inv + out_offset);
    }
    //-- gettime(&t_inv_2);
    printf("Invert Gauss Jordan golden output succcesfully computed.\n");

    // SD update - compute golden outputs
    printf("SD update output: # %d pixels\n", SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_sd_update_1);
    flt_pixel_t *golden_sd_delta_p = malloc(SD_UPDATE_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * 6;
        __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD, nCols - 2 * PAD,
                    golden_sd_delta_p + out_offset);
    }
    //-- gettime(&t_sd_update_2);
    printf("SD update golden output succcesfully computed.\n");

    // Mult - compute golden outputs
    printf("Mult output: # %d pixels\n", MULT_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_mult_1);
    flt_pixel_t *golden_delta_p = malloc(MULT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset_1 = img * 36;
        in_offset_2 = img * 6;
        out_offset  = img * 6;
        __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6, golden_delta_p + out_offset);
    }
    //-- gettime(&t_mult_2);
    printf("Mult golden output succcesfully computed.\n");

    // Reshape - compute golden outputs
    printf("Reshape output: # %d pixels\n", RESHAPE_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_reshape_1);
    flt_pixel_t *golden_sd_delta_p_nxt = malloc(MULT_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
    }
    //-- gettime(&t_reshape_2);
    printf("Reshape golden output succcesfully computed.\n");

    // Add - compute golden outputs
    printf("Add output: # %d pixels\n", ADD_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_add_1);
    flt_pixel_t *golden_affine_warp = malloc(ADD_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * 6;
        out_offset = img * 6;
        __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
    }
    //-- gettime(&t_add_2);
    printf("Add golden output succcesfully computed.\n");

    // Warp-iwxp - compute golden outputs
    printf("Warp-iwxp output: # %d pixels\n", WARP_TOTAL_OUTPUT_NUM_PXL);
    //-- gettime(&t_warp_iwxp_1);
    flt_pixel_t *golden_warp_iwxp = malloc(WARP_TOTAL_OUTPUT_NUM_PXL * sizeof(flt_pixel_t));
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                     golden_warp_iwxp + out_offset);
    }
    //-- gettime(&t_warp_iwxp_2);
    printf("Warp-dy golden output succcesfully computed.\n");

    // Change detection - compute golden outputs
    printf("Change detection output: # %d pixels\n", CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);
    uint8_t *golden_foreground = malloc(CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL * sizeof(uint8_t));

    gmm_img = malloc(CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL * sizeof(uint16_t));

    // cast from float to usigned short (16 bits)
    for (i = 0; i < batch * (nRows - 2 * PAD) * (nCols - 2 * PAD); i++) {
        gmm_img[i] = (uint16_t)golden_warp_iwxp[i];
    }
    for (img = 0; img < batch; img++) {
        in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_gmm(nRows - 2 * PAD, nCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                   golden_foreground + out_offset, gmm_img + in_offset);
    }

    printf("Change detection output succcesfully computed.\n");
    printf("-------------------------------------------------------\n");
    printf("-------------------------------------------------------\n");

    buf = (token_t *)esp_alloc(50000000);
    cfg_wami_warp_xy[0].hw_buf = buf;
    
    printf("---- debug 4\n");
    esp_run(cfg_wami_warp_xy, 1);
    printf("---- debug 5\n");
    esp_free(buf);
    printf("---- debug 6\n");

/*
    unsigned long long time_s;
    time_s = ts_subtract(&t_debayer_1, &t_debayer_2);                   printf("sw time of debayer_3       : %llu\n", time_s);
    time_s = ts_subtract(&t_grayscale_1, &t_grayscale_2);                   printf("sw time of grayscale_3     : %llu\n", time_s);
    time_s = ts_subtract(&t_gradient_1, &t_gradient_2);                   printf("sw time of gradient_3      : %llu\n", time_s);
    time_s = ts_subtract(&t_warp_img_1, &t_warp_img_2);                   printf("sw time of warp_img_3      : %llu\n", time_s);
    time_s = ts_subtract(&t_subtract_1, &t_subtract_2);                   printf("sw time of subtract_3      : %llu\n", time_s);
    time_s = ts_subtract(&t_warp_x_1, &t_warp_x_2);                   printf("sw time of warp_x_3        : %llu\n", time_s);
    time_s = ts_subtract(&t_warp_y_1, &t_warp_y_2);                   printf("sw time of warp_y_3        : %llu\n", time_s);
    time_s = ts_subtract(&t_steep_descent_1, &t_steep_descent_2);                   printf("sw time of steep_descent_3 : %llu\n", time_s);
    time_s = ts_subtract(&t_hessian_1, &t_hessian_2);                   printf("sw time of hessian_3       : %llu\n", time_s);
    time_s = ts_subtract(&t_inv_1, &t_inv_2);                   printf("sw time of inv_3           : %llu\n", time_s);
    time_s = ts_subtract(&t_sd_update_1, &t_sd_update_2);                   printf("sw time of sd_update_3     : %llu\n", time_s);
    time_s = ts_subtract(&t_mult_1, &t_mult_2);                   printf("sw time of mult_3          : %llu\n", time_s);
    time_s = ts_subtract(&t_reshape_1, &t_reshape_2);                   printf("sw time of reshape_3       : %llu\n", time_s);
    time_s = ts_subtract(&t_add_1, &t_add_2);                   printf("sw time of add_3           : %llu\n", time_s);
    time_s = ts_subtract(&t_warp_iwxp_1, &t_warp_iwxp_2);                              printf("sw time of warp_iwxp_3     : %llu\n", time_s);
*/

    // free the memory
    free(golden_rgb_imgs);
    free(golden_gs_imgs);
    free(golden_grad_x);
    free(golden_grad_y);
    free(golden_warp_img);
    free(affine_warp);
    free(golden_sub);
    free(golden_warp_dx);
    free(golden_warp_dy);
    free(golden_I_steepest);
    free(golden_hess);
    free(golden_hess_inv);
    free(igj_workspace);
    free(golden_sd_delta_p);
    free(golden_delta_p);
    free(golden_sd_delta_p_nxt);
    free(golden_affine_warp);
    free(golden_warp_iwxp);
    free(golden_foreground);
    free(gmm_img);

    free(mu);
    free(sigma);
    free(weight);
    free(images);
    free(results);

    printf("-------------------------------------------------------\n");
    printf("-------------------------------------------------------\n");
    printf("-------------------------------------------------------\n");
    return 0;

    return 0;
}
