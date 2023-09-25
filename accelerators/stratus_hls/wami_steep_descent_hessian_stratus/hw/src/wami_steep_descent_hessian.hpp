// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_STEEP_DESCENT_HESSIAN_HPP__
#define __WAMI_STEEP_DESCENT_HESSIAN_HPP__

// #include "wami_steep_descent_hessian_data.hpp"
#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"

#include "wami_conf_info.hpp"
#include "wami_steep_descent_hessian_debug_info.hpp"

#include "esp_templates.hpp"

#include "wami_steep_descent_hessian_directives.hpp"

#include "utils/esp_handshake.hpp"

class wami_steep_descent_hessian : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;
    // handshake_t input_ready2;  // Handshake INPUT vs. COMPUTATION
    // handshake_t output_ready2; // Handshake COMPUTATION vs. OUTPUT
    // handshake_t next_image;    // Handshake INPUT vs. OUTPUT

    // Constructor
    SC_HAS_PROCESS(wami_steep_descent_hessian);
    wami_steep_descent_hessian(const sc_module_name &name)
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
        HLS_MAP_A0_in1;
        HLS_MAP_A0_in2;
        HLS_MAP_B0_steep_descent;

        HLS_MAP_A0_steepest_descent;
        HLS_MAP_B0_hess;
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

    // Configure wami_steep_descent_hessian
    esp_config_proc cfg;

    // Functions
    void sd_descent(uint32_t nRows, uint32_t nCols);
    void hess(uint32_t nCols, uint32_t nRows, uint32_t np);

    // -- Private local memories
    FPDATA_WORD A0_in1[WAMI_GRAYSCALE_IMG_NUM_ROWS * WAMI_SUBTRACT_IMG_NUM_COLS];               // 128x128
    FPDATA_WORD A0_in2[WAMI_GRAYSCALE_IMG_NUM_ROWS * WAMI_SUBTRACT_IMG_NUM_COLS];               // 128x128
    FPDATA_WORD B0_steep_descent[6 * WAMI_GRAYSCALE_IMG_NUM_ROWS * WAMI_SUBTRACT_IMG_NUM_COLS]; // 6x128x128

    FPDATA_WORD A0_steepest_descent[6 * WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];
    FPDATA_WORD B0_hess[36];

    // -- Private state variables
};

inline void wami_steep_descent_hessian::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_steep_descent_hessian::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_STEEP_DESCENT_HESSIAN_HPP__ */
