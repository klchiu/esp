// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include "system.hpp"

// [humu]: use fish_132_17.bin as the input for now
#define INPUT_SIZE INPUT_SIZE_132_17

#include "../../../wami_common/fileio.hpp"
#include "../../../wami_common/file_io_txt.hpp"
#include "../../../wami_common/wami_config_tb.hpp"
#include "../../../wami_common/wami_utils.hpp"

#include "../../../wami_common/wami_pv_headers/wami_debayer_pv.hpp"
#include "../../../wami_common/wami_pv_headers/wami_grayscale_pv.hpp"
#include "../../../wami_common/wami_pv_headers/wami_gradient_pv.hpp"
#include "../../../wami_common/wami_pv_headers/wami_lucas_kanade_pv.hpp"
#include "../../../wami_common/wami_pv_headers/wami_matrix_ops.hpp"
#include "../../../wami_common/wami_pv_headers/wami_gmm_pv.hpp"

std::ofstream ofs;

bool pixel_error_threshold(uint32_t errors, uint32_t total)
{
    double threshold = ((double)errors / (double)total);
    return (threshold > PXL_MAX_ERROR);
}

bool check_error_threshold(flt_pixel_t golden, flt_pixel_t out, double MAX_ERROR_THRESHOLD, double &error)
{
    error = (golden != 0) ? (golden - out) / golden : (out - golden) / out;
    error = (error < 0) ? -error : error;
    return (error > MAX_ERROR_THRESHOLD);
}

void compute_max_error(double error, double &max_error) { max_error = (error > max_error) ? error : max_error; }

void system_t::read_debayer_input()
{
    unsigned flag = integrated_binary_read(input_filename, &nRows, &nCols, &padding_in, &M, &N, &IWxp, &nModels, &mu,
                                           &sigma, &weight, &nTestImages, &images, &results);
    if (flag) {
        ESP_REPORT_INFO("Reading input file failed!");
    }

    nTestImages = 1; // [humu]: force to test only 1 images for now
}

void system_t::malloc_arrays()
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
}

void system_t::free_arrays()
{
    printf("1\n");
    free(IWxp);
    printf("2\n");
    free(mu);
    printf("3\n");
    free(sigma);
    printf("4\n");
    free(weight);
    printf("5\n");
    free(images);
    printf("6\n");
    free(results);

    printf("7\n");
    free(rgbtmpimg);
    printf("8\n");
    free(imgs);
    printf("9\n");
    free(grad_x);
    printf("0\n");
    free(grad_y);
    printf("1\n");
    free(IWxp_);
    printf("2\n");
    free(sub);
    printf("3\n");
    free(nabla_Ix);
    printf("4\n");
    free(nabla_Iy);
    printf("5\n");
    free(I_steepest);
    printf("6\n");
    free(hess);
    printf("7\n");
    free(hess_inv);
    printf("8\n");
    free(sd_delta_p);
    printf("9\n");
    free(delta_p);
    printf("0\n");
    free(sd_delta_p_nxt);
    printf("1\n");
    free(affine_warp_nxt);
    printf("2\n");
    free(warp_iwxp);
    printf("3\n");
    free(foreground);
    printf("4\n");
    free(gmm_img);
    printf("5\n");
    free(mu_dump);
    printf("6\n");
    free(sigma_dump);
    printf("7\n");
    free(weight_dump);

    printf("8\n");
    free(golden_rgb_imgs);
    printf("9\n");
    free(golden_gs_imgs);
    printf("0\n");
    free(golden_grad_x);
    printf("1\n");
    free(golden_grad_y);
    printf("2\n");
    free(golden_warp_img);
    printf("3\n");
    free(affine_warp);
    printf("4\n");
    free(golden_sub);
    printf("5\n");
    free(golden_warp_dx);
    printf("6\n");
    free(golden_warp_dy);
    printf("7\n");
    free(golden_warp_iwxp);
    printf("8\n");
    free(golden_I_steepest);
    printf("9\n");
    free(golden_hess);
    printf("0\n");
    free(golden_hess_inv);
    printf("1\n");
    free(igj_workspace);
    printf("2\n");
    free(golden_delta_p);
    printf("3\n");
    free(golden_sd_delta_p);
    printf("4\n");
    free(golden_sd_delta_p_nxt);
    printf("5\n");
    free(golden_affine_warp);
}

