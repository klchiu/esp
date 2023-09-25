// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "wami_steep_descent_hessian.hpp"
#include "wami_steep_descent_hessian_directives.hpp"

// [humu]: use baseline
#define BASELINE

// Functions

#include "wami_steep_descent_hessian_functions.hpp"

// Processes

void wami_steep_descent_hessian::load_input()
{
    uint32_t mem_src_dst_offset_2 = 0;
    uint32_t mem_src_dst_offset_3 = 0;
    uint32_t img_num_col          = 0;
    uint32_t img_num_row          = 0;
    uint32_t img_num_img          = 0;
    uint32_t img_batch            = 0;

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
        mem_src_dst_offset_2 = config.src_dst_offset_2;
        mem_src_dst_offset_3 = config.src_dst_offset_3;
        img_num_row          = config.num_row;
        img_num_col          = config.num_col;
        img_num_img          = config.num_img;
        img_batch            = config.batch;
    }

    // Load
    //-- printf("[load_input]: debug 3\n");

    for (uint32_t img = 0; img < img_num_img; img++) {

        HLS_PROTO("load-dma");

        this->load_store_handshake();

        // == BURST 1 (read first input) ====================================================
        uint32_t   index  = mem_src_dst_offset_2;
        uint32_t   length = img_num_col * img_num_row;
        dma_info_t dma_info_1(index, length, SIZE_WORD);
        this->dma_read_ctrl.put(dma_info_1);
        //-- printf("[load_input]: debug 6, index: %d, length: %d\n", index, length);

        for (unsigned i = 0; i < length; i++) {
            sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
            wait();

            HLS_BREAK_DEP(A0_in1);

            FPDATA_WORD pixel_fp;
            pixel_fp = pixel_bv.range(63, 0).to_int64();

            A0_in1[i] = pixel_fp;
            wait();
        }
        //-- printf("[load_input]: debug 7\n");

        // == BURST 2 (read second input) ====================================================
        index  = mem_src_dst_offset_3;
        length = img_num_col * img_num_row;
        dma_info_t dma_info_2(index, length, SIZE_WORD);
        this->dma_read_ctrl.put(dma_info_2);
        //-- printf("[load_input]: debug 8, index: %d, length: %d\n", index, length);

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
        //-- printf("[load_input]: debug 11\n");
    }

    //-- printf("[load_input]: debug 12\n");
    // Conclude
    {
        HLS_PROTO("load-done");
        this->process_done();
    }
}

void wami_steep_descent_hessian::store_output()
{
    uint32_t mem_src_dst_offset_0 = 0;
    uint32_t mem_src_dst_offset_1 = 0;
    uint32_t img_batch            = 0;
    uint32_t img_num_col          = 0;
    uint32_t img_num_row          = 0;
    uint32_t img_num_img          = 0;
    uint32_t num_output_copy      = 0;
    uint32_t index, length;

    //-- printf("[store_output]: debug 1\n");
    // Reset
    {
        HLS_PROTO("store-reset");

        this->reset_store_output();
        accel_ready.req.reset_req();

        // User-defined reset code
        wait();
    }

    //-- printf("[store_output]: debug 2\n");
    // Config
    {
        HLS_PROTO("store-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        mem_src_dst_offset_0 = config.src_dst_offset_0;
        mem_src_dst_offset_1 = config.src_dst_offset_1;
        img_batch            = config.batch;
        img_num_col          = config.num_col;
        img_num_row          = config.num_row;
        img_num_img          = config.num_img;
        num_output_copy      = config.src_dst_offset_4;
    }

    for (uint32_t img = 0; img < img_num_img; img++) {
        HLS_PROTO("store-dma");

        this->store_load_handshake();
        this->store_compute_handshake();

        for (unsigned x = 0; x < num_output_copy; x++) {

            // output to sd update
            // Configure DMA write
            index  = mem_src_dst_offset_0;
            length = 6 * img_num_col * img_num_row;
            dma_info_t dma_info_1(index, length, SIZE_WORD);
            wait();
            this->dma_write_ctrl.put(dma_info_1);
            wait();
            //-- fprintf(stderr, "[store_output]: debug 4.2, index = %d, length = %d\n", index, length);

            for (unsigned i = 0; i < length; i++) {

                // HLS_BREAK_DEP(B0_steep_descent);
                HLS_BREAK_DEP(A0_steepest_descent);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                // pixel_fp = B0_steep_descent[i];
                pixel_fp = A0_steepest_descent[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
                wait();
            }

            // output to matrix inv
            //-- printf("[store_output]: debug 4\n");
            // Configure DMA write
            /*  // [humu]: comment this out for p2p combo3 testing (this is wrong)
            uint32_t   index  = mem_src_dst_offset_1;
            uint32_t   length = 36;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < length; i++) {

                HLS_BREAK_DEP(B0_hess);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = B0_hess[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
            */
        }
    }

    // Conclude
    {
        HLS_PROTO("store-done");
        this->accelerator_done();
        this->process_done();
    }
}

void wami_steep_descent_hessian::compute_kernel()
{
    uint32_t img_num_row = 0;
    uint32_t img_num_col = 0;
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
        img_num_row        = config.num_row;
        img_num_col        = config.num_col;
        img_num_img        = config.num_img;
    }

    // Compute
    for (uint16_t n = 0; n < img_num_img; n++) {
        HLS_UNROLL_LOOP(OFF);

        this->compute_load_handshake();

        // Computing phase implementation
        sd_descent(img_num_row, img_num_col);

        hess(img_num_col, img_num_row, 6);

        this->compute_store_handshake();
    }

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
