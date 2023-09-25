// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_HESSIAN_INV_FUNCTIONS_HPP__
#define __WAMI_HESSIAN_INV_FUNCTIONS_HPP__

#include "wami_hessian_inv.hpp"

// Optional application-specific helper functions

// Compute the 1D x and y gradients of the input matrix / image

#define CLOSE_TO_ZERO(X) (((X) < FPDATA(1e-36)) && ((X) > FPDATA(-1e-36)))

void wami_hessian_inv::hess(uint32_t nCols, uint32_t nRows, uint32_t np)
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

void wami_hessian_inv::swap_row_workspace(uint32_t nRows, uint32_t nCols, uint32_t r1, uint32_t r2)
{
SWAP_ROW_LOOP:
    for (uint32_t x = 0; x < nCols; x++) {
        wait(); // STG_STATE_swap_row_0T:
        FPDATA_WORD tmp = C0_workspace[r1 * nCols + x];
        wait(); // STG_STATE_swap_row_1T:
        FPDATA_WORD tmp2 = C0_workspace[r2 * nCols + x];
        wait(); // STG_STATE_swap_row_2T:
        C0_workspace[r1 * nCols + x] = tmp2;
        wait(); // STG_STATE_swap_row_3T:
        C0_workspace[r2 * nCols + x] = tmp;
    }
}

void wami_hessian_inv::swap_row_inverse(uint32_t nRows, uint32_t nCols, uint32_t r1, uint32_t r2)
{
SWAP_ROW_LOOP:
    for (uint32_t x = 0; x < nCols; x++) {
        wait(); // STG_STATE_swap_row_0T:
        FPDATA_WORD tmp = B0_hess_inv[r1 * nCols + x];
        wait(); // STG_STATE_swap_row_1T:
        FPDATA_WORD tmp2 = B0_hess_inv[r2 * nCols + x];
        wait(); // STG_STATE_swap_row_2T:
        B0_hess_inv[r1 * nCols + x] = tmp2;
        wait(); // STG_STATE_swap_row_3T:
        B0_hess_inv[r2 * nCols + x] = tmp;
    }
}

void wami_hessian_inv::scale_row_workspace(uint32_t nRows, uint32_t nCols, uint32_t r, FPDATA scale)
{
SCALE_ROW_LOOP:
    for (uint32_t x = 0; x < nCols; x++) {
        wait(); // STG_STATE_scale_row_0:
        FPDATA_WORD tmp   = C0_workspace[r * nCols + x];
        FPDATA      tmp01 = int2fp<FPDATA, FPDATA_WL>(tmp);
        wait(); // STG_STATE_scale_row_1:
        FPDATA      tmp02 = tmp01 * scale;
        FPDATA_WORD tmp2  = fp2bv<FPDATA, FPDATA_WL>(tmp02);
        wait(); // STG_STATE_scale_row_2:
        C0_workspace[r * nCols + x] = tmp2;
        wait(); // STG_STATE_scale_row_3:
    }
}

void wami_hessian_inv::scale_row_inverse(uint32_t nRows, uint32_t nCols, uint32_t r, FPDATA scale)
{
SCALE_ROW_LOOP:
    for (uint32_t x = 0; x < nCols; x++) {
        wait(); // STG_STATE_scale_row_0:
        FPDATA_WORD tmp   = B0_hess_inv[r * nCols + x];
        FPDATA      tmp01 = int2fp<FPDATA, FPDATA_WL>(tmp);
        wait(); // STG_STATE_scale_row_1:
        FPDATA      tmp02 = tmp01 * scale;
        FPDATA_WORD tmp2  = fp2bv<FPDATA, FPDATA_WL>(tmp02);
        wait(); // STG_STATE_scale_row_2:
        B0_hess_inv[r * nCols + x] = tmp2;
        wait(); // STG_STATE_scale_row_3:
    }
}

void wami_hessian_inv::scale_and_add_row_workspace(uint32_t nRows, uint32_t nCols, uint32_t r1, uint32_t r2,
                                                   FPDATA scale)
{
    wait(); // SCALE_AND_ADD_ROW_LOOP:
    for (uint32_t x = 0; x < nCols; x++) {
        wait(); // STG_STATE_scale_and_add_row_0:
        FPDATA_WORD tmp   = C0_workspace[r2 * nCols + x];
        FPDATA      tmp01 = int2fp<FPDATA, FPDATA_WL>(tmp);

        wait(); // STG_STATE_scale_and_add_row_1:
        FPDATA tmp02 = scale * tmp01;
        wait(); // STG_STATE_scale_and_add_row_2:
        FPDATA_WORD tmp3  = C0_workspace[r1 * nCols + x];
        FPDATA      tmp03 = int2fp<FPDATA, FPDATA_WL>(tmp3);

        wait(); // STG_STATE_scale_and_add_row_3:
        FPDATA      tmp04            = tmp03 + tmp02;
        FPDATA_WORD tmp4             = fp2bv<FPDATA, FPDATA_WL>(tmp04);
        C0_workspace[r1 * nCols + x] = tmp4;
        wait(); // STG_STATE_scale_and_add_row_4:
    }
}

