// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_GRADIENT3_FUNCTIONS_HPP__
#define __WAMI_GRADIENT3_FUNCTIONS_HPP__

#include "wami_gradient3.hpp"

// Optional application-specific helper functions

// Compute the 1D x and y gradients of the input matrix / image

void wami_gradient3::gradientXY(uint32_t nCols, uint32_t nRows)
{
    int    x, y;
    FPDATA fp_x_1;
    FPDATA fp_x_2;
    FPDATA fp_x;

    FPDATA fp_y_1;
    FPDATA fp_y_2;
    FPDATA fp_y;
    // NOTE this could be made harder to parallelize if the stencil operation
    // iterated over inputs and scattered to output rather than iterating output
    // and accumulating from inputs; however, this could also be made better by
    // collapsing / combining some of these loops

    /* compute gradient for inside matrix using central difference */
    for (y = 1; y < nRows - 1; y++) {
        for (x = 1; x < nCols - 1; x++) {
            HLS_PROTO("kernel gradientXY 1");
            wait();
            fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[y * nCols + (x + 1)]);
            wait();
            fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[y * nCols + (x - 1)]);
            fp_x   = (fp_x_1 - fp_x_2) / FPDATA(2.0);
            wait();
            B0_gradient_x[y * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
            wait();
            fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(y - 1) * nCols + x]);
            wait();
            fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(y + 1) * nCols + x]);
            fp_y   = (fp_y_2 - fp_y_1) / FPDATA(2.0);
            wait();
            B0_gradient_y[y * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
        }
    }

    /* handle edge cases */
    /* compute gradient for outer matrix forward / backward difference */
    for (x = 1; x < nCols - 1; x++) {
        HLS_PROTO("kernel gradientXY 2");
        wait();
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[x + 1]);
        wait();
        fp_x_2           = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[x - 1]);
        fp_x             = (fp_x_1 - fp_x_2) / FPDATA(2.0);
        B0_gradient_x[x] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + (x + 1)]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + (x - 1)]);
        fp_x   = (fp_x_1 - fp_x_2) / FPDATA(2.0);
        wait();
        B0_gradient_x[(nRows - 1) * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[x]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[nCols + x]);
        fp_y   = fp_y_2 - fp_y_1;
        wait();
        B0_gradient_y[x] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + x]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 2) * nCols + x]);
        fp_y   = fp_y_1 - fp_y_2;
        wait();
        B0_gradient_y[(nRows - 1) * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
        wait();
    }
    for (y = 1; y < nRows - 1; y++) {
        HLS_PROTO("kernel gradientXY 3");
        wait();
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[y * nCols]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[y * nCols + 1]);
        fp_x   = fp_x_2 - fp_x_1;
        wait();
        B0_gradient_x[y * nCols] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[y * nCols + (nCols - 1)]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[y * nCols + (nCols - 2)]);
        fp_x   = fp_x_1 - fp_x_2;
        wait();
        B0_gradient_x[y * nCols + (nCols - 1)] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(y + 1) * nCols]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(y - 1) * nCols]);
        fp_y   = (fp_y_1 - fp_y_2) / FPDATA(2.0);
        wait();
        B0_gradient_y[y * nCols] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(y + 1) * nCols + (nCols - 1)]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(y - 1) * nCols + (nCols - 1)]);
        fp_y   = (fp_y_1 - fp_y_2) / FPDATA(2.0);
        wait();
        B0_gradient_y[y * nCols + (nCols - 1)] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
        wait();
    }

    /* compute corners */
    {
        HLS_PROTO("kernel gradientXY 4");
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[0]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[nCols + 0]);
        fp_y   = fp_y_2 - fp_y_1;
        wait();
        B0_gradient_y[0] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
    }
    {
        HLS_PROTO("kernel gradientXY 5");
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + 0]);
        wait();
        fp_y_2                                 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 2) * nCols + 0]);
        fp_y                                   = fp_y_1 - fp_y_2;
        B0_gradient_y[(nRows - 1) * nCols + 0] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
    }
    {
        HLS_PROTO("kernel gradientXY 6");
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nCols - 1)]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[nCols + (nCols - 1)]);
        fp_y   = fp_y_2 - fp_y_1;
        wait();
        B0_gradient_y[(nCols - 1)] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
    }
    {
        HLS_PROTO("kernel gradientXY 7");
        wait();
        fp_y_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + (nCols - 1)]);
        wait();
        fp_y_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 2) * nCols + (nCols - 1)]);
        fp_y   = fp_y_1 - fp_y_2;
        wait();
        B0_gradient_y[(nRows - 1) * nCols + (nCols - 1)] = fp2bv<FPDATA, FPDATA_WL>(fp_y);
        wait();
    }
    {
        HLS_PROTO("kernel gradientXY 8");
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[0 * nCols]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[0 * nCols + 1]);
        fp_x   = fp_x_2 - fp_x_1;
        wait();
        B0_gradient_x[0 * nCols] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
    }

    {
        HLS_PROTO("kernel gradientXY 9");
        wait();
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[0 * nCols + (nCols - 1)]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[0 * nCols + (nCols - 2)]);
        fp_x   = fp_x_1 - fp_x_2;
        wait();
        B0_gradient_x[0 * nCols + (nCols - 1)] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
    }
    {
        HLS_PROTO("kernel gradientXY 10");

        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + 1]);
        fp_x   = fp_x_2 - fp_x_1;
        wait();
        B0_gradient_x[(nRows - 1) * nCols] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
    }
    {
        HLS_PROTO("kernel gradientXY 11");
        wait();
        fp_x_1 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + (nCols - 1)]);
        wait();
        fp_x_2 = int2fp<FPDATA, FPDATA_WL>(A0_grayscale[(nRows - 1) * nCols + (nCols - 2)]);
        fp_x   = fp_x_1 - fp_x_2;
        wait();
        B0_gradient_x[(nRows - 1) * nCols + (nCols - 1)] = fp2bv<FPDATA, FPDATA_WL>(fp_x);
        wait();
    }
}

#endif // __WAMI_GRADIENT3_FUNCTIONS_HPP__
