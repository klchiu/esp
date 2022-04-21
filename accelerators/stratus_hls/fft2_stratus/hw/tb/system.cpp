// Copyright (c) 2011-2019 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include <random>
#include <sstream>
#include "system.hpp"

// Helper random generator
static std::uniform_real_distribution<float> *dis;
static std::random_device                     rd;
static std::mt19937 *                         gen;

#include "in_data"
#include "out_data"

static void init_random_distribution(void)
{
    const float LO = -1.5;
    const float HI = 1.5;

    gen = new std::mt19937(rd());
    dis = new std::uniform_real_distribution<float>(LO, HI);
}

static float gen_random_float(void) { return (*dis)(*gen); }

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
        config.logn_samples = logn_samples;
        config.num_ffts     = num_ffts;
        config.do_inverse   = do_inverse;
        config.do_shift     = do_shift;
        config.scale_factor = scale_factor;

        wait();
        conf_info.write(config);
        conf_done.write(true);
    }

    ESP_REPORT_INFO("config done");

    // Compute
    {
        // Print information about begin time
        sc_time begin_time = sc_time_stamp();
        ESP_REPORT_TIME(begin_time, "BEGIN - fft2");

        // Wait the termination of the accelerator
        do {
            wait();
        } while (!acc_done.read());
        debug_info_t debug_code = debug.read();

        // Print information about end time
        sc_time end_time = sc_time_stamp();
        ESP_REPORT_TIME(end_time, "END - fft2");

        esc_log_latency(sc_object::basename(), clock_cycle(end_time - begin_time));
        wait();
        conf_done.write(false);
    }

    // Validate
    {
        const double ERROR_COUNT_TH = 0.02;
        dump_memory(); // store the output in more suitable data structure if needed
        // check the results with the golden model
        int    errs    = validate();
        double pct_err = ((double)errs / (double)(2 * num_ffts * num_samples));

        fprintf(stderr, "DMA_WIDTH: %u\n", DMA_WIDTH);
        fprintf(stderr, "FX_WIDTH: %u\n", FX_WIDTH);
        fprintf(stderr, "WORD_SIZE = %d\n", WORD_SIZE);
        fprintf(stderr, "FPDATA_IL = %d\n", FPDATA_IL);
        fprintf(stderr, "num_ffts: %u\n", num_ffts);
        fprintf(stderr, "logn_samples: %u\n", logn_samples);
        fprintf(stderr, "errs: %u\n", errs);
        fprintf(stderr, "in_size: %u\n", (2 * num_ffts * num_samples));
        fprintf(stderr, "out_size: %u\n", (2 * num_ffts * num_samples));
        fprintf(stderr, "pct_err: %u\n", pct_err);
        fprintf(stderr, "ERROR_COUNT_TH: %u\n", ERROR_COUNT_TH);

        ESP_REPORT_INFO("-------------------------------------");
        if (pct_err > ERROR_COUNT_TH) {
            ESP_REPORT_INFO("Validation Failed!");
        } else {
            ESP_REPORT_INFO("Validation Passed!");
        }
        ESP_REPORT_INFO("-------------------------------------");
    }

    // Conclude
    {
        sc_stop();
    }
}

// Functions
void system_t::generate_input()
{
    // [humu]: move the part here
}

void system_t::compute_golden_output()
{
    // [humu]: move the part here
}

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
    in_words_adj  = 2 * num_ffts * num_samples;
    out_words_adj = in_words_adj;
#else
    in_words_adj  = round_up(2 * num_ffts * num_samples, DMA_WORD_PER_BEAT);
    out_words_adj = in_words_adj;
