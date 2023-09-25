// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <iostream>
#include <string>

#include "wami_conf_info.hpp"
// #include "../../../wami_common/wami_C_data.hpp"
#include "../../../wami_common/wami_config_tb.hpp"
// #include "../../../wami_common/wami_utils.hpp"

#include "wami_steep_descent_hessian_debug_info.hpp"
#include "wami_steep_descent_hessian.hpp"
#include "wami_steep_descent_hessian_directives.hpp"
#include "fpdata.hpp"

#include "esp_templates.hpp"

#define DATA_WIDTH 64

// TODO: [humu] right now it's oversized, don't know why there's an error during compilation
// const size_t MEM_SIZE = 5000000;

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
    #include "wami_steep_descent_hessian_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, 5000000>
{
  public:
    // ACC instance
#ifdef CADENCE
    wami_steep_descent_hessian_wrapper *acc;
#else
    wami_steep_descent_hessian *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, 5000000>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new wami_steep_descent_hessian_wrapper("wami_steep_descent_hessian_wrapper");
#else
        acc = new wami_steep_descent_hessian("wami_steep_descent_hessian_wrapper");
#endif
        // Binding ACC
        acc->clk(clk);
        acc->rst(acc_rst);
        acc->dma_read_ctrl(dma_read_ctrl);
        acc->dma_write_ctrl(dma_write_ctrl);
        acc->dma_read_chnl(dma_read_chnl);
        acc->dma_write_chnl(dma_write_chnl);
        acc->conf_info(conf_info);
        acc->conf_done(conf_done);
        acc->acc_done(acc_done);
        acc->debug(debug);

        nRows = 0;
        nCols = 0;
    }

    // Processes

    // Configure accelerator
    void config_proc();

    // Load internal memory
    void load_memory();

    // Dump internal memory
    void dump_memory();

    // Validate accelerator results
    int validate();

    // Optionally free resources (arrays)
    void read_debayer_input();
    void malloc_arrays();
    void free_arrays();

    void run_pv();

    // Accelerator-specific data
    uint32_t base_addr;
    uint32_t base_addr_1;
    uint32_t base_addr_2;
    uint32_t base_addr_3;
    uint32_t base_addr_4;

    // Gold images
    rgb_pixel_t *golden_rgb_imgs;
    flt_pixel_t *golden_gs_imgs;
    flt_pixel_t *golden_grad_x;
    flt_pixel_t *golden_grad_y;
    flt_pixel_t *golden_warp_img;
    flt_pixel_t *affine_warp;
    flt_pixel_t *golden_sub;
    flt_pixel_t *golden_warp_dx;
    flt_pixel_t *golden_warp_dy;
    flt_pixel_t *golden_warp_iwxp;
    flt_pixel_t *golden_I_steepest;
    flt_pixel_t *golden_hess;
    flt_pixel_t *golden_hess_inv;
    flt_pixel_t *igj_workspace;
    flt_pixel_t *golden_delta_p;
    flt_pixel_t *golden_sd_delta_p;
    flt_pixel_t *golden_sd_delta_p_nxt;
    flt_pixel_t *golden_affine_warp;

    // Input images
    uint32_t     nTestImages;
    uint16_t *   u16tmpimg;
    uint16_t *   images;
    uint8_t *    results;
    rgb_pixel_t *rgbtmpimg;
    uint32_t     nRows; // original size of the bayer images
    uint32_t     nCols; // original size of the bayer images
    uint32_t     M;
    uint32_t     N;

    // Warp and registration - k2 lucas kanade
    flt_pixel_t *imgs;
    flt_pixel_t *grad_x;
    flt_pixel_t *grad_y;
    flt_pixel_t *IWxp_; // TODO
    flt_pixel_t *sub;
    flt_pixel_t *nabla_Ix;
    flt_pixel_t *nabla_Iy;
    flt_pixel_t *I_steepest;
    flt_pixel_t *hess;
    flt_pixel_t *hess_inv;
    flt_pixel_t *sd_delta_p;
    flt_pixel_t *delta_p;
    flt_pixel_t *sd_delta_p_nxt;
    flt_pixel_t *affine_warp_nxt;
    flt_pixel_t *warp_iwxp;

    flt_pixel_t *tmplt;
    flt_pixel_t *IWxp;
    flt_pixel_t *swap;

    int      i, a, f, errors;
    uint32_t nModels, padding_in;

    // GMM storage
    uint16_t *gmm_img;
    float *   mu, *mu_dump;
    float *   sigma, *sigma_dump;
    float *   weight, *weight_dump;
    uint8_t * foreground;

    // Affine warp parameter set p
    float warp_p[6];

    // Other Functions
};

#endif // __SYSTEM_HPP__
