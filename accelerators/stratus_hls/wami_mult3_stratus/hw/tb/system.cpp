// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include <iostream>
#include <vector>
#include <sstream>
#include <fstream>
#include <stdio.h>
#include "system.hpp"

// [humu]: use fish_132_17.bin as the test input
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
    unsigned flag = integrated_binary_read(input_filename, &srcRows, &srcCols, &padding_in, &nRows, &nCols, &IWxp,
                                           &nModels, &mu, &sigma, &weight, &nTestImages, &images, &results);
    if (flag) {
        ESP_REPORT_INFO("Reading input file failed!");
    }
}

void system_t::malloc_arrays()
{
    uint32_t img_num_pxls   = DEBAYER_OUTPUT_NUM_PXL;
    uint32_t train_num_pxls = nModels * img_num_pxls;

    // malloc golden outputs
    golden_rgb_imgs       = (rgb_pixel_t *)malloc(sizeof(rgb_pixel_t) * DEBAYER_TOTAL_OUTPUT_NUM_PXL);
    golden_gs_imgs        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * GRAYSCALE_TOTAL_OUTPUT_NUM_PXL);
    golden_grad_x         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    golden_grad_y         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * GRADIENT_TOTAL_OUTPUT_NUM_PXL);
    golden_warp_img       = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * WARP_TOTAL_OUTPUT_NUM_PXL);
    affine_warp           = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    golden_sub            = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    golden_warp_dx        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * SUBTRACT_TOTAL_OUTPUT_NUM_PXL);
    golden_warp_dy        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * WARP_TOTAL_OUTPUT_NUM_PXL);
    golden_I_steepest     = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * STEEPEST_DESCENT_TOTAL_OUTPUT_NUM_PXL);
    golden_hess           = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * HESSIAN_TOTAL_OUTPUT_NUM_PXL);
    golden_hess_inv       = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    igj_workspace         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * INVERT_GAUSS_JORDAN_TOTAL_OUTPUT_NUM_PXL);
    golden_sd_delta_p     = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * SD_UPDATE_TOTAL_OUTPUT_NUM_PXL);
    golden_delta_p        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * MULT_TOTAL_OUTPUT_NUM_PXL);
    golden_sd_delta_p_nxt = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * MULT_TOTAL_OUTPUT_NUM_PXL);
    golden_affine_warp    = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * ADD_TOTAL_OUTPUT_NUM_PXL);
    golden_warp_iwxp      = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * WARP_TOTAL_OUTPUT_NUM_PXL);
    golden_foreground     = (uint8_t *)malloc(sizeof(uint8_t) * CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);

    // malloc test ouptuts
    rgb_imgs        = (rgb_pixel_t *)malloc(sizeof(rgb_pixel_t) * img_num_pxls);
    gs_imgs         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    grad_x          = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    grad_y          = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    warp_iwxp       = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    sub             = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    I_steepest      = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6 * img_num_pxls);
    hess            = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 36);
    hess_inv        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 36);
    sd_delta_p      = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    delta_p         = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    sd_delta_p_nxt  = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    affine_warp_nxt = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * 6);
    foreground      = (uint8_t *)malloc(sizeof(uint8_t) * img_num_pxls);

    // IWxp_           = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    // nabla_Ix        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);
    // nabla_Iy        = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * img_num_pxls);

    gmm_img = (uint16_t *)malloc(sizeof(uint16_t) * img_num_pxls);
    // mu_dump     = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * train_num_pxls);
    // sigma_dump  = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * train_num_pxls);
    // weight_dump = (flt_pixel_t *)malloc(sizeof(flt_pixel_t) * train_num_pxls);
}

void system_t::free_arrays()
{
    // free golden outputs
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
    free(golden_foreground);

    // free test outputs
    free(rgb_imgs);
    free(gs_imgs);
    free(grad_x);
    free(grad_y);
    free(warp_iwxp);
    free(sub);
    free(I_steepest);
    free(hess);
    free(hess_inv);
    free(sd_delta_p);
    free(delta_p);
    free(sd_delta_p_nxt);
    free(affine_warp_nxt);
    free(foreground);

    // free(IWxp_);
    // free(nabla_Ix);
    // free(nabla_Iy);
    free(gmm_img);
    // free(mu_dump);
    // free(sigma_dump);
    // free(weight_dump);
    // some mallocs are from the integrated_binary_read function
    free(IWxp);
    free(mu);
    free(sigma);
    free(weight);
    free(images);
    free(results);
}

