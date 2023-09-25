// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_RESHAPE_ADD_HPP__
#define __WAMI_RESHAPE_ADD_HPP__

// #include "wami_reshape_add_data.hpp"
#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"

#include "wami_conf_info.hpp"
#include "wami_reshape_add_debug_info.hpp"

#include "esp_templates.hpp"

#include "wami_reshape_add_directives.hpp"

#include "utils/esp_handshake.hpp"

class wami_reshape_add : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;
    // handshake_t input_ready2;  // Handshake INPUT vs. COMPUTATION
    // handshake_t output_ready2; // Handshake COMPUTATION vs. OUTPUT
    // handshake_t next_image;    // Handshake INPUT vs. OUTPUT

    // Constructor
    SC_HAS_PROCESS(wami_reshape_add);
    wami_reshape_add(const sc_module_name &name)
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
        HLS_MAP_A0_delta_p;
        HLS_MAP_B0_sd_delta_p;

        HLS_MAP_A0_affine_params;
        HLS_MAP_A0_sd_delta_p;
        HLS_MAP_B0_affine_params;
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

#ifdef BASELINE
        // This has been introduced for the ASPDAC paper. It virtually disables the
        // parallelism between input/output and computation processes.
        //
        // input <-> computation <-> output
        //   ^--------------------------^    <- Introduce this handshaking
        //
        // handshake_t next_row;
#endif

    // Configure wami_reshape_add
    esp_config_proc cfg;

    // Functions
    void kernel_reshape(uint32_t nRows, uint32_t nCols, uint32_t newRows, uint32_t newCols);

    void addition(uint32_t nRows, uint32_t nCols);

    // -- Private local memories
    // reshape: A0_delta_p --> A0_sd_delta_p
    // add:     A0_sd_delta_p + A0_affine_params --> B0_affine_params
    FPDATA_WORD A0_delta_p[6];
    FPDATA_WORD B0_sd_delta_p[6];

    FPDATA_WORD A0_affine_params[6];
    FPDATA_WORD A0_sd_delta_p[6];
    FPDATA_WORD B0_affine_params[6];

    // -- Private state variables
};

inline void wami_reshape_add::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_reshape_add::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_RESHAPE_ADD_HPP__ */