void wami_hessian_inv::scale_and_add_row_inverse(uint32_t nRows, uint32_t nCols, uint32_t r1, uint32_t r2, FPDATA scale)
{
    wait(); // SCALE_AND_ADD_ROW_LOOP:
    for (uint32_t x = 0; x < nCols; x++) {
        wait(); // STG_STATE_scale_and_add_row_0:
        FPDATA_WORD tmp   = B0_hess_inv[r2 * nCols + x];
        FPDATA      tmp01 = int2fp<FPDATA, FPDATA_WL>(tmp);

        wait(); // STG_STATE_scale_and_add_row_1:
        FPDATA tmp02 = scale * tmp01;
        wait(); // STG_STATE_scale_and_add_row_2:
        FPDATA_WORD tmp3  = B0_hess_inv[r1 * nCols + x];
        FPDATA      tmp03 = int2fp<FPDATA, FPDATA_WL>(tmp3);

        wait(); // STG_STATE_scale_and_add_row_3:
        FPDATA      tmp04           = tmp03 + tmp02;
        FPDATA_WORD tmp4            = fp2bv<FPDATA, FPDATA_WL>(tmp04);
        B0_hess_inv[r1 * nCols + x] = tmp4;
        wait(); // STG_STATE_scale_and_add_row_4:
    }
}

void wami_hessian_inv::invert_gj(uint32_t nRows, uint32_t nCols)
{
    fprintf(stderr, "[function]: invert_gj\n");

    uint32_t row, col;
    // int      i;

    // for (i = 0; i < 36; i++) {
    //     FPDATA_WORD temp0111 = A0_hess[i];
    //     FPDATA      tmp111   = int2fp<FPDATA, FPDATA_WL>(temp0111);
    //     fprintf(stderr, "A0_hess: %d\t %f\n", i, (float)tmp111);
    // }

    /* set inverse to identity */
    for (uint32_t y = 0; y < nRows; y++) {
        for (uint32_t x = 0; x < nCols; x++) {

            if (x == y) {
                B0_hess_inv[y * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(FPDATA(1.0));
            } else {
                B0_hess_inv[y * nCols + x] = fp2bv<FPDATA, FPDATA_WL>(FPDATA(0.0));
            }

            FPDATA_WORD temp01          = B0_hess[y * nCols + x]; // A0_hess[y * nCols + x];
            C0_workspace[y * nCols + x] = temp01;
        }
    }

    /* for each column */
    for (col = 0; col < nCols; col++) {
        FPDATA scale = FPDATA(1.0);

        /* swap rows if close to zero */
        FPDATA_WORD temp03 = C0_workspace[col * nCols + col];
        FPDATA      tmp3   = int2fp<FPDATA, FPDATA_WL>(temp03);
        if (CLOSE_TO_ZERO(tmp3)) {
            for (row = col + 1; row < nCols; row++) {
                FPDATA_WORD temp04 = C0_workspace[col * nCols + col];
                FPDATA      tmp4   = int2fp<FPDATA, FPDATA_WL>(temp04);
                wait(); // STG_STATE_invert_gj_2:
                if (!CLOSE_TO_ZERO(tmp4)) {
                    swap_row_workspace(nRows, nCols, row, col);
                    swap_row_inverse(nRows, nCols, row, col);
                    break;
                }
            }
        }

        /* scale operation */
        FPDATA_WORD temp05 = C0_workspace[col * nCols + col];
        FPDATA      tmp5   = int2fp<FPDATA, FPDATA_WL>(temp05);
        scale              = FPDATA(0.0); // FPDATA(1.0) / tmp5; // [humu]: the scale in sw is close to 0 anyway
        // scale = cynw_fixed<FPDATA_WL, FPDATA_WL, SC_TRN, SC_WRAP, 1>(FPDATA(1.0) / tmp5);

        scale_row_workspace(nRows, nCols, col, scale);
        scale_row_inverse(nRows, nCols, col, scale);

        // THIS IS A BUG!!!
        // If floating-point precision is used the code is an "identity-function".
        // It scales the rows of 1: (1/CONST)/CONST = 1.
        // If you port to fixed-point precision it introduces an approximation.
        //
        // scale = INVGJ64_CTOS_FLOAT(1.0) / C0_workspace[col * nCols + col];
        // scale_row(workspace, nRows, nCols, col, scale);
        // scale_row(inverse, nRows, nCols, col, scale);

        /* zero column */
        for (row = 0; row < nRows; row++) {
            wait(); // STG_STATE_invert_gj_3:
            if (row != col) {
                FPDATA_WORD temp06 = C0_workspace[row * nCols + col];
                FPDATA      tmp6   = int2fp<FPDATA, FPDATA_WL>(temp06);

                scale = tmp6 * FPDATA(-1);

                scale_and_add_row_workspace(nRows, nCols, row, col, scale);
                scale_and_add_row_inverse(nRows, nCols, row, col, scale);
            }
        }
    }
}

#endif // __WAMI_HESSIAN_INV_FUNCTIONS_HPP__
