// Copyright (c) 2011-2024 Columbia University, System Level Design Group
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
        config.rows = 4;
        config.cols = 4;
        config.loaded_cols = 4;

        wait(); 
        conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - huan_huangemm3s10000");

        // Wait the termination of the accelerator
        do { wait(); } while (!acc_done.read());
        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - huan_huangemm3s10000");

        esc_log_latency(sc_object::basename(), clock_cycle(end_time - begin_time));
        wait(); conf_done.write(false);
    }

    // Validate
    {
        dump_memory(); // store the output in more suitable data structure if needed
        // check the results with the golden model
        if (validate())
        {
            ESP_REPORT_ERROR("validation failed!");
        } else
        {
            ESP_REPORT_INFO("validation passed!");
        }
    }

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
    if (esc_argc() != 1)
    {
        ESP_REPORT_INFO("usage: %s\n", esc_argv()[0]);
        sc_stop();
    }
#endif

    // Input data and golden output (aligned to DMA_WIDTH makes your life easier)
#if (DMA_WORD_PER_BEAT == 0)
    // A_size_adj = loaded_cols * rows;
    // B_size_adj = loaded_cols * cols;
    // O_size_adj = rows * cols;
#else
    // A_size_adj = loaded_cols * rows + (loaded_cols * rows) % DMA_WORD_PER_BEAT 
    A_size_adj = round_up(loaded_cols*rows, DMA_WORD_PER_BEAT);
    B_size_adj = round_up(loaded_cols*cols, DMA_WORD_PER_BEAT);
    O_size_adj = round_up(rows*cols, DMA_WORD_PER_BEAT);
#endif
    A_size = A_size_adj;
    B_size = B_size_adj;
    O_size = O_size_adj;

    // Initialize input
    // TPH: need to change it to fixed point
    ESP_REPORT_INFO("TB -- A size: %d\n", A_size);
    ESP_REPORT_INFO("TB -- B size: %d\n", B_size);
    ESP_REPORT_INFO("TB -- O size: %d\n", O_size);
    
    inA = new int32_t[A_size];
    inA[0] = 1;
    inA[1] = 5;
    inA[2] = 9;
    inA[3] = 13;
    inA[4] = 2;
    inA[5] = 6;
    inA[6] = 10;
    inA[7] = 14;
    inA[8] = 3;
    inA[9] = 7;
    inA[10] = 11;
    inA[11] = 15;
    inA[12] = 4;
    inA[13] = 8;
    inA[14] = 12;
    inA[15] = 16;

    inB = new int32_t[B_size];
    inB[0] = 0+1;
    inB[1] = 1+1;
    inB[2] = 2+1;
    inB[3] = 3+1;
    inB[4] = 4+1;
    inB[5] = 5+1;
    inB[6] = 6+1;
    inB[7] = 7+1;
    inB[8] = 8+1;
    inB[9] = 9+1;
    inB[10] = 10+1;
    inB[11] = 11+1;
    inB[12] = 12+1;
    inB[13] = 13+1;
    inB[14] = 14+1;
    inB[15] = 15+1;




    // for (int i = 0; i < huangemm3s10000_n; i++)
    //     for (int j = 0; j < huangemm3s10000_len * huangemm3s10000_vec; j++)
    //         in[i * in_words_adj + j] = j % huangemm3s10000_vec;

    // Compute golden output
    // [ 90 100 110 120 202 228 254 280 314 356 398 440 426 484 542 600]
    gold = new int32_t[O_size];
    gold[0] = 90;
    gold[1] = 100;
    gold[2] = 110;
    gold[3] = 120;
    gold[4] = 202;
    gold[5] = 228;
    gold[6] = 254;
    gold[7] = 280;
    gold[8] = 314;
    gold[9] = 356;
    gold[10] = 398;
    gold[11] = 440;
    gold[12] = 426;
    gold[13] = 484;
    gold[14] = 542;
    gold[15] = 600;


    // Memory initialization:
#if (DMA_WORD_PER_BEAT == 0)
    // for (int i = 0; i < in_size; i++)  {
    //     sc_dt::sc_bv<DATA_WIDTH> data_bv(in[i]);
    //     for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
    //         mem[DMA_BEAT_PER_WORD * i + j] = data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH);
    // }
#else
    
    for (int i = 0; i < A_size / DMA_WORD_PER_BEAT; i++)  {
        sc_dt::sc_bv<DMA_WIDTH> data_bv(inA[i]);
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
            data_bv.range((j+1) * DATA_WIDTH - 1, j * DATA_WIDTH) = inA[i * DMA_WORD_PER_BEAT + j];
        mem[i] = data_bv;
    }
    int offset_B = A_size / DMA_WORD_PER_BEAT;
    for (int i = 0; i < B_size / DMA_WORD_PER_BEAT; i++)  {
        sc_dt::sc_bv<DMA_WIDTH> data_bv(inB[i]);
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
            data_bv.range((j+1) * DATA_WIDTH - 1, j * DATA_WIDTH) = inB[i * DMA_WORD_PER_BEAT + j];
        mem[offset_B+i] = data_bv;
    }

    

#endif

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    // Get results from memory
    int offset_O = A_size / DMA_WORD_PER_BEAT + B_size / DMA_WORD_PER_BEAT;
    outO = new int32_t[O_size];
    // uint32_t offset = in_size;

#if (DMA_WORD_PER_BEAT == 0)
    // offset = offset * DMA_BEAT_PER_WORD;
    // for (int i = 0; i < out_size; i++)  {
    //     sc_dt::sc_bv<DATA_WIDTH> data_bv;

    //     for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
    //         data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH) = mem[offset + DMA_BEAT_PER_WORD * i + j];

    //     out[i] = data_bv.to_int64();
    // }
#else
    // offset = offset / DMA_WORD_PER_BEAT;
    for (int i = 0; i < O_size / DMA_WORD_PER_BEAT; i++)
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++)
            outO[i * DMA_WORD_PER_BEAT + j] = mem[offset_O + i].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH).to_int64();
#endif

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    // Check for mismatches
    uint32_t errors = 0;

    // for (int i = 0; i < huangemm3s10000_n; i++)
    //     for (int j = 0; j < huangemm3s10000_vec; j++)
    //         if (gold[i * out_words_adj + j] != outO[i * out_words_adj + j])
    //             errors++;
    
    for (int i = 0; i < O_size; i++){
        if (gold[i] != outO[i]){
            // ESP_REPORT_INFO("gold[%d] = %d, outO[%d] = %d", i, gold[i], i, outO[i]);
            // ESP_REPORT_INFO("dump memory completed");
            errors++;
        }
    }

    delete [] inA;
    delete [] inB;
    delete [] outO;
    delete [] gold;

    return errors;
}
