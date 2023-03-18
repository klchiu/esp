// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __TF_SUB3_HPP__
#define __TF_SUB3_HPP__

#include "fpdata.hpp"


#include "tf_sub3_conf_info.hpp"
#include "tf_sub3_debug_info.hpp"
#include "tf_sub3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

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
        HLS_MAP_A0_in1;
        HLS_MAP_A0_in2;
        HLS_MAP_B0_out;
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
    void sub(uint32_t length);

    // -- Private local memories
    // B0_out = A0_in1 + A0_in2
    FPDATA_WORD A0_in1[16384];
    FPDATA_WORD A0_in2[16384];
    FPDATA_WORD B0_out[16384];

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
