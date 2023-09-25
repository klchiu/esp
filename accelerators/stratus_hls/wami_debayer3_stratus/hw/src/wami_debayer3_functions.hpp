// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_DEBAYER3_FUNCTIONS_HPP__
#define __WAMI_DEBAYER3_FUNCTIONS_HPP__

#include "wami_debayer3.hpp"

// Optional application-specific helper functions

#define PIXEL_MAX WAMI_DEBAYER_IMG_PIXEL_MAX // 65535

static uint16_t __compute_and_clamp_pixel(uint16_t pos, uint16_t neg)
{
    if (pos < neg) {
        return 0;
    } else {
        const uint16_t pixel = (pos - neg) >> 3;
        if (pixel > PIXEL_MAX) {
            return PIXEL_MAX;
        } else {
            return pixel;
        }
    }
}

/*
 * This version handles masks with fractional negative values. In those
 * cases truncating before subtraction does not generally yield the
 * same result as truncating after subtraction.  The negative value
 * is using weights in units of 1/16ths so that the one-half portions
 * are retained.
 */
uint16_t wami_debayer3::__compute_and_clamp_pixel_fractional_neg(uint16_t pos, uint16_t neg)
{
    /*
     * The positive portion is converted to uint32_t prior to doubling because
     * otherwise some of the weights could yield overflow. At that point,
     * all weights are effectively 16x their actual value, so combining
     * the positive and negative portions and then shifting by four bits
     * yields the equivalent of a floor() applied to the result of the
     * full precision convolution.
     */
    const uint32_t pos_u32 = ((uint32_t)pos) << 1;
    const uint32_t neg_u32 = (uint32_t)neg;
    if (pos_u32 < neg_u32) {
        return 0;
    } else {
        const uint16_t pixel = (uint16_t)((pos_u32 - neg_u32) >> 4);
        if (pixel > PIXEL_MAX) {
            return PIXEL_MAX;
        } else {
            return pixel;
        }
    }
}

uint16_t wami_debayer3::__interp_G_at_RRR_or_G_at_BBB(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols)
{
    /*
     * The mask to interpolate G at R or B is:
     *
     * [0  0 -1  0  0
     *  0  0  2  0  0
     * -1  2  4  2 -1
     *  0  0  2  0  0
     *  0  0 -1  0  0] /8
     */
    uint16_t pos, neg;
    {
        HLS_PROTO("__interp_G_at_RRR_or_G_at_BBB");
        uint16_t pos1 = 2 * A0_bayer[ncols * (row - 1) + (col)];
        wait();
        uint16_t pos2 = 2 * A0_bayer[ncols * (row) + (col - 1)];
        wait();
        uint16_t pos3 = 4 * A0_bayer[ncols * (row) + (col)];
        wait();
        uint16_t pos4 = 2 * A0_bayer[ncols * (row) + (col + 1)];
        wait();
        uint16_t pos5 = 2 * A0_bayer[ncols * (row + 1) + (col)];
        wait();
        pos = pos1 + pos2 + pos3 + pos4 + pos5;
        wait();
        uint16_t neg1 = A0_bayer[ncols * (row) + (col + 2)];
        wait();
        uint16_t neg2 = A0_bayer[ncols * (row - 2) + (col)];
        wait();
        uint16_t neg3 = A0_bayer[ncols * (row) + (col - 2)];
        wait();
        uint16_t neg4 = A0_bayer[ncols * (row + 2) + (col)];
        wait();
        neg = neg1 + neg2 + neg3 + neg4;
    }
    return __compute_and_clamp_pixel(pos, neg);
}