#endif

    in_size  = in_words_adj;
    out_size = out_words_adj;
    fprintf(stderr, "load_memory: MEM_SIZE = %u\n", MEM_SIZE);
    fprintf(stderr, "load_memory: in_size = %u\n", in_size);
    fprintf(stderr, "load_memory: out_size = %u\n", out_size);
    fprintf(stderr, "load_memory: gold_size = %u\n", out_size);

    if (in_size > MEM_SIZE) {
        ESP_REPORT_INFO("[ERROR]: in_size should be smaller than MEM_SIZE");
    }
    if (out_size > MEM_SIZE) {
        ESP_REPORT_INFO("[ERROR]: out_size should be smaller than MEM_SIZE");
    }

    init_random_distribution();
    in = new float[in_size];

    fprintf(stderr, "load_memory: in[%d] @ %p\n", 0, &in[0]);
    fprintf(stderr, "load_memory: in[%d] @ %p\n", (in_size - 1), &in[in_size - 1]);
    for (int j = 0; j < in_size; j++) {
        if (use_input_files) {
            in[j] = FFT_INPUTS[j];
        } else {
            in[j] = gen_random_float();
        }
    }

    fprintf(stderr, "---- IN_DATA (only print first 10):\n");
    for (int j = 0; j < 10; j++) {
        // in[j] = ((float)j); ///10000.0;
        fprintf(stderr, "    %.15f,\n", in[j]);
    }
    fprintf(stderr, "---- end of IN_DATA\n");

    // Compute golden output
    gold = new float[out_size];
    memcpy(gold, in, out_size * sizeof(float));

    fft2_comp(gold, num_ffts, num_samples, logn_samples, do_inverse, do_shift); // do_bitrev is always true

    // fprintf(stderr, "GOLD_OUT_DATA:\n");
    // fprintf(stderr, "float FFT_OUTPUTS[%u] = {\n", 2 * num_ffts * num_samples);
    // for (int j = 0; j < 2 * num_ffts * num_samples; j++) {
    //     fprintf(stderr, "    %.15f,\n", gold[j]);
    // }
    // fprintf(stderr, "};\nEND_OF_GOLD_OUT_DATA\n");

    // Memory initialization:
#if (DMA_WORD_PER_BEAT == 0)
    for (int i = 0; i < in_size; i++) {
        sc_dt::sc_bv<DATA_WIDTH> data_bv(in[i]);
        for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
            mem[DMA_BEAT_PER_WORD * i + j] = data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH);
    }
#else
    for (int i = 0; i < in_size / DMA_WORD_PER_BEAT; i++) {
        // fprintf(stderr, "i = %d\n", i);
        sc_dt::sc_bv<DMA_WIDTH> data_bv;
        // fprintf(stderr, "debug 1\n");
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {
            // fprintf(stderr, "j = %d\n", j);
            data_bv.range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH) =
                fp2bv<FPDATA, WORD_SIZE>(FPDATA(in[i * DMA_WORD_PER_BEAT + j]));
        }
        mem[i] = data_bv;
    }
#endif

    ESP_REPORT_INFO("load memory completed");
}

void system_t::dump_memory()
{
    // Get results from memory
    out             = new float[out_size];
    uint32_t offset = 0;

#if (DMA_WORD_PER_BEAT == 0)
    offset = offset * DMA_BEAT_PER_WORD;
    for (int i = 0; i < out_size; i++) {
        sc_dt::sc_bv<DATA_WIDTH> data_bv;

        for (int j = 0; j < DMA_BEAT_PER_WORD; j++)
            data_bv.range((j + 1) * DMA_WIDTH - 1, j * DMA_WIDTH) = mem[offset + DMA_BEAT_PER_WORD * i + j];

        FPDATA out_fx = bv2fp<FPDATA, WORD_SIZE>(data_bv);
        out[i]        = (float)out_fx;
    }
#else
    offset = offset / DMA_WORD_PER_BEAT;
    for (int i = 0; i < out_size / DMA_WORD_PER_BEAT; i++)
        for (int j = 0; j < DMA_WORD_PER_BEAT; j++) {
            FPDATA out_fx = bv2fp<FPDATA, WORD_SIZE>(mem[offset + i].range((j + 1) * DATA_WIDTH - 1, j * DATA_WIDTH));
            out[i * DMA_WORD_PER_BEAT + j] = (float)out_fx;
        }
#endif

    ESP_REPORT_INFO("dump memory completed");
}

