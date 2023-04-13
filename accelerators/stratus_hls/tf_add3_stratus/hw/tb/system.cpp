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

std::ofstream ofs;


void system_t::malloc_arrays(int len)
{
    output_0 = (float*)malloc(sizeof(float) * len);
    input_1 = (float*)malloc(sizeof(float) * len);
    input_2 = (float*)malloc(sizeof(float) * len);
    gold_0 = (float*)malloc(sizeof(float) * len);
}

void system_t::free_arrays()
{
    free(output_0);
    free(input_1);
    free(input_2);
    free(gold_0);
}

void system_t::init_arrays(int len)
{
    int i;

    for (i = 0 ; i < len; i++){
        float val_1 = i *100 / 3.0 - 17;
        float val_2 = i *100 / 9.0 - 17;

        input_1[i] = val_1;
        input_2[i] = val_2;

if(i < 10){
        ESP_REPORT_INFO("%d: val_1: %f\tval_2: %f", i, val_1, val_2);
}

    }
}

void system_t::run_pv(int len)
{
    int i;
    for (i = 0 ; i < len; i++){
        gold_0[i] = input_1[i] + input_2[i];
    }
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


    length = 2 << 15;

    malloc_arrays(length);

    init_arrays(length);

    run_pv(length);

    // Config
    {
        // length = 1024;
        base_addr_0  = 0;
        base_addr_1  = length;
        base_addr_2  = length + length;
        chunk_size = 64;

        conf_info_t config(length, base_addr_0, base_addr_1, base_addr_2, chunk_size);

        ESP_REPORT_INFO("[config]: length = %d", config.length);
        ESP_REPORT_INFO("[config]: base_addr_0 = %d", config.src_dst_offset_0);
        ESP_REPORT_INFO("[config]: base_addr_1 = %d", config.src_dst_offset_1);
        ESP_REPORT_INFO("[config]: base_addr_2 = %d", config.src_dst_offset_2);
        ESP_REPORT_INFO("[config]: chunk_size = %d", config.chunk_size);

        wait();
        conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Load input data
    load_memory();

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - tf_add3");

        // Wait the termination of the accelerator
        do {
            wait();
        } while (!acc_done.read());

        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - tf_add3");

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

    // Load first input
    mem_base_addr = base_addr_1;
    ESP_REPORT_INFO("[load_memory]: mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < length; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(input_1[index]);
        // convert fx to bv
        data_bv.range(63, 0) = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    // Load second input
    mem_base_addr = base_addr_2;
    ESP_REPORT_INFO("[load_memory]: mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < length; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        // convert float to fx
        data_fp = FPDATA(input_2[index]);
        // convert fx to bv
        data_bv = fp2bv<FPDATA, FPDATA_WL>(data_fp);

        mem[mem_base_addr + index].range(63, 0) = data_bv;
    }

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    uint32_t mem_base_addr = 0;

    mem_base_addr = base_addr_0;
    ESP_REPORT_INFO("[dump_memory]: mem_base_addr: %d", mem_base_addr);
    for (uint32_t index = 0; index < length; index++) {
        sc_dt::sc_bv<FPDATA_WL> data_bv;
        FPDATA                  data_fp;

        data_bv = mem[mem_base_addr + index];
        // convert bv to fp
        data_fp = bv2fp<FPDATA, FPDATA_WL>(data_bv);
        // conver fp to float
        output_0[index] = (float)data_fp;
        // printf("[dump] : %d\t%f\n", index, sub[i]);
    }

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    ESP_REPORT_INFO("---- validate ----");

    int error_count = 0;
    int total       = 0;

    for (uint32_t i = 0; i < length; i++) {
        if (abs(gold_0[i] - output_0[i]) > 0.001) {
            error_count++;
            if (error_count < 5) {
                ESP_REPORT_INFO("%d: out: %f\tgold: %f", i, output_0[i], gold_0[i]);
            }
        }
                    if (total < 5) {
                ESP_REPORT_INFO("%d: out: %f\tgold: %f", i, output_0[i], gold_0[i]);
            }

        total++;
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
