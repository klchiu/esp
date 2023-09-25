// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_GRAYSCALE3_FUNCTIONS_HPP__
#define __WAMI_GRAYSCALE3_FUNCTIONS_HPP__

#include "wami_grayscale3.hpp"

void wami_grayscale3::kernel_rgb_to_grayscale(uint32_t n_rows, uint32_t n_cols)
{
    for (uint32_t r = 0; r < n_rows; r++) {
        for (uint32_t c = 0; c < n_cols; c++) {
            uint32_t index = r * n_cols + c;

            uint64_t red = plm_A0_debayer[index].r;
            wait();
            uint64_t green = plm_A0_debayer[index].g;
            wait();
            uint64_t blue = plm_A0_debayer[index].b;
            wait();

            FPDATA grayv = FPDATA(0.0);
            grayv += FPDATA(red) * FPDATA(0.2126);
            grayv += FPDATA(green) * FPDATA(0.7152);
            grayv += FPDATA(blue) * FPDATA(0.0722);
            plm_B0_grayscale[index] = fp2bv<FPDATA, WORD_SIZE>(grayv);
        }
    }
}

#endif // __WAMI_GRAYSCALE3_FUNCTIONS_HPP__
