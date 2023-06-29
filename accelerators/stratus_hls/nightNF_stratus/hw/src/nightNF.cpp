// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "nightNF.hpp"
#include "nightNF_directives.hpp"

// Functions

#include "nightNF_functions.hpp"

// Processes

void nightNF::load_input()
{
    uint32_t n_Images;
    uint32_t n_Rows;
    uint32_t n_Cols;

    // Reset
    {
        HLS_PROTO("load-reset");

        this->reset_load_input();
        accel_ready.ack.reset_ack();

        // User-defined reset code
        n_Images = 0;
        n_Rows   = 0;
        n_Cols   = 0;

        wait();
    }

    // Config
    {
        HLS_PROTO("load-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        n_Images = config.n_Images;
        n_Rows   = config.n_Rows;
        n_Cols   = config.n_Cols;
    }

    // Load
    uint32_t dma_addr = 0;
    for (uint16_t a = 0; a < n_Images; a++) {
        HLS_PROTO("load-dma");

        this->load_store_handshake();

        // Configure DMA transaction
        uint32_t   index  = 0;
        uint32_t   length = n_Rows * n_Cols;
        dma_info_t dma_info_1(index, length, SIZE_WORD);
        this->dma_read_ctrl.put(dma_info_1);

        for (uint32_t i = 0; i < length; i++) {
            sc_dt::sc_bv<DMA_WIDTH> data = this->dma_read_chnl.get();
            HLS_BREAK_DEP(mem_buff_1);
            HLS_BREAK_DEP(mem_buff_2);
            wait();

            // Write to PLM
            mem_buff_1[i] = data.to_uint();
            mem_buff_2[i] = 0;

            // printf("mem_buff_1[%d]: %d\n", i, mem_buff_1[i]);
        }

        this->load_compute_handshake();
    }

    // Conclude
    {
        HLS_PROTO("load-done");
        this->process_done();
    }
}

void nightNF::store_output()
{
    uint32_t n_Images;
    uint32_t n_Rows;
    uint32_t n_Cols;
    uint32_t is_p2p;
    uint32_t num_output_copy;

    // Reset
    {
        HLS_PROTO("store-reset");

        this->reset_store_output();
        accel_ready.req.reset_req();

        // User-defined reset code
        n_Images        = 0;
        n_Rows          = 0;
        n_Cols          = 0;
        is_p2p          = 0;
        num_output_copy = 0;

        wait();
    }

    // Config
    {
        HLS_PROTO("store-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        n_Images        = config.n_Images;
        n_Rows          = config.n_Rows;
        n_Cols          = config.n_Cols;
        is_p2p          = config.is_p2p;
        num_output_copy = config.p2p_config_0;
    }

    // Store
    uint32_t dma_addr = 0;
    for (uint16_t a = 0; a < n_Images; a++) {
        HLS_PROTO("store-dma");

        this->store_load_handshake();
        this->store_compute_handshake();

        if (is_p2p == 0) { // normal shared memory version
            // Configure DMA write
            uint32_t   index  = 0;
            uint32_t   length = n_Rows * n_Cols;
            dma_info_t dma_info(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info);
            wait();

            for (uint32_t i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> data;

                wait();

                data.range(63, 0) = sc_bv<MAX_PXL_WIDTH>(mem_buff_2[i]);

                this->dma_write_chnl.put(data);
            }

        } else { // p2p version
            for (uint32_t x = 0; x < num_output_copy; x++) {
                // Configure DMA write
                uint32_t   index  = 0;
                uint32_t   length = n_Rows * n_Cols;
                dma_info_t dma_info(index, length, SIZE_WORD);
                this->dma_write_ctrl.put(dma_info);
                wait();

                for (uint32_t i = 0; i < length; i++) {
                    sc_dt::sc_bv<DMA_WIDTH> data;

                    wait();

                    data.range(63, 0) = sc_bv<MAX_PXL_WIDTH>(mem_buff_2[i]);

                    this->dma_write_chnl.put(data);
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

void nightNF::compute_kernel()
{
    uint32_t n_Images;
    uint32_t n_Rows;
    uint32_t n_Cols;
    uint32_t plm_size;
    uint32_t index_last_row;
    bool     do_dwt;

    // Reset
    {
        HLS_PROTO("compute-reset");

        this->reset_compute_kernel();

        // User-defined reset code
        n_Images = 0;
        n_Rows   = 0;
        n_Cols   = 0;
        do_dwt   = 0;

        wait();
    }

    {
        HLS_PROTO("compute-config");

        cfg.wait_for_config(); // config process
        conf_info_t config = this->conf_info.read();

        // User-defined config code
        n_Images       = config.n_Images;
        n_Rows         = config.n_Rows;
        n_Cols         = config.n_Cols;
        do_dwt         = config.do_dwt;
        plm_size       = n_Cols * n_Rows;
        index_last_row = plm_size - n_Cols;
    }

    // Compute
    for (uint16_t a = 0; a < n_Images; a++) {
        this->compute_load_handshake();

        // Computing phase implementation
        kernel_nf(n_Rows, n_Cols);
        // kernel_hist(n_Rows, n_Cols);
        // kernel_histEq(n_Rows, n_Cols);
        // if (do_dwt)
        //     kernel_dwt(n_Rows, n_Cols);

        this->compute_store_handshake();
    }

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