void system_t::run_pv()
{
    int batch = 1;

    // Debayer - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * srcRows * srcCols;
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __wami_debayer(images + in_offset, golden_rgb_imgs + out_offset, srcRows, srcCols);
    }

    // Grayscale - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __rgb_to_grayscale(golden_rgb_imgs + in_offset, golden_gs_imgs + out_offset, srcRows - 2 * PAD,
                           srcCols - 2 * PAD);
    }

    // Gradient - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __gradientXY(golden_gs_imgs + in_offset, srcRows - 2 * PAD, srcCols - 2 * PAD, golden_grad_x + out_offset,
                     golden_grad_y + out_offset);
    }

    // Warp-img - compute golden outputs
    for (uint32_t i = 0; i < 6; i++)
        affine_warp[i] = 0.0;
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, srcCols - 2 * PAD, srcRows - 2 * PAD, affine_warp,
                     golden_warp_img + out_offset);
    }

    // Subtract - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        //    uint32_t in_offset = img * (srcRows-2*PAD) * (srcCols-2*PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);

        __subtract(IWxp,
                   golden_warp_img + out_offset, // TODO????
                   srcCols - 2 * PAD, srcRows - 2 * PAD, golden_sub + out_offset);
    }

    // Warp-dx - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __warp_image(golden_grad_x + in_offset, srcCols - 2 * PAD, srcRows - 2 * PAD, affine_warp,
                     golden_warp_dx + out_offset);
    }

    // Warp-dy - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __warp_image(golden_grad_y + in_offset, srcCols - 2 * PAD, srcRows - 2 * PAD, affine_warp,
                     golden_warp_dy + out_offset);
    }

    // Steepest descent - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * 6 * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __steepest_descent(golden_warp_dx + in_offset, golden_warp_dy + in_offset, srcCols - 2 * PAD, srcRows - 2 * PAD,
                           golden_I_steepest + out_offset);
    }

    // Hessian - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * 36;
        __hessian(golden_I_steepest + in_offset, srcCols - 2 * PAD, srcRows - 2 * PAD, 6, golden_hess + out_offset);
    }

    // Invert Gauss Jordan - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * 36;
        __invert_gauss_jordan(golden_hess + in_offset, igj_workspace + in_offset, 6, 6, golden_hess_inv + out_offset);
    }

    // SD update - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * 6;
        __sd_update(golden_I_steepest + (6 * in_offset), golden_sub + in_offset, 6, srcRows - 2 * PAD,
                    srcCols - 2 * PAD, golden_sd_delta_p + out_offset);
    }

    // Mult - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset_1 = img * 36;
        uint32_t in_offset_2 = img * 6;
        uint32_t out_offset  = img * 6;
        __mult(golden_hess_inv + in_offset_1, golden_sd_delta_p + in_offset_2, 6, 1, 6, golden_delta_p + out_offset);
    }

    // Reshape - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * 6;
        uint32_t out_offset = img * 6;
        __reshape(golden_delta_p + in_offset, 6, 1, 2, 3, golden_sd_delta_p_nxt + out_offset);
    }

    // Add - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * 6;
        uint32_t out_offset = img * 6;
        __add(affine_warp + in_offset, golden_sd_delta_p_nxt + in_offset, 2, 3, golden_affine_warp + out_offset);
    }

    // Warp-iwxp - compute golden outputs
    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __warp_image(golden_gs_imgs + in_offset, srcCols - 2 * PAD, srcRows - 2 * PAD, golden_affine_warp,
                     golden_warp_iwxp + out_offset);
    }

    // Change detection - compute golden outputs
    // ESP_REPORT_INFO("Change detection output: %d pixels", CHANGE_DETECTION_TOTAL_OUTPUT_NUM_PXL);
    // ESP_REPORT_INFO("run_pv debug 0");

    // cast from float to usigned short (16 bits)
    for (uint32_t i = 0; i < batch * (srcRows - 2 * PAD) * (srcCols - 2 * PAD); i++) {
        gmm_img[i] = (uint16_t)(golden_warp_iwxp[i]);
    }
    // ESP_REPORT_INFO("run_pv debug 1, nModels = %d", nModels);

    for (uint32_t img = 0; img < batch; img++) {
        uint32_t in_offset  = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        uint32_t out_offset = img * (srcRows - 2 * PAD) * (srcCols - 2 * PAD);
        __wami_gmm(srcRows - 2 * PAD, srcCols - 2 * PAD, nModels, mu + in_offset, sigma + in_offset, weight + in_offset,
                   golden_foreground + out_offset, gmm_img + in_offset);
    }
    // ESP_REPORT_INFO("Change detection golden output succcesfully computed.");
    // ESP_REPORT_INFO("Foreground # %d", f);
}


