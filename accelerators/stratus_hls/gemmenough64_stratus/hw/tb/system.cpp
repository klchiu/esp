// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include <sstream>
#include "system.hpp"

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
    load_memory();
    {
        conf_info_t config;
        // Custom configuration
        /* <<--params-->> */
        config.ninputs    = ninputs;
        config.d3         = d3;
        config.d2         = d2;
        config.d1         = d1;
        config.st_offset  = st_offset;
        config.ld_offset1 = ld_offset1;
        config.ld_offset2 = ld_offset2;

        wait();
        conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - gemmenough64");

        // Wait the termination of the accelerator
        do {
            wait();
        } while (!acc_done.read());
        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - gemmenough64");

        esc_log_latency(sc_object::basename(), clock_cycle(end_time - begin_time));
        wait();
        conf_done.write(false);
    }

    // Validate
    dump_memory(); // store the output in more suitable data structure if needed

    // check the results with the golden model
    validate();

    // Conclude
    {
        sc_stop();
    }
}

// Functions
void system_t::load_memory()
{
    // Optional usage check
#ifdef CADENCE
    if (esc_argc() != 1) {
        ESP_REPORT_INFO("usage: %s\n", esc_argv()[0]);
        sc_stop();
    }
#endif

    // Input data and golden output (aligned to DMA_WIDTH makes your life easier)
#if (DMA_WORD_PER_BEAT == 0)
    in_words_adj  = 2 * ld_offset2;
    out_words_adj = st_offset;
#else
    in_words_adj  = round_up(2 * ld_offset2, DMA_WORD_PER_BEAT);
    out_words_adj = round_up(st_offset, DMA_WORD_PER_BEAT);
#endif

    in_size  = in_words_adj * (1);
    out_size = out_words_adj * (1);

    ESP_REPORT_INFO("[humu]: load_memory() debug 1, in_size = %d, out_size = %d", in_size, out_size);

    // Initialize input
    in = new int32_t[in_size];
    for (int i = 0; i < 1; i++)
        for (int j = 0; j < 2 * ld_offset2; j++)
            in[i * in_words_adj + j] = (int32_t)(j % 1000);

    // for (int i = 0; i < d1; i++) {
    //     for (int j = 0; j < d2; j++) {
    //         std::cout << "in[" << i * d2 + j << "] = " << in[i * d2 + j] << std::endl;
    //     }
    // }

    // for (int i = 0; i < d2; i++) {
    //     for (int j = 0; j < d3; j++) {
    //         std::cout << "in[" << d1 * d2 + i * d3 + j << "] = " << in[d1 * d2 + i * d3 + j] << std::endl;
    //     }
    // }

    // Compute golden output
    gold = new int32_t[out_size];
    // for (int i = 0; i < 1; i++)
    //     for (int j = 0; j < st_offset; j++)
    //         gold[i * out_words_adj + j] = (int32_t) j;

    // Initialize the result matrix C to all 0s
    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d3; j++) {
            gold[i * d3 + j] = 0;
        }
    }

    // Perform matrix multiplication
    for (int i = 0; i < d1; i++) {
        for (int j = 0; j < d3; j++) {
            for (int k = 0; k < d2; k++) {
                // Access elements in 1D arrays
                gold[i * d3 + j] += in[i * d2 + k] * in[d1 * d2 + k * d3 + j];
            }
        }
    }

    // for (int i = 0; i < d1; i++) {
    //     for (int j = 0; j < d3; j++) {
    //         std::cout << "gold[" << i * d3 + j << "] = " << gold[i * d3 + j] << std::endl;
    //     }
    // }

    // Memory initialization:
#if (DMA_WORD_PER_BEAT == 0)
    for (int i = 0; i < in_size; i++) {
        sc_dt::sc_bv<DATA_WIDTH> data_bv(in[i]);
        for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
            mem[DMA_BEAT_PER_WORD * i + j] = data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH);
    }
#else
    for (int i = 0; i < in_size / DMA_WORD_PER_BEAT; i++) {
        sc_dt::sc_bv<DMA_WIDTH> data_bv(in[i]);
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
            data_bv.range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH) = in[i * DMA_WORD_PER_BEAT + j];
        mem[i] = data_bv;
    }
#endif

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    // ESP_REPORT_INFO("[humu]: dump_memory() start");

    // Get results from memory
    out             = new int32_t[out_size];
    uint32_t offset = in_size;

#if (DMA_WORD_PER_BEAT == 0)
    // ESP_REPORT_INFO("[humu]: dump_memory() debug 1");

    offset = offset * DMA_BEAT_PER_WORD;
    for (int i = 0; i < out_size; i++) {
        sc_dt::sc_bv<DATA_WIDTH> data_bv;

        for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
            data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH) = mem[offset + DMA_BEAT_PER_WORD * i + j];

        out[i] = data_bv.to_int64();
    }
#else
    // ESP_REPORT_INFO("[humu]: dump_memory() debug 2");

    offset = offset / DMA_WORD_PER_BEAT;

    // ESP_REPORT_INFO("[humu]: dump_memory() debug 3: out_size = %d, offset = %d, DMA_WORD_PER_BEAT = %d", out_size,
    //                 offset, DMA_WORD_PER_BEAT);

    for (int i = 0; i < out_size / DMA_WORD_PER_BEAT; i++)
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {

            out[i * DMA_WORD_PER_BEAT + j] = mem[offset + i].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH).to_int64();
        }
#endif

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    // ESP_REPORT_INFO("[humu]: validate() start, out_words_adj = %d, st_offset = %d", out_words_adj, st_offset);

    // Check for mismatches
    uint32_t errors = 0;

    for (int i = 0; i < d1 * d3; i++) {
        if (gold[i] != out[i]) {
            errors++;
            if (errors < 5) {
                std::cout << "gold[" << i << "] = " << gold[i] << "\tout[" << i << "] = " << out[i] << std::endl;
            }
        }
        if (i < 5) {
            std::cout << "gold[" << i << "] = " << gold[i] << "\tout[" << i << "] = " << out[i] << std::endl;
        }
    }

    delete[] in;
    delete[] out;
    delete[] gold;

    if (errors) {
        ESP_REPORT_ERROR("==> validation FAILED! # of errors: %d", errors);
    } else {
        ESP_REPORT_INFO("==> validation PASSED!");
    }

    ESP_REPORT_INFO("[humu]: validate() done: %d", errors);

    return errors;
}
