// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_STEEP_DESCENT_HESSIAN_FUNCTIONS_HPP__
#define __WAMI_STEEP_DESCENT_HESSIAN_FUNCTIONS_HPP__

#include "wami_steep_descent_hessian.hpp"

// Optional application-specific helper functions

// Compute the 1D x and y gradients of the input matrix / image

void wami_steep_descent_hessian::sd_descent(uint32_t nRows, uint32_t nCols)
{
    // printf("kernl: debug 1\n");
    int    k;
    int    x, y;
    FPDATA Jacobian_x[6];
    FPDATA Jacobian_y[6];
    int    index, j_index;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            index = y * nCols + x;
            // printf("kernl: debug 2: %d\n", index);
            Jacobian_x[0] = FPDATA(x);
            Jacobian_x[1] = FPDATA(0.0);
            Jacobian_x[2] = FPDATA(y);
            Jacobian_x[3] = FPDATA(0.0);
            Jacobian_x[4] = FPDATA(1.0);
            Jacobian_x[5] = FPDATA(0.0);

            Jacobian_y[0] = FPDATA(0.0);
            Jacobian_y[1] = FPDATA(x);
            Jacobian_y[2] = FPDATA(0.0);
            Jacobian_y[3] = FPDATA(y);
            Jacobian_y[4] = FPDATA(0.0);
            Jacobian_y[5] = FPDATA(1.0);

            for (k = 0; k < 6; k++) {
                j_index          = (6 * y * nCols) + (nCols * k) + x;
                FPDATA_WORD tmp1 = A0_in1[index];
                FPDATA_WORD tmp2 = A0_in2[index];

                FPDATA fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
                FPDATA fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);

                FPDATA temp01 = fp_1 * Jacobian_x[k];
                FPDATA temp02 = fp_2 * Jacobian_y[k];

                FPDATA      temp03 = temp01 + temp02;
                FPDATA_WORD tmp3   = fp2bv<FPDATA, FPDATA_WL>(temp03);

                // B0_steep_descent[j_index] = tmp3;
                A0_steepest_descent[j_index] = tmp3;
            }
        }
    }
}

void wami_steep_descent_hessian::hess(uint32_t nCols, uint32_t nRows, uint32_t np)
{
    int i, j;
    int x, y;

    for (i = 0; i < np; i++) {
        for (j = 0; j < np; j++) {
            FPDATA      x       = FPDATA(0.0);
            FPDATA_WORD tmp3    = fp2bv<FPDATA, FPDATA_WL>(x);
            B0_hess[i * np + j] = tmp3;
        }
    }

    for (y = 0; y < nRows; y++) {
        /* compare each image in the 6-wide I_steepest to each other image */
        for (i = 0; i < np; i++) {
            for (j = 0; j < np; j++) { /* sum the element-wise product of the images */
                FPDATA total = FPDATA(0.0);
                for (x = 0; x < nCols; x++) {
                    int         index1 = (np * y * nCols) + (nCols * i) + x;
                    int         index2 = (np * y * nCols) + (nCols * j) + x;
                    FPDATA_WORD tmp1   = A0_steepest_descent[index1];
                    FPDATA_WORD tmp2   = A0_steepest_descent[index2];

                    FPDATA in_fp_1 = int2fp<FPDATA, FPDATA_WL>(tmp1);
                    FPDATA in_fp_2 = int2fp<FPDATA, FPDATA_WL>(tmp2);

                    FPDATA temp03 = total + (in_fp_1 * in_fp_2);
                    total         = temp03;
                }

                FPDATA_WORD tmp0    = B0_hess[np * j + i];
                FPDATA      in_fp_0 = int2fp<FPDATA, FPDATA_WL>(tmp0);
                FPDATA      temp04  = total + in_fp_0;

                FPDATA_WORD tmp3 = fp2bv<FPDATA, FPDATA_WL>(temp04);
                wait();
                B0_hess[np * j + i] = tmp3;
            }
        }
    }
}
#endif // __WAMI_STEEP_DESCENT_HESSIAN_FUNCTIONS_HPP__
