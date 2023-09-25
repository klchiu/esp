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

std::ofstream ofs;

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

    // Config
    {
        num_img     = 1; // test only 1 images for single accelerator, otherwise use "nTestImages"
        num_col     = 128*128;  // load_size
        num_row     = 128*128;  // store_size
        pad         = 2;
        kern_id     = SYNTH_KERN_ID;
        batch       = 0;
        base_addr_0 = 0;                 // use this for output
        base_addr_1 = num_col; // use this for input
        base_addr_2 = 0;                 // reserved
        base_addr_3 = 0;                 // reserved
        base_addr_4 = 0;                 // reserved
        is_p2p      = 0;
        // [note]: delay_A * delay_B should be smaller than 65536
        p2p_config_0 = 128; // compute_delay_A
        p2p_config_1 = 128; // compute_delay_B

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

        ESP_REPORT_INFO("config done");
    }

    // Load input data
    ESP_REPORT_INFO("INPUT_SIZE = %d\n", INPUT_SIZE);
    load_memory();

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - wami_synth3");

        // Wait the termination of the accelerator
        do {
            wait();
        } while (!acc_done.read());

        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - wami_synth3");

        esc_log_latency(sc_object::name(), clock_cycle(end_time - begin_time));
        wait();
        conf_done.write(false);
    }

    // Validate
    {
        dump_memory();

        validate();
    }

    // Conclude
    {
        // Clean up
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

    // Load input image
    mem_base_addr = base_addr_1; // 128*128
    ESP_REPORT_INFO("[load_memory]: mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < num_col; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = index;
        // convert fx to bv
        data_bv.range(63, 0) = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    ESP_REPORT_INFO("---- dump memory ----");
    uint32_t mem_base_addr = 0;

    mem_base_addr = base_addr_0;
    ESP_REPORT_INFO("[dump_memory]: mem_base_addr: %d", mem_base_addr);

    for (uint32_t index = 0; index < num_row; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        data_bv = mem[mem_base_addr + index];
        // convert bv to fp
        data_fp = bv2fp<FPDATA, FPDATA_WL>(data_bv);
        // conver fp to float
        output[index] = (float)data_fp;
    }

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    ESP_REPORT_INFO("---- validate ----");
    int error_count = 0;
    int total       = 0;

    for (int i = 0; i < p2p_config_0 * p2p_config_1; i++) {
        if (output[i] != (float)(i + 1)) {
            error_count++;
            printf("output[%d] = %f\n", i, output[i]);
        }
    }

    ESP_REPORT_INFO("====================================================");
    if (error_count > 0) {
        ESP_REPORT_INFO("Errors: %d out of %d pixel(s) not match.", error_count, total);
    } else {
        ESP_REPORT_INFO("Correct!!");
    }
    ESP_REPORT_INFO("====================================================");

    ESP_REPORT_INFO("Validate completed");

    return error_count;
}