void system_t::run_pv()
{
    int batch = 1;

    // Debayer - compute golden outputs
    golden_rgb_imgs = (rgb_pixel_t *)malloc(sizeof(rgb_pixel_t) * DEBAYER_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * nRows * nCols;
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, nRows, nCols);
    }

    // Grayscale - compute golden outputs
    golden_gs_imgs = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, nRows - 2 * PAD, nCols - 2 * PAD);
    }
    // for(int i = 0 ;i < 100; i++){
    //     /printf("golden_gs_imgs[%d]: %f\n", i, golden_gs_imgs[i]);
    // }

    // Gradient - compute golden outputs
    golden_grad_x = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    golden_grad_y = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __gradientXY(golden_gs_imgs + in_offset, nRows - 2 * PAD, nCols - 2 * PAD, golden_grad_x + out_offset,
                     golden_grad_y + out_offset);
    }

    // Warp-img - compute golden outputs
    golden_warp_img = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * WARP_TOTAL_OUTPUT_NUM_PXL);
    affine_warp     = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    for (uint32_t i = 0; i < 6; i++)
        affine_warp[i] = 0.0;
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_img + out_offset);
    }

    // Subtract - compute golden outputs
    golden_sub = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        //    uint32_t in_offset = img * (nRows-2*PAD) * (nCols-2*PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);

        __subtract(IWxp,
                   golden_warp_img + out_offset, // TODO????
                   nCols - 2 * PAD, nRows - 2 * PAD, golden_sub + out_offset);
    }

    // Warp-dx - compute golden outputs
    golden_warp_dx = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_x + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dx + out_offset);
    }

    // Warp-dy - compute golden outputs
    golden_warp_dy = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * WARP_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_grad_y + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, affine_warp,
                     golden_warp_dy + out_offset);
    }

    // Steepest descent - compute golden outputs
    golden_I_steepest = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * 6 * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, nCols - 2 * PAD, nRows - 2 * PAD,
                           golden_I_steepest + out_offset);
    }

    // Hessian - compute golden outputs
    golden_hess = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * HESSIAN_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * 36;
        __hessian(golden_I_steepest + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, 6, golden_hess + out_offset);
    }

    // Invert Gauss Jordan - compute golden outputs
    golden_hess_inv = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    igj_workspace   = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * 36;
        __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6, golden_hess_inv + out_offset);
    }

    // SD update - compute golden outputs
    golden_sd_delta_p = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * 6;
        __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, nRows - 2 * PAD, nCols - 2 * PAD,
                    golden_sd_delta_p + out_offset);
    }

    // Mult - compute golden outputs
    golden_delta_p = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * MULT_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset_1 = img * 36;
        uint32_t in_offset_2 = img * 6;
        uint32_t out_offset  = img * 6;
        __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6, golden_delta_p + out_offset);
    }

    // Reshape - compute golden outputs
    golden_sd_delta_p_nxt = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * MULT_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * 6;
        uint32_t out_offset = img * 6;
        __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
    }

    // Add - compute golden outputs
    golden_affine_warp = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * ADD_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * 6;
        uint32_t out_offset = img * 6;
        __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
    }

    // Warp-iwxp - compute golden outputs
    golden_warp_iwxp = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * WARP_TOTAL_OUTPUT_NUM_PXL);
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        uint32_t out_offset = img * (nRows - 2 * PAD) * (nCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, nCols - 2 * PAD, nRows - 2 * PAD, golden_affine_warp,
                     golden_warp_iwxp + out_offset);
    }
}

// Process
void system_t::config_proc()
{
    read_debayer_input();

    base_addr   = 0;                                                   // [humu]: use this for now, for output image
    base_addr_1 = WAMI_WARP_IMG_NUM_ROWS * WAMI_WARP_IMG_NUM_COLS;     // [humu]: for input image
    base_addr_2 = 3 * WAMI_WARP_IMG_NUM_ROWS * WAMI_WARP_IMG_NUM_COLS; // [humu]:  for input matrix[6]

    // Reset
    {
        conf_done.write(false);
        conf_info.write(conf_info_t());
        wait();
    }

    ESP_REPORT_INFO("reset done");

    run_pv();

    // init some arrays
    malloc_arrays();

    // Load input data
    ESP_REPORT_INFO("INPUT_SIZE = %d\n", INPUT_SIZE);
    load_memory();

    // Config
    {
        // conf_info_t config(base_addr, nTestImages, nCols - 2 * PAD, nRows - 2 * PAD, padding_in, WARP_KERN_ID,
        //                    base_addr_1, base_addr_2);
        batch       = 1;
        base_addr_3 = 1;
        conf_info_t config(base_addr, nTestImages, nCols - 2 * PAD, nRows - 2 * PAD, padding_in, WARP_KERN_ID,
                           base_addr_1, base_addr_2, base_addr_3, base_addr_4, batch);

        ESP_REPORT_INFO("base_addr = %d\n", base_addr);
        ESP_REPORT_INFO("nTestImages = %d\n", nTestImages);
        ESP_REPORT_INFO("nRows = %d\n", nRows);
        ESP_REPORT_INFO("nCols = %d\n", nCols);
        ESP_REPORT_INFO("padding_in = %d\n", padding_in);
        ESP_REPORT_INFO("batch = %d\n", batch);
        ESP_REPORT_INFO("base_addr_1 = %d\n", base_addr_1);
        ESP_REPORT_INFO("base_addr_2 = %d\n", base_addr_2);
        ESP_REPORT_INFO("base_addr_3 = %d\n", base_addr_3);
        ESP_REPORT_INFO("base_addr_4 = %d\n", base_addr_4);

        wait();
        conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - wami_warp_xy");

        // Wait the termination of the accelerator
        do {
            wait();
            // printf("%d", acc_done.read());
        } while (!acc_done.read());

        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - wami_warp_xy");

        esc_log_latency(sc_object::name(), clock_cycle(end_time - begin_time));
        wait();
        conf_done.write(false);
    }

    // Validate
    {
        dump_memory(); // store the output in more suitable data structure if needed

        validate();
    }

    // Conclude
    {
        // while(true) wait();
        // Clean up
        ESP_REPORT_INFO("free arrays");
        // free_arrays();
        ESP_REPORT_INFO("Time to stop");
        sc_stop();
    }
}

