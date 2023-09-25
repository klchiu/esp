// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "wami_add3.hpp"
#include "wami_add3_directives.hpp"


// Functions

#include "wami_add3_functions.hpp"

// Processes

void wami_add3::load_input()
{
    uint32_t img_num_img          = 0;
    uint32_t mem_src_dst_offset_1 = 0;
    uint32_t mem_src_dst_offset_2 = 0;
    uint32_t is_p2p               = 0;
    uint32_t index, length;

    // Reset
    {
        HLS_PROTO("load-reset");

        this->reset_load_input();
        accel_ready.ack.reset_ack();

        // User-defined reset code

        wait();
    }

    // Config
    {
        HLS_PROTO("load-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        img_num_img          = config.num_img;
        mem_src_dst_offset_1 = config.src_dst_offset_1;
        mem_src_dst_offset_2 = config.src_dst_offset_2;
        is_p2p               = config.is_p2p;
    }

    // Load
    if (is_p2p == 0) { // normal shared memory version

        for (uint32_t img = 0; img < img_num_img; img++) {

            HLS_PROTO("load-dma");

            this->load_store_handshake();

            // == BURST (read first input) ==
            index  = mem_src_dst_offset_1;
            length = 6;
            dma_info_t dma_info_1(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_1);

            for (unsigned i = 0; i < 6; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_affine_params);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.range(63, 0).to_int64();

                A0_affine_params[i] = pixel_fp;
                wait();
            }

            // == BURST (read second input) ==
            index  = mem_src_dst_offset_2;
            length = 6;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < 6; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_sd_delta_p);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_sd_delta_p[i] = pixel_fp;
                wait();
            }

            this->load_compute_handshake();
        }
    } else { // p2p version

        // the initial first input is 0
        for (unsigned i = 0; i < 6; i++) {
            HLS_PROTO("load-dma-p2p-pre0");

            HLS_BREAK_DEP(A0_affine_params);

            A0_affine_params[i] = 0;
            wait();
        }

        for (uint32_t img = 0; img < img_num_img; img++) {

            HLS_PROTO("load-dma-p2p");

            this->load_store_handshake();

            // == BURST (read second input) ==
            index  = mem_src_dst_offset_2;
            length = 6;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < 6; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_sd_delta_p);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_sd_delta_p[i] = pixel_fp;
                wait();
            }

            this->load_compute_handshake();
        }
    }

    // Conclude
    {
        HLS_PROTO("load-done");
        this->process_done();
    }
}

void wami_add3::store_output()
{
    uint32_t img_num_img          = 0;
    uint32_t mem_src_dst_offset_0 = 0;
    uint32_t is_p2p               = 0;
    uint32_t num_forward_pass     = 0;
    uint32_t num_backward_pass    = 0;
    uint32_t index, length;

    // Reset
    {
        HLS_PROTO("store-reset");

        this->reset_store_output();
        accel_ready.req.reset_req();

        // User-defined reset code
        wait();
    }

    // Config
    {
        HLS_PROTO("store-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        img_num_img          = config.num_img;
        mem_src_dst_offset_0 = config.src_dst_offset_0;
        is_p2p               = config.is_p2p;
        num_forward_pass     = config.p2p_config_0;
        num_backward_pass    = config.p2p_config_1;
    }

    if (is_p2p == 0) { // normal shared memory version

        for (uint32_t img = 0; img < img_num_img; img++) {
            HLS_PROTO("store-dma");

            this->store_load_handshake();
            this->store_compute_handshake();

            // Configure DMA write
            index  = mem_src_dst_offset_0;
            length = 6;
            dma_info_t dma_info(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info);

            for (unsigned i = 0; i < 6; i++) {

                HLS_BREAK_DEP(B0_affine_params);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = B0_affine_params[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
        }
    } else { // p2p version
        // send the output num_backward_pass times to first
        for (unsigned x = 0; x < num_backward_pass; x++) {
            HLS_PROTO("store-dma-p2p-1");
            // Configure DMA write
            index  = mem_src_dst_offset_0;
            length = 6;
            dma_info_t dma_info_p2p(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info_p2p);

            for (unsigned i = 0; i < 6; i++) {

                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = 0;

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
        }

        for (uint32_t img = 0; img < img_num_img - 1; img++) {
            HLS_PROTO("store-dma-p2p");

            this->store_load_handshake();
            this->store_compute_handshake();

            for (unsigned x = 0; x < num_backward_pass + num_forward_pass; x++) {
                // Configure DMA write
                index  = mem_src_dst_offset_0;
                length = 6;
                dma_info_t dma_info_p2p(index, length, SIZE_WORD);
                this->dma_write_ctrl.put(dma_info_p2p);

                for (unsigned i = 0; i < 6; i++) {

                    HLS_BREAK_DEP(B0_affine_params);
                    FPDATA_WORD             pixel_fp;
                    sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                    wait();
                    pixel_fp = B0_affine_params[i];

                    // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                    pixel_bv.range(63, 0) = pixel_fp;

                    wait();
                    this->dma_write_chnl.put(pixel_bv);
                }
            }
        }

        {
            // need to send the last one to warp-iwxp (num_backward_pass = 1)
            for (unsigned x = 0; x < num_forward_pass; x++) {

                HLS_PROTO("store-dma-p2p-last-to-iwxp");
                this->store_load_handshake();
                this->store_compute_handshake();

                // Configure DMA write
                index  = mem_src_dst_offset_0;
                length = 6;
                dma_info_t dma_info_p2p(index, length, SIZE_WORD);
                this->dma_write_ctrl.put(dma_info_p2p);

                for (unsigned i = 0; i < 6; i++) {

                    HLS_BREAK_DEP(B0_affine_params);
                    FPDATA_WORD             pixel_fp;
                    sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                    wait();
                    pixel_fp = B0_affine_params[i];

                    // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                    pixel_bv.range(63, 0) = pixel_fp;

                    wait();
                    this->dma_write_chnl.put(pixel_bv);
                }
            }
        }
    }

    // Conclude
    {
        HLS_PROTO("store-done");
        this->accelerator_done();
        this->process_done();
    }
}

void wami_add3::compute_kernel()
{
    uint32_t img_num_img = 0;
    // Reset
    {
        HLS_PROTO("compute-reset");

        this->reset_compute_kernel();

        // User-defined reset code
        wait();
    }

    {
        HLS_PROTO("compute-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        img_num_img = config.num_img;
    }

    // Compute
    for (uint16_t n = 0; n < img_num_img; n++) {
        HLS_UNROLL_LOOP(OFF);

        this->compute_load_handshake();

        // Computing phase implementation
        addition(6, 1);

        this->compute_store_handshake();
    }

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
