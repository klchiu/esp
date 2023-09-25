#ifndef _WAMI_MATRIX_OPS_HPP_
#define _WAMI_MATRIX_OPS_HPP_

#include <stdio.h>
#include <math.h>

#include "../wami_typedef.hpp"
#include "../wami_params_forpv.hpp"

/* Basic operations for matrices of floating point numbers - naive implementations, inputs
 * checked for sanity unless otherwise noted.
 */

int __reshape(fltPixel_t *in, int nRows, int nCols, int newRows, int newCols, fltPixel_t *out);

int __subtract(fltPixel_t *inA, fltPixel_t *inB, int nRows, int nCols, fltPixel_t *out);

int __add(fltPixel_t *inA, fltPixel_t *inB, int nRows, int nCols, fltPixel_t *out);

/* A [nRows x nCommon] * B [nCommon x nCols] -> out[nRows x nCols] */
int __mult(fltPixel_t *inA, fltPixel_t *inB, int nRows, int nCols, int nCommon, fltPixel_t *out);

int __swap_row(fltPixel_t *data, int nRows, int nCols, int r1, int r2);

int __scale_row(fltPixel_t *data, int nRows, int nCols, int r, float scale);

int __scale_and_add_row(fltPixel_t *data, int nRows, int nCols, int r1, int r2, float scale);

int __invert_gauss_jordan(fltPixel_t *data, fltPixel_t *workspace, int nRows, int nCols, fltPixel_t *inverse);

int print_matrix(fltPixel_t *mat, int nRows, int nCols);

int print_submatrix(fltPixel_t *mat, int nRows, int nCols, int width, int height);

int print_submatrix_uint16(uint16_t *mat, int nRows, int nCols, int width, int height);

int print_submatrix_uint8_file(FILE *fp, char *name, uint8_t *mat, int nRows, int nCols, int width, int height);

int print_submatrix_rgb_file(FILE *fp, char *name, rgb_pixel_t *mat, int nRows, int nCols, int width, int height);

int print_submatrix_file(FILE *fp, char *name, fltPixel_t *mat, int nRows, int nCols, int width, int height);

int print_submatrix_rgb(rgb_pixel_t *mat, int nRows, int nCols, int width, int height, int channel);



#define CLOSE_TO_ZERO(X) (((X) < 1e-36) && ((X) > -1e-36))

int __reshape(fltPixel_t *in, int nRows, int nCols, int newRows, int newCols, fltPixel_t *out)
{
    int cur_index;

    if (!in || !out)
        return -1;

    if (nCols < 0 || nRows < 0 || newRows < 0 || newCols < 0)
        return -2;

    if (newRows * newCols > nRows * nCols)
        return -3;

    if (!nCols || !nRows || !nRows || !newCols)
        return 0;

#if 0
  for (i = 0; i < 6; i++)
    printf("IN[%d] = %e\n", i, in[i]);
#endif

    for (cur_index = 0; cur_index < newRows * newCols; cur_index++) {
        out[(cur_index % newRows) * newCols + (cur_index / newRows)] =
            in[(cur_index % nRows) * nCols + (cur_index / nRows)];
    }

#if 0
  for (i = 0; i < 6; i++)
    printf("OUT[%d] = %e\n", i, out[i]);
#endif

    return 0;
}

int __subtract(fltPixel_t *inA, fltPixel_t *inB, int nRows, int nCols, fltPixel_t *out)
{
    int x, y;

    if (!inA || !inB || !out)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            out[y * nCols + x] = inA[y * nCols + x] - inB[y * nCols + x];
        }
    }

    return 0;
}

int __add(fltPixel_t *inA, fltPixel_t *inB, int nRows, int nCols, fltPixel_t *out)
{
    int x, y;

    if (!inA || !inB || !out)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

#if 0
  for (j = 0; j < 6; j++)
    printf("inA[%d] = %e\n", j, inA[j]);

  for (j = 0; j < 6; j++)
    printf("inB[%d] = %e\n", j, inB[j]);
#endif

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            out[y * nCols + x] = inA[y * nCols + x] + inB[y * nCols + x];
        }
    }

#if 0
  for (j = 0; j < 6; j++)
    printf("out[%d] = %e\n", j, out[j]);
#endif

    return 0;
}