// Functions
void system_t::load_memory()
{
#ifdef CADENCE
    // Optional usage check
    if (esc_argc() != 1) {
        ESP_REPORT_INFO("usage: %s\n", esc_argv()[0]);
        sc_stop();
    }
#endif

    //  Memory initialization:
    ESP_REPORT_INFO("---- load memory ----");
    uint32_t mem_base_addr = 0;

    // Load input image for warp (128x128)
    mem_base_addr = base_addr_1;
    ESP_REPORT_INFO("[load_memory]: load input image, mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < WAMI_WARP_IMG_NUM_ROWS * WAMI_WARP_IMG_NUM_COLS; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(golden_gs_imgs[index]); // [humu]: error is here
        // convert fx to bv
        data_bv = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        // printf("----- load memory %d, %f\t%f\n", index, golden_gs_imgs[index], (float)data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }
    mem_base_addr = base_addr_1 + 128*128;
    for (uint32_t index = 0; index < WAMI_WARP_IMG_NUM_ROWS * WAMI_WARP_IMG_NUM_COLS; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(golden_gs_imgs[index]); // [humu]: error is here
        // convert fx to bv
        data_bv = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        // printf("----- load memory %d, %f\t%f\n", index, golden_gs_imgs[index], (float)data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    // Load input array for warp (6)
    mem_base_addr = base_addr_2;
    ESP_REPORT_INFO("[load_memory]: load input array (6), mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < 6; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(golden_affine_warp[index]);
        // convert fx to bv
        data_bv = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        // printf("----- load memory %d, %f\t%f\n", index, golden_affine_warp[index], (float)data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    uint32_t mem_base_addr = base_addr; // 0

    ESP_REPORT_INFO("[dump_memory]: mem_base_addr: %d", mem_base_addr);

    for (uint32_t index = 0; index < WAMI_WARP_IMG_NUM_ROWS * WAMI_WARP_IMG_NUM_COLS; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        data_bv = mem[mem_base_addr + index];
        // convert bv to fp
        data_fp = bv2fp<FPDATA, FPDATA_WL>(data_bv);
        // conver fp to float
        warp_iwxp[index] = (float)data_fp;
    }
    write_float_matrix_to_file(warp_iwxp, (nRows - 2 * PAD), (nCols - 2 * PAD), "../output/fish_warp_iwxp_out.txt",
                               "Warp IWXP");

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    int      errors = 0;
    uint32_t batch  = 1;

    ESP_REPORT_INFO("---- validate ----");
    int    error_count = 0;
    double max_error   = 0;
    int    total       = 0;

    ESP_REPORT_INFO("Validating Warp IWXP");
    for (uint32_t row = 0; row < nRows - 2 * PAD; row++) {
        for (uint32_t col = 0; col < nCols - 2 * PAD; col++) {
            uint32_t i = row * nCols + col;

            if (abs(warp_iwxp[i] - golden_warp_iwxp[i]) > 1) {
                error_count++;
                // if (error_count < 20) {
                //     ESP_REPORT_INFO("%d: out: %f\tgold: %f", i, warp_iwxp[i], golden_warp_iwxp[i]);
                // }
            }
            if (i < 20) {
                ESP_REPORT_INFO("%d: out: %f\tgold: %f", i, warp_iwxp[i], golden_warp_iwxp[i]);
            }
            total++;
        }
    }
    ESP_REPORT_INFO("[validate] debug 4");

    ESP_REPORT_INFO("====================================================");
    if (error_count > 0) {
        ESP_REPORT_INFO("Errors: %d out of %d pixel(s) not match.", error_count, total);
    } else {
        ESP_REPORT_INFO("Correct!!");
    }
    ESP_REPORT_INFO("====================================================");

    ESP_REPORT_INFO("Validate completed");

    return errors;
}