int system_t::validate()
{
    // Check for mismatches
    uint32_t    errors1 = 0;
    uint32_t    errors2 = 0;
    uint32_t    errors3 = 0;
    const float ERR_TH  = 0.05;
    const float minV    = 1.0 / (float)(1 << 14);

    fprintf(stderr, "[validate]: minV = %.15f\n", minV);

    for (int nf = 0; nf < num_ffts; nf++) {
        for (int j = 0; j < 2 * num_samples; j++) {
            int   idx  = 2 * nf * num_samples + j;
            float eOvG = (fabs(gold[idx] - out[idx]) / fabs(gold[idx]));
            float eGvO = (fabs(out[idx] - gold[idx]) / fabs(out[idx]));

            // if ((fabs(gold[idx] - out[idx]) / fabs(gold[idx])) > ERR_TH) {
            if ((eOvG > ERR_TH) && (eGvO > ERR_TH)) {
                if (errors1 < 4) {
                    // ESP_REPORT_INFO(" ERROR : GOLD[%u] = %f vs %f = out[%u]\n", idx, gold[idx], out[idx], idx);
                    fprintf(stderr, " ERROR %u : fft %u : idx %u : gold %.15f  out %.15f\n", errors1, nf, idx,
                            gold[idx], out[idx]);
                }
                errors1++;

                if (fabs(gold[j]) > minV) {
                    if (errors2 < 4) {
                        // ESP_REPORT_INFO(" ERROR : GOLD[%u] = %f vs %f = out[%u]\n", idx, gold[idx], out[idx], idx);
                        fprintf(stderr, " ERROR %u : fft %u : idx %u : gold %.15f  out %.15f\n", errors2, nf, idx,
                                gold[idx], out[idx]);
                    }
                    errors2++;
                }

                if (fabs(out[j]) > minV) {
                    if (errors3 < 4) {
                        // ESP_REPORT_INFO(" ERROR : GOLD[%u] = %f vs %f = out[%u]\n", idx, gold[idx], out[idx], idx);
                        fprintf(stderr, " ERROR %u : fft %u : idx %u : gold %.15f  out %.15f\n", errors3, nf, idx,
                                gold[idx], out[idx]);
                    }
                    errors3++;
                }
            }
            /*
            if ((fabs(FFT_OUTPUTS[idx] - out[idx]) / fabs(FFT_OUTPUTS[idx])) > ERR_TH) {
                if (errors2 < 4) {
                    //ESP_REPORT_INFO(" ERROR : FFT_OUTPUTS[%u] = %f vs %f = out[%u]\n", idx, FFT_OUTPUTS[idx],
            out[idx], idx); ESP_REPORT_INFO(" ERROR %u : fft %u : idx %u : FFT_OUTPUTS %.15f  out %.15f", errors2, nf,
            idx, FFT_OUTPUTS[idx], out[idx]);
                }
                errors2++;
            }
            if ((fabs(FFT_OUTPUTS[idx] - gold[idx]) / fabs(FFT_OUTPUTS[idx])) > ERR_TH) {
                if (errors3 < 4) {
                    //ESP_REPORT_INFO(" ERROR : FFT_OUTPUTS[%u] = %f vs %f = gold[%u]\n", idx, FFT_OUTPUTS[idx],
            gold[idx], idx); ESP_REPORT_INFO(" ERROR %u : fft %u : idx %u : FFT_OUTPUTS %.15f  GOLD %.15f", errors3, nf,
            idx, FFT_OUTPUTS[idx], gold[idx]);
                }
                errors3++;
            }
            */
        }
    }

    ESP_REPORT_INFO("DMA_%u_FX_%u: ERR_TH = %f, errors1 = %d, errors2 = %d, errors3 = %d\n", DMA_WIDTH, FX_WIDTH,
                    ERR_TH, errors1, errors2, errors3);

    delete[] in;
    delete[] out;
    delete[] gold;

    return errors1;
}