uint16_t wami_debayer3::__interp_R_at_GRB_or_B_at_GBR(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols)
{
    /*
     * [0  0 0.5 0  0
     *  0 -1  0 -1  0
     * -1  4  5  4 -1
     *  0 -1  0 -1  0
     *  0  0 0.5 0  0] /8;
     */
    uint16_t pos, neg;
    {
        HLS_PROTO("__interp_R_at_GRB_or_B_at_GBR");

        uint16_t pos1 = A0_bayer[ncols * (row - 2) + (col)];
        wait();
        uint16_t pos2 = A0_bayer[ncols * (row + 2) + (col)];
        wait();
        uint16_t pos3 = 4 * A0_bayer[ncols * (row) + (col - 1)];
        wait();
        uint16_t pos4 = 5 * A0_bayer[ncols * (row) + (col)];
        wait();
        uint16_t pos5 = 4 * A0_bayer[ncols * (row) + (col + 1)];
        wait();
        pos = ((pos1 + pos2) >> 1) + pos3 + pos4 + pos5;
        wait();
        uint16_t neg1 = A0_bayer[ncols * (row - 1) + (col - 1)];
        wait();
        uint16_t neg2 = A0_bayer[ncols * (row - 1) + (col + 1)];
        wait();
        uint16_t neg3 = A0_bayer[ncols * (row) + (col - 2)];
        wait();
        uint16_t neg4 = A0_bayer[ncols * (row) + (col + 2)];
        wait();
        uint16_t neg5 = A0_bayer[ncols * (row + 1) + (col - 1)];
        wait();
        uint16_t neg6 = A0_bayer[ncols * (row + 1) + (col + 1)];
        wait();
        neg = neg1 + neg2 + neg3 + neg4 + neg5 + neg6;
        wait();
    }
    return __compute_and_clamp_pixel(pos, neg);
}

uint16_t wami_debayer3::__interp_R_at_GBR_or_B_at_GRB(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols)
{
    /*
     * [0  0 -1  0  0
     *  0 -1  4 -1  0
     * 0.5 0  5  0 0.5
     *  0 -1  4 -1  0
     *  0  0 -1  0  0] /8;
     */
    uint16_t pos, neg;

    {
        HLS_PROTO("__interp_R_at_GBR_or_B_at_GRB");

        uint16_t pos1 = 4 * A0_bayer[ncols * (row - 1) + (col)];
        wait();
        uint16_t pos2 = A0_bayer[ncols * (row) + (col - 2)];
        wait();
        uint16_t pos3 = A0_bayer[ncols * (row) + (col + 2)];
        wait();
        uint16_t pos4 = 5 * A0_bayer[ncols * (row) + (col)];
        wait();
        uint16_t pos5 = 4 * A0_bayer[ncols * (row + 1) + (col)];
        wait();
        pos = pos1 + ((pos2 + pos3) >> 1) + pos4 + pos5;
        wait();
        uint16_t neg1 = A0_bayer[ncols * (row - 2) + (col)];
        wait();
        uint16_t neg2 = A0_bayer[ncols * (row - 1) + (col - 1)];
        wait();
        uint16_t neg3 = A0_bayer[ncols * (row - 1) + (col + 1)];
        wait();
        uint16_t neg4 = A0_bayer[ncols * (row + 1) + (col - 1)];
        wait();
        uint16_t neg5 = A0_bayer[ncols * (row + 1) + (col + 1)];
        wait();
        uint16_t neg6 = A0_bayer[ncols * (row + 2) + (col)];
        wait();
        neg = neg1 + neg2 + neg3 + neg4 + neg5 + neg6;
        wait();
    }

    return __compute_and_clamp_pixel(pos, neg);
}

