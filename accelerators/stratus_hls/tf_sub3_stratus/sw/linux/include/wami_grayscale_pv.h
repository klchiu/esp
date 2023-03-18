#ifndef _TF_SUB3_PV_H_
#define _TF_SUB3_PV_H_

#include "wami_params.h"
#include "wami_typedef.h"

/* using RGB to lumincance covnersion for ITU-R BT.709 / sRGB
      Y = (0.2126*R) + (0.7152*G) + (0.0722*B) */
int __rgb_to_grayscale_row(rgb_pixel_t *color, fltPixel_t *out, uint32_t ncols);

int __rgb_to_grayscale(rgb_pixel_t *color, fltPixel_t *out, uint32_t nrows, uint32_t ncols);


int __rgb_to_grayscale_row(rgb_pixel_t *color, fltPixel_t *out, uint32_t ncols)
{
    int col;

    if (!color || !out)
        return -1;

    /* using RGB to lumincance covnersion for ITU-R BT.709 / sRGB
          Y = (0.2126*R) + (0.7152*G) + (0.0722*B) */

    for (col = 0; col < ncols; col++) {
        out[col] = color[col].r * 0.2126 + color[col].g * 0.7152 + color[col].b * 0.0722;
    }

    return 0;
}

int __rgb_to_grayscale(rgb_pixel_t *color, fltPixel_t *out, uint32_t nrows, uint32_t ncols)
{
    int row;

    if (!color || !out)
        return -1;

    /* using RGB to lumincance covnersion for ITU-R BT.709 / sRGB
          Y = (0.2126*R) + (0.7152*G) + (0.0722*B) */

    for (row = 0; row < nrows; row++) {
        __rgb_to_grayscale_row(color + row * ncols, out + row * ncols, ncols);
    }

    return 0;
}


#endif /* _TF_SUB3_PV_H_ */
