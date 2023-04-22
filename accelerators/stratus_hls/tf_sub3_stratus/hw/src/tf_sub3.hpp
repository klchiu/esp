// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __TF_SUB3_HPP__
#define __TF_SUB3_HPP__


#include "tf_sub3_conf_info.hpp"
#include "tf_sub3_debug_info.hpp"
#include "fpdata.hpp"
#include "tf_sub3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

#define __round_mask(x, y) ((y)-1)
#define round_up(x, y) ((((x)-1) | __round_mask(x, y))+1)

#define PLM_ONLY 1 //is this a PLM only implementaion
#define PLM_USED 1 //if it is not PLM only then this indicates if PLM is used at all

#define IN_MAX 16384
#define OUT_MAX 16384

//Data defines
#define DATA_WIDTH 32
#if (DATA_WIDTH == 32)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 2
#define LOG_DMA_WORD_PER_BEAT 1
#define PLM_A_IN_NAME "plm_a_full32"
#define PLM_B_OUT_NAME "plm_b_full32"
#elif (DATA_WIDTH == 64)
#define DMA_BEAT_PER_WORD 1
#define DMA_WORD_PER_BEAT 1
#define LOG_DMA_WORD_PER_BEAT 0
#define PLM_A_IN_NAME "plm_a_full"
#define PLM_B_OUT_NAME "plm_b_full"
#endif

//Compute defines
#if (PLM_ONLY == 0)
#define UNROLL_IN 32
#define UNROLL_PLM 16
#endif
#if (PLM_ONLY == 1)
#define UNROLL_IN 16
#define UNROLL_PLM_LOG 4
#endif

#if (PLM_USED == 0)
#define MAX_CHUNK_SIZE UNROLL_IN
#endif
#if (PLM_USED == 1)
#define MAX_CHUNK_SIZE IN_MAX
#endif


class tf_sub3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;

    // Constructor
    SC_HAS_PROCESS(tf_sub3);
    tf_sub3(const sc_module_name &name)
        : esp_accelerator_3P<DMA_WIDTH>(name)
        , cfg("config")
        , accel_ready("accel_ready")
    {
        // SC_CTHREAD(config_kernel, clk.pos());
        // reset_signal_is(rst, false);

        // Signal binding
        cfg.bind_with(*this);
        accel_ready.bind_with(*this);

        // Clock binding for memories
        // HLS_MAP_A0_in_chunk(A0_in1_ping);
        // HLS_MAP_A0_in_chunk(A0_in1_pong);
        // HLS_MAP_A0_in_chunk(A0_in2_ping);
        // HLS_MAP_A0_in_chunk(A0_in2_pong);
        // HLS_MAP_B0_out_chunk(B0_out_ping);
        // HLS_MAP_B0_out_chunk(B0_out_pong);

#if (PLM_USED == 1)
        HLS_MAP_A0_in_full(A0_in1_ping);
        HLS_MAP_A0_in_full(A0_in1_pong);
        HLS_MAP_A0_in_full(A0_in2_ping);
        HLS_MAP_A0_in_full(A0_in2_pong);
        HLS_MAP_B0_out_full(B0_out_ping);
        HLS_MAP_B0_out_full(B0_out_pong);
#endif

#if (PLM_ONLY == 0)
#if (PLM_USED == 0) //checking if this is just a registers implementation
        HLS_FLAT(A0_in1_ping);
        HLS_FLAT(A0_in1_pong);
        HLS_FLAT(A0_in2_ping);
        HLS_FLAT(A0_in2_pong);
        HLS_FLAT(B0_out_ping);
        HLS_FLAT(B0_out_pong);
#endif
#if (PLM_USED == 1) //checking it's not just a registers implementation
        HLS_FLAT(A0_inter_in1);
        HLS_FLAT(A0_inter_in2);
        HLS_FLAT(B0_inter_out);
#endif
#endif
    }

    // Processes

    // void config_kernel();  // configuration
    void load_input();     // load input
    void compute_kernel(); // iterative Debayer
    void store_output();   // store output

    // Additional handshakes
    inline void store_load_handshake();
    inline void load_store_handshake();

    // Internal synchronization signals
    sc_signal<bool> init_done;


    // Configure tf_sub3
    esp_config_proc cfg;

    // Functions
    void add(uint32_t length, bool pingpong);

    // -- Private local memories
    // B0_out = A0_in1 + A0_in2
    FPDATA_WORD A0_in1_ping[MAX_CHUNK_SIZE];
    FPDATA_WORD A0_in2_ping[MAX_CHUNK_SIZE];
    FPDATA_WORD A0_in1_pong[MAX_CHUNK_SIZE];
    FPDATA_WORD A0_in2_pong[MAX_CHUNK_SIZE];
    FPDATA_WORD B0_out_ping[MAX_CHUNK_SIZE];
    FPDATA_WORD B0_out_pong[MAX_CHUNK_SIZE];

#if (PLM_ONLY == 0)
#if (PLM_USED == 1) //checking it's not just a registers implementation
    FPDATA_WORD A0_inter_in1[UNROLL_IN];
    FPDATA_WORD A0_inter_in2[UNROLL_IN];
    FPDATA_WORD B0_inter_out[UNROLL_IN];
#endif
#endif
    // -- Private state variables
};

inline void tf_sub3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void tf_sub3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __TF_SUB3_HPP__ */