/* A [nRows x nCommon] * B [nCommon x nCols] -> out[nRows x nCols] */
int __mult(fltPixel_t *inA, fltPixel_t *inB, int nRows, int nCols, int nCommon, fltPixel_t *out)
{
    int x, y, i;
#if 0
  for (j = 0; j < 36; j++)
    printf("inA[%d] = %e\n", j, inA[j]);

  for (j = 0; j < 6; j++)
    printf("inB[%d] = %e\n", j, inB[j]);
#endif

    if (!inA || !inB || !out)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            out[y * nCols + x] = 0;
        }
    }

    /* most trivial implementation */
    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            for (i = 0; i < nCommon; i++) {
#if 0
        printf("TB-tmp1 = %e\n", inA[y * nCommon + i]);
        printf("TB-tmp2 = %e\n", inB[i * nCols + x]);
        printf("TB-tmp3 = %e\n", inA[y * nCommon + i] * inB[i * nCols + x]);
        printf("TB-tmp4 = %e\n", out[y * nCols + x]);
        printf("TB-tmp5 = %e\n", out[y * nCols + x] + inA[y * nCommon + i] * inB[i * nCols + x]);
#endif
                out[y * nCols + x] += inA[y * nCommon + i] * inB[i * nCols + x];
            }
        }
    }

    return 0;
}

int __swap_row(fltPixel_t *data, int nRows, int nCols, int r1, int r2)
{
    int x;

    if (!data)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (r1 > nRows || r2 > nRows)
        return -3;

    if (!nCols || !nRows)
        return 0;

    for (x = 0; x < nCols; x++) {
        float tmp            = data[r1 * nCols + x];
        data[r1 * nCols + x] = data[r2 * nCols + x];
        data[r2 * nCols + x] = tmp;
    }

    return 0;
}

int __scale_row(fltPixel_t *data, int nRows, int nCols, int r, float scale)
{
    int x;

    if (!data)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (r > nRows)
        return -3;

    if (!nCols || !nRows)
        return 0;

    for (x = 0; x < nCols; x++) {
        data[r * nCols + x] *= scale;
    }

    return 0;
}

int __scale_and_add_row(fltPixel_t *data, int nRows, int nCols, int r1, int r2, float scale)
{
    int x;

    if (!data)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (r1 > nRows || r2 > nRows)
        return -3;

    if (!nCols || !nRows)
        return 0;

    for (x = 0; x < nCols; x++) {
        data[r1 * nCols + x] += scale * data[r2 * nCols + x];
    }

    return 0;
}

int __invert_gauss_jordan(fltPixel_t *data, fltPixel_t *workspace, int nRows, int nCols, fltPixel_t *inverse)
{
    int x, y;
    int r, c;

    if (!data || !inverse)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    /* set inverse to identity */
    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            inverse[y * nCols + x]   = (x == y) ? 1.0 : 0.0;
            workspace[y * nCols + x] = data[y * nCols + x];
        }
    }

    /* for each column */
    for (c = 0; c < nCols; c++) {
        float scale = 1.0;

        /* swap rows if close to zero */
        if (CLOSE_TO_ZERO(workspace[c * nCols + c])) {
            for (r = c + 1; r < nCols; r++) {
                if (!CLOSE_TO_ZERO(workspace[r * nCols + c])) {
                    __swap_row(workspace, nRows, nCols, r, c);
                    __swap_row(inverse, nRows, nCols, r, c);
                    break;
                }
            }
            if (r >= nCols)
                return -3;
        }

        /* scale operation */
        scale = 1.0f / workspace[c * nCols + c];
        __scale_row(workspace, nRows, nCols, c, scale);
        __scale_row(inverse, nRows, nCols, c, scale);

        /**
         * THIS IS A BUG!!!
         *  If floating-point precision is used the code is an "identity-function".
         *  It scales the rows of 1: (1/CONST)/CONST = 1.
         *  If you port to fixed-point precision it introduces an approximation.
         *
         * scale = 1.0f / workspace[c * nCols + c];
         * __scale_row(workspace, nRows, nCols, c, scale);
         * __scale_row(inverse, nRows, nCols, c, scale);
         */

        /* zero column */
        for (r = 0; r < nRows; r++) {
            if (r != c) {
                scale = -workspace[r * nCols + c];
                __scale_and_add_row(workspace, nRows, nCols, r, c, scale);
                __scale_and_add_row(inverse, nRows, nCols, r, c, scale);
            }
        }
    }

    return 0;
}

int print_matrix(fltPixel_t *mat, int nRows, int nCols)
{
    int x, y;

    if (!mat)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    printf("[");
    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            printf(" %8e ", mat[y * nCols + x]);
        }
        printf(y == (nRows - 1) ? "]\n" : ";\n");
    }

    return 0;
}

int print_submatrix(fltPixel_t *mat, int nRows, int nCols, int width, int height)
{
    int x, y;

    if (!mat)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    printf("[\n");
    for (y = 0; y < width; y++) {
        for (x = 0; x < height; x++) {
            printf(" %8e ", mat[y * nCols + x]);
        }
        printf(";\n");
    }
    printf("]\n");

    return 0;
}

