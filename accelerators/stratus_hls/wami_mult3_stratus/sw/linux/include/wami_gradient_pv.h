#ifndef _WAMI_GRADIENT_PV_H_
#define _WAMI_GRADIENT_PV_H_

#include "wami_params.h"

int __gradientXY(flt_pixel_t *Iin, int nCols, int nRows, flt_pixel_t *Xgradout, flt_pixel_t *Ygradout);

typedef float fltPixel_t;

/* compute the 1D x and y gradients of the input matrix / image */
int __gradientXY(fltPixel_t *Iin, int nCols, int nRows, fltPixel_t *Xgradout, fltPixel_t *Ygradout)
{
    int x, y;

    if (!Iin || !Xgradout || !Ygradout)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    /* NOTE this could be made harder to parallelize if the stencil operation iterated over inputs
     * and scattered to output rather than iterating output and accumulating from inputs; however,
     * this could also be made better by collapsing / combining some of these loops */

    /* compute gradient for inside matrix using central difference */
    for (y = 1; y < nRows - 1; y++) {
        for (x = 1; x < nCols - 1; x++) {
            Xgradout[y * nCols + x] = (Iin[y * nCols + (x + 1)] - Iin[y * nCols + (x - 1)]) / 2.0;
            Ygradout[y * nCols + x] = (-Iin[(y - 1) * nCols + x] + Iin[(y + 1) * nCols + x]) / 2.0;
        }
    }

    /* handle edge cases */
    /* compute gradient for outer matrix forward / backward difference */
    for (x = 1; x < nCols - 1; x++) {
        Xgradout[x] = (Iin[(x + 1)] - Iin[(x - 1)]) / 2.0;
        Xgradout[(nRows - 1) * nCols + x] =
            (Iin[(nRows - 1) * nCols + (x + 1)] - Iin[(nRows - 1) * nCols + (x - 1)]) / 2.0;
        Ygradout[x]                       = -Iin[x] + Iin[nCols + x];
        Ygradout[(nRows - 1) * nCols + x] = Iin[(nRows - 1) * nCols + x] - Iin[(nRows - 2) * nCols + x];
    }
    for (y = 1; y < nRows - 1; y++) {
        Xgradout[y * nCols]               = -Iin[y * nCols] + Iin[y * nCols + 1];
        Xgradout[y * nCols + (nCols - 1)] = Iin[y * nCols + (nCols - 1)] - Iin[y * nCols + (nCols - 2)];
        Ygradout[y * nCols]               = (Iin[(y + 1) * nCols] - Iin[(y - 1) * nCols]) / 2.0;
        Ygradout[y * nCols + (nCols - 1)] =
            (Iin[(y + 1) * nCols + (nCols - 1)] - Iin[(y - 1) * nCols + (nCols - 1)]) / 2.0;
    }

    /* compute corners */
    Ygradout[0]                       = -Iin[0] + Iin[nCols + 0];
    Ygradout[(nRows - 1) * nCols + 0] = Iin[(nRows - 1) * nCols + 0] - Iin[(nRows - 2) * nCols + 0];
    Ygradout[(nCols - 1)]             = -Iin[(nCols - 1)] + Iin[nCols + (nCols - 1)];
    Ygradout[(nRows - 1) * nCols + (nCols - 1)] =
        Iin[(nRows - 1) * nCols + (nCols - 1)] - Iin[(nRows - 2) * nCols + (nCols - 1)];
    Xgradout[0 * nCols]               = -Iin[0 * nCols] + Iin[0 * nCols + 1];
    Xgradout[0 * nCols + (nCols - 1)] = Iin[0 * nCols + (nCols - 1)] - Iin[0 * nCols + (nCols - 2)];
    Xgradout[(nRows - 1) * nCols]     = -Iin[(nRows - 1) * nCols] + Iin[(nRows - 1) * nCols + 1];
    Xgradout[(nRows - 1) * nCols + (nCols - 1)] =
        Iin[(nRows - 1) * nCols + (nCols - 1)] - Iin[(nRows - 1) * nCols + (nCols - 2)];

    return 0;
}

#ifdef GRADIENT_TEST
int main(int argc, char **argv)
{
    float input[]      = {0, 1, 0, 1, 0, 1, 0, 1, 0};
    float input_copy[] = {1, 1, 1, 0, 1, 1, 0, 0, 1};
    float inverse[9];
    float result[9];

    printf("Gradient of:\n");
    print_matrix(input, 3, 3);
    gradientXY(input, 3, 3, inverse, result);
    printf("is:\n");
    print_matrix(inverse, 3, 3);
    print_matrix(result, 3, 3);
}
#endif


#endif /* _WAMI_GRADIENT_PV_H_ */