uint16_t wami_debayer3::__interp_R_at_BBB_or_B_at_RRR(uint32_t row, uint32_t col, uint32_t nrows, uint32_t ncols)
{
    /*
     * [0   0 -1.5 0  0
     *  0   2  0   2  0
     * -1.5 0  6   0 -1.5
     *  0   2  0   2  0
     *  0   0 -1.5 0  0] /8;
     */
    uint16_t pos, neg;
    {
        HLS_PROTO("__interp_R_at_BBB_or_B_at_RRR");
        wait();
        uint16_t pos1 = 2 * A0_bayer[ncols * (row - 1) + (col - 1)];
        wait();
        uint16_t pos2 = 2 * A0_bayer[ncols * (row - 1) + (col + 1)];
        wait();
        uint16_t pos3 = 6 * A0_bayer[ncols * (row) + (col)];
        wait();
        uint16_t pos4 = 2 * A0_bayer[ncols * (row + 1) + (col - 1)];
        wait();
        uint16_t pos5 = 2 * A0_bayer[ncols * (row + 1) + (col + 1)];
        wait();
        pos = pos1 + pos2 + pos3 + pos4 + pos5;
        wait();
        uint16_t neg1 = 3 * A0_bayer[ncols * (row - 2) + (col)];
        wait();
        uint16_t neg2 = 3 * A0_bayer[ncols * (row) + (col - 2)];
        wait();
        uint16_t neg3 = 3 * A0_bayer[ncols * (row) + (col + 2)];
        wait();
        uint16_t neg4 = 3 * A0_bayer[ncols * (row + 2) + (col)];
        wait();
        neg = neg1 + neg2 + neg3 + neg4;
        wait();
    }

    return __compute_and_clamp_pixel_fractional_neg(pos, neg);
}

void wami_debayer3::wami_debayer(uint32_t nrows, uint32_t ncols)
{
    uint32_t row, col;
    uint16_t tmp;
    /*
     * Demosaic the following Bayer pattern:
     * R G ...
     * G B ...
     * ... ...
     */

    /* Copy red pixels through directly */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-1");
            wait();
            tmp = A0_bayer[ncols * (row) + (col)];
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r = tmp;
            wait();
        }
    }

    /* Copy top-right green pixels through directly */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-2");
            wait();
            tmp = A0_bayer[ncols * (row) + (col)];
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g = tmp;
            wait();
        }
    }

    /* Copy bottom-left green pixels through directly */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loo-3");
            wait();
            tmp = A0_bayer[ncols * (row) + (col)];
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g = tmp;
            wait();
        }
    }

    /* Copy blue pixels through directly */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-4");
            wait();
            tmp = A0_bayer[ncols * (row) + (col)];
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b = tmp;
            wait();
        }
    }

    /* Interpolate green pixels at red pixels */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-5");
            wait();
            tmp = __interp_G_at_RRR_or_G_at_BBB(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g = tmp;
            wait();
        }
    }

    /* Interpolate green pixels at blue pixels */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-6");
            wait();
            tmp = __interp_G_at_RRR_or_G_at_BBB(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g = tmp;
            wait();
        }
    }

    /* Interpolate red pixels at green pixels, red row, blue column */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-7");
            wait();
            tmp = __interp_R_at_GRB_or_B_at_GBR(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r = tmp;
            wait();
        }
    }

    /* Interpolate blue pixels at green pixels, blue row, red column */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-8");
            wait();
            tmp = __interp_R_at_GRB_or_B_at_GBR(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b = tmp;
            wait();
        }
    }

    /* Interpolate red pixels at green pixels, blue row, red column */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-9");
            wait();
            tmp = __interp_R_at_GBR_or_B_at_GRB(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r = tmp;
            wait();
        }
    }

    /* Interpolate blue pixels at green pixels, red row, blue column */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-10");
            wait();
            tmp = __interp_R_at_GBR_or_B_at_GRB(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b = tmp;
            wait();
        }
    }

    /* Interpolate red pixels at blue pixels, blue row, blue column */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-11");
            wait();
            tmp = __interp_R_at_BBB_or_B_at_RRR(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r = tmp;
            wait();
        }
    }

    /* Interpolate blue pixels at red pixels, red row, red column */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            HLS_PROTO("debayer-loop-12");
            wait();
            tmp = __interp_R_at_BBB_or_B_at_RRR(row, col, nrows, ncols);
            wait();
            B0_debayer[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b = tmp;
            wait();
        }
    }
}

#endif // __WAMI_DEBAYER3_FUNCTIONS_HPP__
