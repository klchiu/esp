// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "wami_sub3.hpp"
#include "wami_sub3_directives.hpp"


// Functions

#include "wami_sub3_functions.hpp"

// Processes

void wami_sub3::load_input()
{
    uint32_t img_num_img          = 0;
    uint32_t img_num_row          = 0;
    uint32_t img_num_col          = 0;
    uint32_t mem_src_dst_offset_1 = 0;
    uint32_t mem_src_dst_offset_2 = 0;

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
        img_num_row          = config.num_row;
        img_num_col          = config.num_col;
        mem_src_dst_offset_1 = config.src_dst_offset_1;
        mem_src_dst_offset_2 = config.src_dst_offset_2;
    }

    // Load

    for (uint32_t img = 0; img < img_num_img; img++) {

        HLS_PROTO("load-dma");

        this->load_store_handshake();

        // == BURST (read first input) ==
        uint32_t   index  = mem_src_dst_offset_1;
        uint32_t   length = img_num_row * img_num_col;
        dma_info_t dma_info_1(index, length, SIZE_WORD);
        this->dma_read_ctrl.put(dma_info_1);

        for (unsigned i = 0; i < length; i++) {
            sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
            wait();

            HLS_BREAK_DEP(A0_in1);

            FPDATA_WORD pixel_fp;
            pixel_fp = pixel_bv.range(63, 0).to_int64();

            A0_in1[i] = pixel_fp;
            wait();
        }

        // == BURST (read second input) ==
        index  = mem_src_dst_offset_2;
        length = img_num_row * img_num_col;
        dma_info_t dma_info_2(index, length, SIZE_WORD);
        this->dma_read_ctrl.put(dma_info_2);

        for (unsigned i = 0; i < length; i++) {
            sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
            wait();

            HLS_BREAK_DEP(A0_in2);

            FPDATA_WORD pixel_fp;
            pixel_fp = pixel_bv.to_int64();

            A0_in2[i] = pixel_fp;
            wait();
        }

        this->load_compute_handshake();
    }

    // Conclude
    {
        HLS_PROTO("load-done");
        this->process_done();
    }
}

void wami_sub3::store_output()
{
    uint32_t img_num_img          = 0;
    uint32_t img_num_row          = 0;
    uint32_t img_num_col          = 0;
    uint32_t mem_src_dst_offset_0 = 0;

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
        img_num_row          = config.num_row;
        img_num_col          = config.num_col;
        mem_src_dst_offset_0 = config.src_dst_offset_0;
    }

    for (uint32_t img = 0; img < img_num_img; img++) {
        HLS_PROTO("store-dma");

        this->store_load_handshake();
        this->store_compute_handshake();

        // Configure DMA write
        uint32_t   index  = mem_src_dst_offset_0;
        uint32_t   length = img_num_row * img_num_col;
        dma_info_t dma_info(index, length, SIZE_WORD);
        this->dma_write_ctrl.put(dma_info);

        for (unsigned i = 0; i < length; i++) {

            HLS_BREAK_DEP(B0_out);
            FPDATA_WORD             pixel_fp;
            sc_dt::sc_bv<WORD_SIZE> pixel_bv;

            wait();
            pixel_fp = B0_out[i];

            // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
            pixel_bv.range(63, 0) = pixel_fp;

            wait();
            this->dma_write_chnl.put(pixel_bv);
        }
    }

    // Conclude
    {
        HLS_PROTO("store-done");
        this->accelerator_done();
        this->process_done();
    }
}

void wami_sub3::compute_kernel()
{
    uint32_t img_num_img = 0;
    uint32_t img_num_col = 0;
    uint32_t img_num_row = 0;

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
        img_num_img        = config.num_img;
        img_num_row        = config.num_row;
        img_num_col        = config.num_col;
    }

    // Compute
    for (uint16_t n = 0; n < img_num_img; n++) {
        HLS_UNROLL_LOOP(OFF);

        this->compute_load_handshake();

        // Computing phase implementation
        subtract(img_num_row, img_num_col);
        this->compute_store_handshake();
    }

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
