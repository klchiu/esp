#ifndef _WAMI_DEBAYER_PV_H_
#define _WAMI_DEBAYER_PV_H_

#include "wami_params.h"

void __wami_debayer(uint16_t *const bayer, rgb_pixel_t *debayered, uint32_t nrows, uint32_t ncols);


#define PIXEL_MAX 65535

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
static uint16_t __compute_and_clamp_pixel_fractional_neg(uint16_t pos, uint16_t neg)
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

static uint16_t __interp_G_at_RRR_or_G_at_BBB(uint16_t *const bayer, uint32_t row, uint32_t col, uint32_t nrows,
                                              uint32_t ncols)
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
    const uint16_t pos = 2 * bayer[ncols * (row - 1) + (col)] + 2 * bayer[ncols * (row) + (col - 1)] +
                         4 * bayer[ncols * (row) + (col)] + 2 * bayer[ncols * (row) + (col + 1)] +
                         2 * bayer[ncols * (row + 1) + (col)];
    const uint16_t neg = bayer[ncols * (row) + (col + 2)] + bayer[ncols * (row - 2) + (col)] +
                         bayer[ncols * (row) + (col - 2)] + bayer[ncols * (row + 2) + (col)];

    return __compute_and_clamp_pixel(pos, neg);
}

static uint16_t __interp_R_at_GRB_or_B_at_GBR(uint16_t *const bayer, uint32_t row, uint32_t col, uint32_t nrows,
                                              uint32_t ncols)
{
    /*
     * [0  0 0.5 0  0
     *  0 -1  0 -1  0
     * -1  4  5  4 -1
     *  0 -1  0 -1  0
     *  0  0 0.5 0  0] /8;
     */
    const uint16_t pos = ((bayer[ncols * (row - 2) + (col)] + bayer[ncols * (row + 2) + (col)]) >> 1) +
                         4 * bayer[ncols * (row) + (col - 1)] + 5 * bayer[ncols * (row) + (col)] +
                         4 * bayer[ncols * (row) + (col + 1)];
    const uint16_t neg = bayer[ncols * (row - 1) + (col - 1)] + bayer[ncols * (row - 1) + (col + 1)] +
                         bayer[ncols * (row) + (col - 2)] + bayer[ncols * (row) + (col + 2)] +
                         bayer[ncols * (row + 1) + (col - 1)] + bayer[ncols * (row + 1) + (col + 1)];

    return __compute_and_clamp_pixel(pos, neg);
}

static uint16_t __interp_R_at_GBR_or_B_at_GRB(uint16_t *const bayer, uint32_t row, uint32_t col, uint32_t nrows,
                                              uint32_t ncols)
{
    /*
     * [0  0 -1  0  0
     *  0 -1  4 -1  0
     * 0.5 0  5  0 0.5
     *  0 -1  4 -1  0
     *  0  0 -1  0  0] /8;
     */
    const uint16_t pos = 4 * bayer[ncols * (row - 1) + (col)] +
                         ((bayer[ncols * (row) + (col - 2)] + bayer[ncols * (row) + (col + 2)]) >> 1) +
                         5 * bayer[ncols * (row) + (col)] + 4 * bayer[ncols * (row + 1) + (col)];
    const uint16_t neg = bayer[ncols * (row - 2) + (col)] + bayer[ncols * (row - 1) + (col - 1)] +
                         bayer[ncols * (row - 1) + (col + 1)] + bayer[ncols * (row + 1) + (col - 1)] +
                         bayer[ncols * (row + 1) + (col + 1)] + bayer[ncols * (row + 2) + (col)];

    return __compute_and_clamp_pixel(pos, neg);
}

static uint16_t __interp_R_at_BBB_or_B_at_RRR(uint16_t *const bayer, uint32_t row, uint32_t col, uint32_t nrows,
                                              uint32_t ncols)
{
    /*
     * [0   0 -1.5 0  0
     *  0   2  0   2  0
     * -1.5 0  6   0 -1.5
     *  0   2  0   2  0
     *  0   0 -1.5 0  0] /8;
     */
    const uint16_t pos = 2 * bayer[ncols * (row - 1) + (col - 1)] + 2 * bayer[ncols * (row - 1) + (col + 1)] +
                         6 * bayer[ncols * (row) + (col)] + 2 * bayer[ncols * (row + 1) + (col - 1)] +
                         2 * bayer[ncols * (row + 1) + (col + 1)];
    const uint16_t neg = (3 * bayer[ncols * (row - 2) + (col)] + 3 * bayer[ncols * (row) + (col - 2)] +
                          3 * bayer[ncols * (row) + (col + 2)] + 3 * bayer[ncols * (row + 2) + (col)]);

    return __compute_and_clamp_pixel_fractional_neg(pos, neg);
}

void __wami_debayer(uint16_t *const bayer, rgb_pixel_t *debayered, uint32_t nrows, uint32_t ncols)
{
    uint32_t row, col;

    /*
     * Demosaic the following Bayer pattern:
     * R G ...
     * G B ...
     * ... ...
     */

    /* Copy red pixels through directly */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r = bayer[ncols * (row) + (col)];
        }
    }

    /* Copy top-right green pixels through directly */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g = bayer[ncols * (row) + (col)];
        }
    }

    /* Copy bottom-left green pixels through directly */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g = bayer[ncols * (row) + (col)];
        }
    }

    /* Copy blue pixels through directly */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b = bayer[ncols * (row) + (col)];
        }
    }

    /* Interpolate green pixels at red pixels */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g =
                __interp_G_at_RRR_or_G_at_BBB(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate green pixels at blue pixels */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].g =
                __interp_G_at_RRR_or_G_at_BBB(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate red pixels at green pixels, red row, blue column */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r =
                __interp_R_at_GRB_or_B_at_GBR(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate blue pixels at green pixels, blue row, red column */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b =
                __interp_R_at_GRB_or_B_at_GBR(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate red pixels at green pixels, blue row, red column */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r =
                __interp_R_at_GBR_or_B_at_GRB(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate blue pixels at green pixels, red row, blue column */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b =
                __interp_R_at_GBR_or_B_at_GRB(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate red pixels at blue pixels, blue row, blue column */
    for (row = PAD + 1; row < nrows - PAD; row += 2) {
        for (col = PAD + 1; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].r =
                __interp_R_at_BBB_or_B_at_RRR(bayer, row, col, nrows, ncols);
        }
    }

    /* Interpolate blue pixels at red pixels, red row, red column */
    for (row = PAD; row < nrows - PAD; row += 2) {
        for (col = PAD; col < ncols - PAD; col += 2) {
            debayered[(ncols - (2 * PAD)) * (row - PAD) + (col - PAD)].b =
                __interp_R_at_BBB_or_B_at_RRR(bayer, row, col, nrows, ncols);
        }
    }
}



#endif /* _WAMI_DEBAYER_PV_H_ */