int print_submatrix_uint16(uint16_t *mat, int nRows, int nCols, int width, int height)
{
    int x, y;

    if (!mat)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    printf("[\n");
    for (y = 0; y < width; y++) {
        for (x = 0; x < height; x++) {
            printf(" %d ", (int)mat[y * nCols + x]);
        }
        printf(";\n");
    }
    printf("]\n");

    return 0;
}

int print_submatrix_uint8_file(FILE *fp, char *name, uint8_t *mat, int nRows, int nCols, int width, int height)
{
    int x, y;

    if (!mat || !fp)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    if (name) {
        fprintf(fp, "%s = [\n", name);
    } else {
        fprintf(fp, "[\n");
    }
    for (y = 0; y < width; y++) {
        for (x = 0; x < height; x++) {
            fprintf(fp, " %d ", (int)mat[y * nCols + x]);
        }
        fprintf(fp, ";\n");
    }
    fprintf(fp, "];\n");

    return 0;
}

int print_submatrix_rgb_file(FILE *fp, char *name, rgb_pixel_t *mat, int nRows, int nCols, int width, int height)
{
    int x, y;

    if (!mat || !fp)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    if (name) {
        fprintf(fp, "%s = [\n", name);
    } else {
        fprintf(fp, "[\n");
    }
    for (y = 0; y < width; y++) {
        for (x = 0; x < height; x++) {
            fprintf(fp, " (%d,%d,%d) ", (int)mat[y * nCols + x].r, (int)mat[y * nCols + x].g,
                    (int)mat[y * nCols + x].b);
        }
        fprintf(fp, ";\n");
    }
    fprintf(fp, "];\n");

    return 0;
}

int print_submatrix_file(FILE *fp, char *name, fltPixel_t *mat, int nRows, int nCols, int width, int height)
{
    int x, y;

    if (!mat || !fp)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    if (name) {
        fprintf(fp, "%s = [\n", name);
    } else {
        fprintf(fp, "[\n");
    }
    for (y = 0; y < width; y++) {
        for (x = 0; x < height; x++) {
            fprintf(fp, " %8e ", (float)mat[y * nCols + x]);
        }
        fprintf(fp, ";\n");
    }
    fprintf(fp, "];\n");

    return 0;
}

int print_submatrix_rgb(rgb_pixel_t *mat, int nRows, int nCols, int width, int height, int channel)
{
    int x, y;

    if (!mat)
        return -1;

    if (nCols < 0 || nRows < 0)
        return -2;

    if (!nCols || !nRows)
        return 0;

    printf("[\n");
    switch (channel) {
        case 0:
            for (y = 0; y < width; y++) {
                for (x = 0; x < height; x++) {
                    printf(" %d ", (int)mat[y * nCols + x].r);
                }
                printf(";\n");
            }
            break;

        default:
        case 1:
            for (y = 0; y < width; y++) {
                for (x = 0; x < height; x++) {
                    printf(" %d ", (int)mat[y * nCols + x].g);
                }
                printf(";\n");
            }
            break;

        case 2:
            for (y = 0; y < width; y++) {
                for (x = 0; x < height; x++) {
                    printf(" %d ", (int)mat[y * nCols + x].b);
                }
                printf(";\n");
            }
            break;
    }
    printf("]\n");

    return 0;
}

#ifdef MATRIX_OPS_TEST
int main(int argc, char **argv)
{
    float input[]      = {1, 1, 1, 0, 1, 1, 0, 0, 1};
    float input_copy[] = {1, 1, 1, 0, 1, 1, 0, 0, 1};
    float inverse[9];
    float result[9];

    printf("Inverse of:\n");
    print_matrix(input, 3, 3);
    invert_gauss_jordan(input_copy, 3, 3, inverse);
    printf("is:\n");
    print_matrix(inverse, 3, 3);
    mult(input, inverse, 3, 3, 3, result);
    printf("multiplied:\n");
    print_matrix(result, 3, 3);
    add(input, inverse, 3, 3, result);
    printf("added:\n");
    print_matrix(result, 3, 3);
    subtract(input, inverse, 3, 3, result);
    printf("subtracted:\n");
    print_matrix(result, 3, 3);

    float mata[] = {1, 1, 1, 0, 1, 1, 0, 0, 1};
    float matb[] = {1, 1, 1, 0, 1, 1};
    float matc[] = {0, 0, 0, 0, 0, 0};

    printf("mata is:\n");
    print_matrix(mata, 3, 3);
    printf("matb is:\n");
    print_matrix(matb, 3, 2);
    mult(mata, matb, 3, 2, 3, matc);
    printf("matc is:\n");
    print_matrix(matc, 3, 2);
}
#endif


#endif /* _WAMI_MATRIX_OPS_HPP_ */
