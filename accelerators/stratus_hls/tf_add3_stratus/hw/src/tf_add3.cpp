// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#include "tf_add3.hpp"
#include "tf_add3_directives.hpp"


// Functions

#include "tf_add3_functions.hpp"

// Processes

void tf_add3::load_input()
{
    uint32_t mem_length = 0;
    uint32_t mem_src_dst_offset_1 = 0;
    uint32_t mem_src_dst_offset_2 = 0;
    uint32_t index, length;
    uint32_t chunk_size;
    bool pingpong = true;

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
        mem_length = config.length;
        mem_src_dst_offset_1 = config.src_dst_offset_1;
        mem_src_dst_offset_2 = config.src_dst_offset_2;
        chunk_size = MAX_CHUNK_SIZE > config.chunk_size ? config.chunk_size : MAX_CHUNK_SIZE;
    }

    // Load


        HLS_PROTO("load-dma");

        // this->load_store_handshake();

#ifndef STRATUS_HLS
        // Print information about begin time
        sc_time load_begin_time = sc_time_stamp();
        ESP_REPORT_TIME(load_begin_time, "LOAD BEGIN - tf_add3");
#endif

        uint32_t remainder = mem_length;

        for(uint32_t c = 0; c < mem_length; c+=chunk_size){

            //this->load_store_handshake();

            // == BURST (read first input) ==
            index = mem_src_dst_offset_1 + c;
            length = remainder < chunk_size ? remainder : chunk_size;

            // remainder = remainder < CHUNK_SIZE ? 0 : remainder - CHUNK_SIZE;

            dma_info_t dma_info_1(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_1);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                // HLS_BREAK_DEP(A0_in1_ping);
                // HLS_BREAK_DEP(A0_in1_pong);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.range(63, 0).to_int64();

                if(pingpong == true)
                    A0_in1_ping[i] = pixel_fp;
                else
                    A0_in1_pong[i] = pixel_fp;
                wait();
            }

            // == BURST (read second input) ==
            index  = mem_src_dst_offset_2 + c;
            length = remainder < chunk_size ? remainder : chunk_size;

            remainder = remainder < chunk_size ? 0 : remainder - chunk_size;
            dma_info_t dma_info_2(index, length, SIZE_WORD);
            this->dma_read_ctrl.put(dma_info_2);

            for (unsigned i = 0; i < length; i++) {
                sc_dt::sc_bv<DMA_WIDTH> pixel_bv = this->dma_read_chnl.get();
                wait();

                // HLS_BREAK_DEP(A0_in2_ping);
                // HLS_BREAK_DEP(A0_in2_pong);

                FPDATA_WORD pixel_fp;
                pixel_fp = pixel_bv.to_int64();

                if(pingpong == true)
                    A0_in2_ping[i] = pixel_fp;
                else
                    A0_in2_pong[i] = pixel_fp;
                wait();
            }

            pingpong = !pingpong;

            this->load_compute_handshake();

        }

#ifndef STRATUS_HLS
            // Print information about begin time
            sc_time load_end_time = sc_time_stamp();
            ESP_REPORT_TIME(load_end_time, "LOAD END - tf_add3");
#endif

        // this->load_compute_handshake();


    // Conclude
    {
        HLS_PROTO("load-done");
        this->process_done();
    }
}

void tf_add3::store_output()
{
    uint32_t mem_length = 0;
    uint32_t mem_src_dst_offset_0 = 0;
    uint32_t index, length;
    uint32_t chunk_size;
    bool pingpong = true;

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
        mem_length = config.length;
        mem_src_dst_offset_0 = config.src_dst_offset_0;
        chunk_size = MAX_CHUNK_SIZE > config.chunk_size ? config.chunk_size : MAX_CHUNK_SIZE;
    }

        HLS_PROTO("store-dma");

        // this->store_load_handshake();
        // this->store_compute_handshake();

#ifndef STRATUS_HLS
        // Print information about begin time
        sc_time store_begin_time = sc_time_stamp();
        ESP_REPORT_TIME(store_begin_time, "STORE BEGIN - tf_add3");
#endif

        uint32_t remainder = mem_length;

        for(uint32_t c = 0; c < mem_length; c+=chunk_size){

            //this->store_load_handshake();
            this->store_compute_handshake();

            // Configure DMA write
            index = mem_src_dst_offset_0 + c;
            length = remainder < chunk_size ? remainder : chunk_size;

            remainder = remainder < chunk_size ? 0 : remainder - chunk_size;

            dma_info_t dma_info(index, length, SIZE_WORD);
            this->dma_write_ctrl.put(dma_info);

            for (unsigned i = 0; i < length; i++) {

                // HLS_BREAK_DEP(B0_out_ping);
                // HLS_BREAK_DEP(B0_out_pong);

                FPDATA_WORD pixel_fp;
                sc_dt::sc_bv<WORD_SIZE> pixel_bv;

                wait();

                if(pingpong == true)
                    pixel_fp = B0_out_ping[i];
                else
                    pixel_fp = B0_out_pong[i];

                // pixel_bv = fp2bv<FPDATA, WORD_SIZE>(pixel_fp);
                pixel_bv.range(63, 0) = pixel_fp;

                wait();
                this->dma_write_chnl.put(pixel_bv);
            }

            pingpong = !pingpong;

        }

#ifndef STRATUS_HLS
        // Print information about begin time
        sc_time store_end_time = sc_time_stamp();
        ESP_REPORT_TIME(store_end_time, "STORE END - tf_add3");
#endif

    // Conclude
    {
        HLS_PROTO("store-done");
        this->accelerator_done();
        this->process_done();
    }
}

void tf_add3::compute_kernel()
{
    uint32_t mem_length = 0;
    // uint32_t index, length;
    uint32_t length;
    uint32_t chunk_size;
    bool pingpong = true;

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
        mem_length = config.length;
        chunk_size = MAX_CHUNK_SIZE > config.chunk_size ? config.chunk_size : MAX_CHUNK_SIZE;
    }

    // Compute
        // HLS_UNROLL_LOOP(OFF);

        // this->compute_load_handshake();

#ifndef STRATUS_HLS
        // Print information about begin time
        sc_time compute_begin_time = sc_time_stamp();
        ESP_REPORT_TIME(compute_begin_time, "COMPUTE BEGIN - tf_add3");
#endif

        uint32_t remainder = mem_length;

        for(uint32_t c = 0; c < mem_length; c+=chunk_size){

            this->compute_load_handshake();

            // Configure DMA write
            //index = c;
            length = remainder < chunk_size ? remainder : chunk_size;

            remainder = remainder < chunk_size ? 0 : remainder - chunk_size;

            // Computing phase implementation
            add(length, pingpong);
            pingpong = !pingpong;

            this->compute_store_handshake();

        }

#ifndef STRATUS_HLS
        // Print information about begin time
        sc_time compute_end_time = sc_time_stamp();
        ESP_REPORT_TIME(compute_end_time, "COMPUTE END - tf_add3");
#endif

        // this->compute_store_handshake();

    // Conclude
    {
        HLS_PROTO("compute-done");
        this->process_done();
    }
}
