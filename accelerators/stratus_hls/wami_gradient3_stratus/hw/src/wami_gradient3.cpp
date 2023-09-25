// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "wami_gradient3.hpp"
#include "wami_gradient3_directives.hpp"


// Functions

#include "wami_gradient3_functions.hpp"

// Processes

void wami_gradient3::load_input()
{
    uint32_t img_num_img  = 0;
    uint32_t img_num_col  = 0;
    uint32_t img_num_row  = 0;
    uint32_t mem_offset_2 = 0;
    uint32_t is_p2p       = 0;
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
        mem_offset_2 = config.src_dst_offset_2;
        img_num_img  = config.num_img;
        img_num_col  = config.num_col;
        img_num_row  = config.num_row;
        is_p2p       = config.is_p2p;
    }

    // Load
    for (uint32_t img = 0; img < img_num_img; img++) {

        HLS_PROTO("load-dma");

        this->load_store_handshake();

        // == BURST 1 (read first input) ==================
        index  = mem_offset_2;
        length = img_num_col * img_num_row;
        dma_info_t dma_info_1(index, length, SIZE_WORD);
        this->dma_read_ctrl.put(dma_info_1);

        for (unsigned i = 0; i < length; i++) {
            sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
            wait();

            HLS_BREAK_DEP(A0_grayscale);

            FPDATA_WORD pixel_fp;
            pixel_fp = pixel_bv.range(63, 0).to_int64();
            wait();
            A0_grayscale[i] = pixel_fp;

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

void wami_gradient3::store_output()
{
    uint32_t img_num_img     = 0;
    uint32_t img_num_col     = 0;
    uint32_t img_num_row     = 0;
    uint32_t mem_offset_0    = 0;
    uint32_t mem_offset_1    = 0;
    uint32_t is_p2p          = 0;
    uint32_t num_output_copy = 0;
    uint32_t is_dummy        = 0;
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
        img_num_img     = config.num_img;
        img_num_col     = config.num_col;
        img_num_row     = config.num_row;
        mem_offset_0    = config.src_dst_offset_0;
        mem_offset_1    = config.src_dst_offset_1;
        is_p2p          = config.is_p2p;
        num_output_copy = config.p2p_config_0;
        is_dummy        = config.p2p_config_1;
    }

    for (uint32_t img = 0; img < img_num_img; img++) {
        HLS_PROTO("store-dma");

        this->store_load_handshake();
        this->store_compute_handshake();

        if (is_p2p == 0 && is_dummy == 0) { // normal shared memory version
            // fprintf(stderr, "[store_output]: is_dummy = %d, is_p2p = %d\n", is_dummy, is_p2p);
            // fprintf(stderr, "[store_output]: mem_offset_0 = %d, mem_offset_1 = %d\n", mem_offset_0, mem_offset_1);

            // write output x
            index  = mem_offset_0;
            length = img_num_col * img_num_row;
            dma_info_t dma_info1(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info1);

            for (unsigned i = 0; i < length; i++) {

                HLS_BREAK_DEP(B0_gradient_x);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = B0_gradient_x[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }

            // write output y
            index  = mem_offset_1;
            length = img_num_col * img_num_row;
            dma_info_t dma_info2(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info2);

            for (unsigned i = 0; i < length; i++) {

                HLS_BREAK_DEP(B0_gradient_y);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = B0_gradient_y[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
        }

        if (is_p2p == 1 && is_dummy == 0) { // p2p version
            // fprintf(stderr, "[store_output]: is_dummy = %d, is_p2p = %d\n", is_dummy, is_p2p);

            // p2p enable, combine 2 outputs to 1 dma transaction and send it 2 times
            for (unsigned x = 0; x < num_output_copy; x++) {
                index  = mem_offset_0;
                length = img_num_col * img_num_row * 2;
                dma_info_t dma_info_p2p(index, length, SIZE_WORD);
                this->dma_write_ctrl.put(dma_info_p2p);

                for (unsigned i = 0; i < length / 2; i++) {

                    HLS_BREAK_DEP(B0_gradient_x);
                    FPDATA_WORD             pixel_fp;
                    sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                    wait();
                    pixel_fp = B0_gradient_x[i];

                    // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                    pixel_bv.range(63, 0) = pixel_fp;

                    wait();
                    this->dma_write_chnl.put(pixel_bv);
                }

                for (unsigned i = 0; i < length / 2; i++) {

                    HLS_BREAK_DEP(B0_gradient_y);
                    FPDATA_WORD             pixel_fp;
                    sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                    wait();
                    pixel_fp = B0_gradient_y[i];

                    // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                    pixel_bv.range(63, 0) = pixel_fp;

                    wait();
                    this->dma_write_chnl.put(pixel_bv);
                }
            }
        }

        if (is_p2p == 1 && is_dummy == 1) { // act as a dummy accelerator to take output for warp-iwxp
            // fprintf(stderr, "[store_output]: is_dummy = %d, is_p2p = %d\n", is_dummy, is_p2p);

            // write dummy output
            index  = mem_offset_0;
            length = img_num_col * img_num_row;
            dma_info_t dma_info1(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info1);

            for (unsigned i = 0; i < length; i++) {

                HLS_BREAK_DEP(A0_grayscale);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = A0_grayscale[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
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

void wami_gradient3::compute_kernel()
{
    uint32_t img_num_img = 0;
    uint32_t img_num_col = 0;
    uint32_t img_num_row = 0;
    uint32_t is_dummy    = 0;

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
        img_num_col = config.num_col;
        img_num_row = config.num_row;
        is_dummy    = config.p2p_config_1;
    }

    // Compute
    for (uint16_t n = 0; n < img_num_img; n++) {
        HLS_UNROLL_LOOP(OFF);

        this->compute_load_handshake();

        // Computing phase implementation

        if (is_dummy == 1) {
            // fprintf(stderr, "[compute_kernel]: dummy\n");
            wait();
        } else {
            // fprintf(stderr, "[compute_kernel]: do compute\n");
            gradientXY(img_num_col, img_num_row);
        }

        this->compute_store_handshake();
    }

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
