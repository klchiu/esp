// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// // SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_DEBAYER3_HPP__
#define __WAMI_DEBAYER3_HPP__

#include "fpdata.hpp"
#include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config.hpp"
#include "../../../wami_common/wami_utils.hpp"
#include "../../../wami_common/wami_conf_info.hpp"

#include "wami_debayer3_debug_info.hpp"
#include "wami_debayer3_directives.hpp"

#include "esp_templates.hpp"

#include "utils/esp_handshake.hpp"

class wami_debayer3 : public esp_accelerator_3P<DMA_WIDTH>
{
  public:
    // Output <-> Input
    handshake_t accel_ready;

    // Constructor
    SC_HAS_PROCESS(wami_debayer3);
    wami_debayer3(const sc_module_name &name)
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
        HLS_MAP_A0_bayer;
        HLS_MAP_B0_debayer;
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


    // Configure wami_debayer3
    esp_config_proc cfg;

    // Functions

    uint16_t __compute_and_clamp_pixel_fractional_neg(uint16_t pos, uint16_t neg);
    uint16_t __interp_G_at_RRR_or_G_at_BBB(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols);
    uint16_t __interp_R_at_GRB_or_B_at_GBR(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols);
    uint16_t __interp_R_at_GBR_or_B_at_GRB(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols);
    uint16_t __interp_R_at_BBB_or_B_at_RRR(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols);

    void wami_debayer(uint32_t nrows, uint32_t ncols);

    // -- Private local memories
    uint16_t    A0_bayer[WAMI_DEBAYER_IMG_NUM_ROWS * WAMI_DEBAYER_IMG_NUM_COLS];       // 132x132
    rgb_pixel_t B0_debayer[WAMI_GRAYSCALE_IMG_NUM_ROWS * WAMI_GRAYSCALE_IMG_NUM_COLS]; // 128x128

    // -- Private state variables
};

inline void wami_debayer3::store_load_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("store-load-handshake");

        accel_ready.req.req();
    }
}

inline void wami_debayer3::load_store_handshake()
{
    {
        HLS_DEFINE_PROTOCOL("load-store-handshake");

        accel_ready.ack.ack();
    }
}
#endif /* __WAMI_DEBAYER3_HPP__ */
