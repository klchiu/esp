// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __NY_ACC_HPP__
#define __NY_ACC_HPP__

#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"

#include "ny_acc_debug_info.hpp"
#include "ny_acc_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class ny_acc : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;
    // handshake_t input_ready2;  // Handshake INPUT vs. COMPUTATION
    // handshake_t output_ready2; // Handshake COMPUTATION vs. OUTPUT
    // handshake_t next_image;    // Handshake INPUT vs. OUTPUT

    // Constructor
    SC_HAS_PROCESS(ny_acc);
    ny_acc(const sc_module_name &name)
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

    // Configure ny_acc
    esp_config_proc cfg;

    // Functions
    void synth(uint32_t delay);

    // -- Private local memories
    FPDATA_WORD A0B0_synth_in[16];
    FPDATA_WORD A0B0_synth_out[16];

    // -- Private state variables
};

inline void ny_acc::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void ny_acc::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __NY_ACC_HPP__ */
