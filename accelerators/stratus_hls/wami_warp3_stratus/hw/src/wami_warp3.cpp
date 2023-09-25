// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "wami_warp3.hpp"
#include "wami_warp3_directives.hpp"


// Functions

#include "wami_warp3_functions.hpp"

// Processes

void wami_warp3::load_input()
{
    uint32_t img_num_img          = 0;
    uint32_t img_num_row          = 0;
    uint32_t img_num_col          = 0;
    uint32_t mem_src_dst_offset_1 = 0;
    uint32_t mem_src_dst_offset_2 = 0;
    uint32_t is_p2p               = 0;
    uint32_t warp_no              = 0;
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
        img_num_row          = config.num_row;
        img_num_col          = config.num_col;
        mem_src_dst_offset_1 = config.src_dst_offset_1;
        mem_src_dst_offset_2 = config.src_dst_offset_2;
        is_p2p               = config.is_p2p;
        warp_no              = config.p2p_config_0;
    }

    // Load
    if (is_p2p == 0) { // normal shared memory version

        for (uint32_t img = 0; img < img_num_img; img++) {

            HLS_PROTO("load-dma");

            this->load_store_handshake();

            // == BURST 2 (read input array) ====================================================
            index  = mem_src_dst_offset_2;
            length = 6;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_W_xp);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_W_xp[i] = pixel_fp;
                wait();
            }

            // == BURST 1 (read input image) ====================================================
            index  = mem_src_dst_offset_1;
            length = img_num_row * img_num_col;
            dma_info_t dma_info_1(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_1);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_img);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_img[i] = pixel_fp;
                wait();
            }

            this->load_compute_handshake();
        }
    }
    if (is_p2p == 1 && (warp_no == 0 || warp_no == 3)) {
        // p2p version, warp-img and warp-iwxp (basically the same as shared mem)
        for (uint32_t img = 0; img < img_num_img; img++) {

            HLS_PROTO("load-dma-p2p");

            this->load_store_handshake();

            // == BURST 2 (read input array) ====================================================
            index  = mem_src_dst_offset_2;
            length = 6;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_W_xp);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_W_xp[i] = pixel_fp;
                wait();
            }

            // == BURST 1 (read input image) ====================================================
            index  = mem_src_dst_offset_1;
            length = img_num_row * img_num_col;
            dma_info_t dma_info_1(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_1);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_img);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_img[i] = pixel_fp;
                wait();
            }

            this->load_compute_handshake();
        }
    }
    if (is_p2p == 1 && (warp_no == 1 || warp_no == 2)) { // p2p version, warp-dx and warp-dy
        // need to discard some input
        // here is_dx_dy should be 21 or 22 (warp-x or warp-y)
        for (uint32_t img = 0; img < img_num_img; img++) {
            HLS_PROTO("load-dma-p2p-dx-dy");

            this->load_store_handshake();

            // == BURST 2 (read input array) ====================================================
            index  = mem_src_dst_offset_2;
            length = 6;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                HLS_BREAK_DEP(A0_W_xp);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                A0_W_xp[i] = pixel_fp;
                wait();
            }

            // == BURST 1 (read input image) ====================================================
            index  = mem_src_dst_offset_1;
            length = img_num_row * img_num_col * 2;
            dma_info_t dma_info_1_p2p(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_1_p2p);

            if (warp_no == 1) { // warp-dx, take the first part
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();

                    HLS_BREAK_DEP(A0_img);

                    FPDATA_WORD pixel_fp;
                    pixel_fp = pixel_bv.to_int64();

                    A0_img[i] = pixel_fp;
                    wait();
                }
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();
                }
            }

            if (warp_no == 2) { // warp-dx, take the second part
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();
                }
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();

                    HLS_BREAK_DEP(A0_img);

                    FPDATA_WORD pixel_fp;
                    pixel_fp = pixel_bv.to_int64();

                    A0_img[i] = pixel_fp;
                    wait();
                }
            }

            this->load_compute_handshake();
        }
    }
    if (is_p2p == 2 && (warp_no == 1 || warp_no == 2)) { // p2p version, warp-dx and warp-dy
        // need to discard some input
        // here is_dx_dy should be 21 or 22 (warp-x or warp-y)
        for (uint32_t img = 0; img < img_num_img; img++) {
            HLS_PROTO("load-dma-p2p-dx-dy");

            this->load_store_handshake();

            // == BURST (read input array) ==

            for (unsigned i = 0; i < length; i++) {

                A0_W_xp[i] = 0;
                wait();
            }

            // == BURST (read input image) ==
            index  = mem_src_dst_offset_1;
            length = img_num_row * img_num_col * 2;
            dma_info_t dma_info_1_p2p(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_1_p2p);

            if (warp_no == 1) { // warp-dx, take the first part
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();

                    HLS_BREAK_DEP(A0_img);

                    FPDATA_WORD pixel_fp;
                    pixel_fp = pixel_bv.to_int64();

                    A0_img[i] = pixel_fp;
                    wait();
                }
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();
                }
            }
            if (warp_no == 2) { // warp-dx, take the second part
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();
                }
                for (unsigned i = 0; i < length / 2; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                    wait();

                    HLS_BREAK_DEP(A0_img);

                    FPDATA_WORD pixel_fp;
                    pixel_fp = pixel_bv.to_int64();

                    A0_img[i] = pixel_fp;
                    wait();
                }
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

