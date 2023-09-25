// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_SD_UPDATE3_FUNCTIONS_HPP__
#define __WAMI_SD_UPDATE3_FUNCTIONS_HPP__

#include "wami_sd_update3.hpp"

// Compute the 1D x and y gradients of the input matrix / image

void wami_sd_update3::sd_update(uint32_t nRows, uint32_t nCols, int N_p)
{
    // N_p = 6
    // nRows = 128
    // nCols = 128

    //-- printf("   N_p = %d\n", N_p);
    //-- printf(" nRows = %d\n", nRows);
    //-- printf(" nCols = %d\n", nCols);

    /*
    FPDATA_WORD A0_steepest_descent[6 * WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS]; // 6x128x128
    FPDATA_WORD A0_subtract[WAMI_GRAYSCALE_IMG_NUM_COLS * WAMI_GRAYSCALE_IMG_NUM_ROWS];             // 128x128
    FPDATA_WORD B0_sd_update[6];
*/

    uint32_t x, y, i;

    for (x = 0; x < 6; x++) {
        B0_sd_update[x] = fp2bv<FPDATA, FPDATA_WL>(FPDATA(0.0));
        //-- printf("[compute]: debug 0 : %d\n", x);

        wait();
    }

    for (y = 0; y < nRows; y++) {
        for (i = 0; i < N_p; i++) {
            for (x = 0; x < nCols; x++) {

                FPDATA_WORD tmp1 = A0_subtract[y * nCols + x];
                FPDATA_WORD tmp2 = A0_steepest_descent[y * N_p * nCols + (x + i * nCols)];

                FPDATA temp01 = int2fp<FPDATA, FPDATA_WL>(tmp1);
                FPDATA temp02 = int2fp<FPDATA, FPDATA_WL>(tmp2);
                FPDATA tmp    = temp01 * temp02;
                FPDATA temp03 = int2fp<FPDATA, FPDATA_WL>(B0_sd_update[i]);
                wait();
                FPDATA      temp04 = temp03 + tmp;
                FPDATA_WORD tmp3   = fp2bv<FPDATA, FPDATA_WL>(temp04);

                B0_sd_update[i] = tmp3;
                wait();
            }
        }
    }
}

#endif // __WAMI_SD_UPDATE3_FUNCTIONS_HPP__