// Process
void system_t::config_proc()
{
    // Reset
    {
        conf_done.write(false);
        conf_info.write(conf_info_t());
        wait();
    }

    ESP_REPORT_INFO("reset done");

    malloc_arrays();

    read_debayer_input();

    run_pv();

    // Config
    {
        num_img      = 1;                      // test only 1 images for single accelerator, otherwise use "nTestImages"
        num_col      = srcCols - 2 * padding_in; // 128 in this test
        num_row      = srcRows - 2 * padding_in; // 128 in this test
        pad          = padding_in;             // 2 in this test
        kern_id      = MULT_KERN_ID;
        batch        = 0;
        base_addr_0  = 0;                // use this for output 1d array
        base_addr_1  = base_addr_0 + 6;  // use this for input 2d array
        base_addr_2  = base_addr_1 + 36; // use this for input 1d array
        base_addr_3  = 0;                // reserved
        base_addr_4  = 0;                // reserved
        is_p2p       = 0;
        p2p_config_0 = 0;
        p2p_config_1 = 0;

        conf_info_t config(num_img, num_row, num_col, pad, kern_id, batch, base_addr_0, base_addr_1, base_addr_2,
                           base_addr_3, base_addr_4, is_p2p, p2p_config_0, p2p_config_1);

        ESP_REPORT_INFO("[config]: num_img = %d", config.num_img);
        ESP_REPORT_INFO("[config]: num_row = %d", config.num_row);
        ESP_REPORT_INFO("[config]: num_col = %d", config.num_col);
        ESP_REPORT_INFO("[config]: pad = %d", config.pad);
        ESP_REPORT_INFO("[config]: kern_id = %d", config.kern_id);
        ESP_REPORT_INFO("[config]: batch = %d", config.batch);
        ESP_REPORT_INFO("[config]: base_addr_0 = %d", config.src_dst_offset_0);
        ESP_REPORT_INFO("[config]: base_addr_1 = %d", config.src_dst_offset_1);
        ESP_REPORT_INFO("[config]: base_addr_2 = %d", config.src_dst_offset_2);
        ESP_REPORT_INFO("[config]: base_addr_3 = %d", config.src_dst_offset_3);
        ESP_REPORT_INFO("[config]: base_addr_4 = %d", config.src_dst_offset_4);
        ESP_REPORT_INFO("[config]: is_p2p = %d", config.is_p2p);
        ESP_REPORT_INFO("[config]: p2p_config_0 = %d", config.p2p_config_0);
        ESP_REPORT_INFO("[config]: p2p_config_1 = %d", config.p2p_config_1);

        wait();
        conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Load input data
    ESP_REPORT_INFO("INPUT_SIZE = %d\n", INPUT_SIZE);
    load_memory();

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - wami_mult3");

        // Wait the termination of the accelerator
        do {
            wait();
        } while (!acc_done.read());

        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - wami_mult3");

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
        // Clean up
        ESP_REPORT_INFO("free arrays");
        free_arrays();
        ESP_REPORT_INFO("Time to stop");
        sc_stop();
    }
}

// Functions
void system_t::load_memory()
{
    //  Memory initialization:
    ESP_REPORT_INFO("---- load memory ----");
    uint32_t mem_base_addr = 0;

    // Load input 2d array (36)
    mem_base_addr = base_addr_1;
    ESP_REPORT_INFO("[load_memory]: load input 2d array, mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < 36; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(golden_hess_inv[index]);
        // convert fx to bv
        data_bv.range(63, 0) = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    // Load input 1d array (6)
    mem_base_addr = base_addr_2;
    ESP_REPORT_INFO("[load_memory]: load input 1d array (6), mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < 6; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(golden_sd_delta_p[index]);
        // convert fx to bv
        data_bv.range(63, 0) = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    // for (int i = 0; i < 6; i++) {
    //     ESP_REPORT_INFO("load--- [%d]: %f\n", i, golden_sd_delta_p[i]);
    // }

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    ESP_REPORT_INFO("---- dump memory ----");
    uint32_t mem_base_addr = 0;

    mem_base_addr = base_addr_0;
    ESP_REPORT_INFO("[dump_memory]: mem_base_addr: %d", mem_base_addr);

    for (uint32_t index = 0; index < 6; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        data_bv = mem[mem_base_addr + index];
        // convert bv to fp
        data_fp = bv2fp<FPDATA, FPDATA_WL>(data_bv);
        // conver fp to float
        delta_p[index] = (float)data_fp;
    }
    write_float_matrix_to_file(delta_p, srcRows, srcCols, "../output/fish_mult_out.txt", "Warp IWXP");

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    int      errors = 0;
    uint32_t batch  = 1;

    ESP_REPORT_INFO("---- validate ----");
    int error_count = 0;
    int total       = 0;

    for (uint32_t i = 0; i < 6; i++) {
        // [humu]: number too small, need to check

        if (abs(golden_delta_p[i] - delta_p[i]) > 0.1) {
            // #define MULT_MAX_ERROR                   3 * 1e-03

            error_count++;
            // ESP_REPORT_INFO("%d: out: %f\tgold: %f", i, delta_p[i], golden_delta_p[i]);
        }
        total++;
        ESP_REPORT_INFO("%d: out: %f\tgold: %f", i, delta_p[i], golden_delta_p[i]);
    }

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
