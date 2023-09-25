// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_STEEP_DESCENT3_FUNCTIONS_HPP__
#define __WAMI_STEEP_DESCENT3_FUNCTIONS_HPP__

#include "wami_steep_descent3.hpp"


void wami_steep_descent3::sd_descent(uint32_t nRows, uint32_t nCols)
{
    int    k;
    int    x, y;
    FPDATA Jacobian_x[6];
    FPDATA Jacobian_y[6];
    int    index, j_index;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            index = y * nCols + x;
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

                B0_steep_descent[j_index] = tmp3;
            }
        }
    }
}

#endif // __WAMI_STEEP_DESCENT3_FUNCTIONS_HPP__
