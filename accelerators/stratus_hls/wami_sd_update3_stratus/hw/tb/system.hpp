// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __SYSTEM_HPP__
#define __SYSTEM_HPP__

#include <iostream>
#include <string>

#include "../../../wami_common/wami_conf_info.hpp"
// #include "../../../wami_common/wami_C_data.hpp"

#include "../../../wami_common/wami_config_tb.hpp"
// #include "../../../wami_common/wami_utils.hpp"

#include "wami_sd_update3_debug_info.hpp"
#include "wami_sd_update3.hpp"
#include "wami_sd_update3_directives.hpp"
#include "fpdata.hpp"

#include "esp_templates.hpp"

#define DATA_WIDTH 64
//#define MEM_SIZE   5000000 // TODO: [humu] right now it's oversized

#include "core/systems/esp_system.hpp"

#ifdef CADENCE
    #include "wami_sd_update3_wrap.h"
#endif

class system_t : public esp_system<DMA_WIDTH, MEM_SIZE>
{
  public:
    // ACC instance
#ifdef CADENCE
    wami_sd_update3_wrapper *acc;
#else
    wami_sd_update3 *acc;
#endif

    // Constructor
    SC_HAS_PROCESS(system_t);
    system_t(sc_module_name name)
        : esp_system<DMA_WIDTH, MEM_SIZE>(name)
    {
        // ACC
#ifdef CADENCE
        acc = new wami_sd_update3_wrapper("wami_sd_update3_wrapper");
#else
        acc = new wami_sd_update3("wami_sd_update3_wrapper");
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
    uint32_t num_img;
    uint32_t num_col;
    uint32_t num_row;
    uint32_t pad;
    uint32_t kern_id;
    uint32_t batch;
    uint32_t base_addr_0;
    uint32_t base_addr_1;
    uint32_t base_addr_2;
    uint32_t base_addr_3;
    uint32_t base_addr_4;
    uint32_t is_p2p;
    uint32_t p2p_config_0;
    uint32_t p2p_config_1;

    // Golden outputs
    rgb_pixel_t *golden_rgb_imgs;       // output of debayer
    flt_pixel_t *golden_gs_imgs;        // output of grayscale
    flt_pixel_t *golden_grad_x;         // output of gradient
    flt_pixel_t *golden_grad_y;         // output of gradient
    flt_pixel_t *golden_warp_img;       // output of warp-img
    flt_pixel_t *affine_warp;           // input of warp-img, warp-dx, warp-dy
    flt_pixel_t *golden_sub;            // output of sub
    flt_pixel_t *golden_warp_dx;        // output of warp-dx
    flt_pixel_t *golden_warp_dy;        // output of warp-dy
    flt_pixel_t *golden_I_steepest;     // output of steep_descent
    flt_pixel_t *golden_hess;           // output of hessian
    flt_pixel_t *golden_hess_inv;       // output of inv
    flt_pixel_t *igj_workspace;         // a temp buffer for inv
    flt_pixel_t *golden_sd_delta_p;     // output of sd_update
    flt_pixel_t *golden_delta_p;        // output of mult
    flt_pixel_t *golden_sd_delta_p_nxt; // output of reshape
    flt_pixel_t *golden_affine_warp;    // output of add
    flt_pixel_t *golden_warp_iwxp;      // output of warp-iwxp
    uint8_t *    golden_foreground;     // output of change_detection

    // Input images from bin
    uint32_t     srcRows;    // original size of the bayer image
    uint32_t     srcCols;    // original size of the bayer image
    uint32_t     padding_in; // padding of the image
    uint32_t     nRows;      // size used for computing (after getting rid of the padding)
    uint32_t     nCols;      // size used for computing (after getting rid of the padding)
    uint32_t     nModels;
    flt_pixel_t *IWxp;
    float *      mu;
    float *      sigma;
    float *      weight;
    uint32_t     nTestImages;
    uint16_t *   images; // the bayer image
    uint8_t *    results;

    // Test outputs
    rgb_pixel_t *rgb_imgs; // [humu]: original name: rgbtmpimg
    flt_pixel_t *gs_imgs;  // [humu]: original name: imgs
    flt_pixel_t *grad_x;
    flt_pixel_t *grad_y;
    flt_pixel_t *warp_iwxp;
    flt_pixel_t *sub;
    flt_pixel_t *I_steepest;
    flt_pixel_t *hess;
    flt_pixel_t *hess_inv;
    flt_pixel_t *sd_delta_p;
    flt_pixel_t *delta_p;
    flt_pixel_t *sd_delta_p_nxt;
    flt_pixel_t *affine_warp_nxt;
    uint8_t *    foreground;

    // [humu] below probably not in use?
    // uint16_t *   u16tmpimg;
    // flt_pixel_t *IWxp_; // TODO
    // flt_pixel_t *nabla_Ix;
    // flt_pixel_t *nabla_Iy;
    uint16_t *gmm_img; // for gmm
    // float *   mu_dump;
    // float *   sigma_dump;
    // float *   weight_dump;
    // flt_pixel_t *tmplt;
    // flt_pixel_t *swap;
    // float warp_p[6];

    int f;

    // Other Functions
};

#endif // __SYSTEM_HPP__