void wami_warp3::store_output()
{
    uint32_t mem_src_dst_offset_0 = 0;
    uint32_t img_num_row          = 0;
    uint32_t img_num_col          = 0;
    uint32_t img_num_img          = 0;
    uint32_t is_p2p               = 0;
    uint32_t iwxp_temp            = 0;
    uint32_t warp_no              = 0;
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
        mem_src_dst_offset_0 = config.src_dst_offset_0;
        img_num_img          = config.num_img;
        img_num_row          = config.num_row;
        img_num_col          = config.num_col;
        is_p2p               = config.is_p2p;
        warp_no              = config.p2p_config_0;
        iwxp_temp            = config.p2p_config_1;
        num_forward_pass     = (iwxp_temp > 16) & 0xFFFF;
        num_backward_pass    = iwxp_temp & 0xFFFF;
    }

    if (is_p2p == 0) { // normal shared memory version
        for (uint32_t img = 0; img < img_num_img; img++) {
            HLS_PROTO("store-dma");

            this->store_load_handshake();
            this->store_compute_handshake();

            // Configure DMA write
            index  = mem_src_dst_offset_0;
            length = img_num_row * img_num_col;
            dma_info_t dma_info(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info);

            for (unsigned i = 0; i < length; i++) {

                HLS_BREAK_DEP(B0_img);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = B0_img[i];

                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
        }
    }

    if (is_p2p == 1 && (warp_no == 0 || warp_no == 1 || warp_no == 2)) {
        for (uint32_t img = 0; img < img_num_img; img++) {
            HLS_PROTO("store-dma-p2p-warp012");

            this->store_load_handshake();
            this->store_compute_handshake();

            // Configure DMA write
            index  = mem_src_dst_offset_0;
            length = img_num_row * img_num_col;
            dma_info_t dma_info(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info);

            for (unsigned i = 0; i < length; i++) {

                HLS_BREAK_DEP(B0_img);
                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = B0_img[i];

                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
        }
    }

    if (is_p2p == 1 && warp_no == 3) {

        for (unsigned x = 0; x < num_backward_pass; x++) {
            HLS_PROTO("store-dma-p2p-iwxp-0");
            // Configure DMA write
            index  = mem_src_dst_offset_0;
            length = img_num_row * img_num_col;
            dma_info_t dma_info(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info);

            for (unsigned i = 0; i < length; i++) {

                FPDATA_WORD             pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();
                pixel_fp = 0;

                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }
        }

        for (uint32_t img = 0; img < img_num_img - 1; img++) {
            HLS_PROTO("store-dma-p2p-iwxp-1");

            this->store_load_handshake();
            this->store_compute_handshake();

            // p2p enable, send the output num_output_copy times
            for (unsigned x = 0; x < num_backward_pass + num_forward_pass; x++) {
                // Configure DMA write
                index  = mem_src_dst_offset_0;
                length = img_num_row * img_num_col;
                dma_info_t dma_info_p2p(index, length, SIZE_WORD);
                this->dma_write_ctrl.put(dma_info_p2p);

                for (unsigned i = 0; i < length; i++) {

                    HLS_BREAK_DEP(B0_img);
                    FPDATA_WORD             pixel_fp;
                    sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                    wait();
                    pixel_fp = B0_img[i];

                    pixel_bv.range(63, 0) = pixel_fp;

                    wait();
                    this->dma_write_chnl.put(pixel_bv);
                }
            }
        }

        {
            for (unsigned x = 0; x < num_forward_pass; x++) {
                HLS_PROTO("store-dma-p2p-iwxp-2");
                this->store_load_handshake();
                this->store_compute_handshake();

                // Configure DMA write
                index  = mem_src_dst_offset_0;
                length = img_num_row * img_num_col;
                dma_info_t dma_info_p2p(index, length, SIZE_WORD);
                this->dma_write_ctrl.put(dma_info_p2p);

                for (unsigned i = 0; i < length; i++) {

                    HLS_BREAK_DEP(B0_img);
                    FPDATA_WORD             pixel_fp;
                    sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                    wait();
                    pixel_fp = B0_img[i];

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

void wami_warp3::compute_kernel()
{
    uint32_t img_num_img = 0;
    uint32_t img_num_row = 0;
    uint32_t img_num_col = 0;

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
        img_num_row = config.num_row;
        img_num_col = config.num_col;
    }

    // Compute
    for (uint16_t n = 0; n < img_num_img; n++) {
        HLS_UNROLL_LOOP(OFF);

        this->compute_load_handshake();

        // Computing phase implementation
        warp_image(img_num_col, img_num_row);

        this->compute_store_handshake();
    }

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
