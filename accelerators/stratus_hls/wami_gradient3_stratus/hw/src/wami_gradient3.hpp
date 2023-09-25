// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_GRADIENT3_HPP__
#define __WAMI_GRADIENT3_HPP__

#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"

#include "wami_gradient3_debug_info.hpp"
#include "wami_gradient3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class wami_gradient3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;

    // Constructor
    SC_HAS_PROCESS(wami_gradient3);
    wami_gradient3(const sc_module_name &name)
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
        HLS_MAP_A0_grayscale;
        HLS_MAP_B0_gradient_x;
        HLS_MAP_B0_gradient_y;
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


    // Configure wami_gradient3
    esp_config_proc cfg;

    // Functions
    void gradientXY(uint32_t nCols, uint32_t nRows);

    // -- Private local memories
    FPDATA_WORD A0_grayscale[WAMI_GRADIENT_IMG_NUM_COLS * WAMI_GRADIENT_IMG_NUM_ROWS];  // 128*128
    FPDATA_WORD B0_gradient_x[WAMI_GRADIENT_IMG_NUM_COLS * WAMI_GRADIENT_IMG_NUM_ROWS]; // 128*128
    FPDATA_WORD B0_gradient_y[WAMI_GRADIENT_IMG_NUM_COLS * WAMI_GRADIENT_IMG_NUM_ROWS]; // 128*128

    // -- Private state variables
};

inline void wami_gradient3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_gradient3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_ADD3_HPP__ */
