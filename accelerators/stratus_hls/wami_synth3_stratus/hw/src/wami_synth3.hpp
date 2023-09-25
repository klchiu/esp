// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_SYNTH3_HPP__
#define __WAMI_SYNTH3_HPP__

#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"

#include "wami_synth3_debug_info.hpp"
#include "wami_synth3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class wami_synth3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;
    // handshake_t input_ready2;  // Handshake INPUT vs. COMPUTATION
    // handshake_t output_ready2; // Handshake COMPUTATION vs. OUTPUT
    // handshake_t next_image;    // Handshake INPUT vs. OUTPUT

    // Constructor
    SC_HAS_PROCESS(wami_synth3);
    wami_synth3(const sc_module_name &name)
        : esp_accelerator_3P<DMA_WIDTH>(name)
        , cfg("config")
        , accel_ready("accel_ready")
    // , input_ready2("input_ready2")
    // , output_ready2("output_ready2")
    // , next_image("next_image")
    {
        // SC_CTHREAD(config_kernel, clk.pos());
        // reset_signal_is(rst, false);

        // Signal binding
        cfg.bind_with(*this);
        accel_ready.bind_with(*this);
        // input_ready2.bind_with(*this);
        // output_ready2.bind_with(*this);
        // next_image.bind_with(*this);

        // Clock binding for memories
        HLS_MAP_A0B0_synth_in;
        HLS_MAP_A0B0_synth_out;
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

    // Configure wami_synth3
    esp_config_proc cfg;

    // Functions
    void synth(uint32_t delay_A, uint32_t delay_B);

    // -- Private local memories
    FPDATA_WORD A0B0_synth_in[65536];
    FPDATA_WORD A0B0_synth_out[65536];

    // -- Private state variables
};

inline void wami_synth3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_synth3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_SYNTH3_HPP__ */
