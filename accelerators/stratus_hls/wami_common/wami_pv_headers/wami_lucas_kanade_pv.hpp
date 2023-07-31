#ifndef _WAMI_LUCAS_KANADE_PV_HPP_
#define _WAMI_LUCAS_KANADE_PV_HPP_

#include <stdint.h>
#include <math.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>

#include "../wami_params_forpv.hpp"

// #include "timer.h"
#include "wami_matrix_ops.hpp"
#include "wami_gradient_pv.hpp"

typedef int   algPixel_t;
typedef float fltPixel_t;

/* Registers tmplt to img and produces a warped output in warped_tmplt to align
 * the images.  Note that the warp parameters are not zeroed such that you
 * can sart with the previous settings or reinitialize at each step.
 * RETURNS non-zero on error
 */
// int kernel2_lucas_kanade(fltPixel_t *img, fltPixel_t *tmplt, float *affine_warp, int num_iterations, int nRows,
//                          int nCols, fltPixel_t *warped_tmplt);

int __sd_update(fltPixel_t *VI_dW_dp, fltPixel_t *error_img, int N_p, int nRows, int nCols, fltPixel_t *sd_delta_p);

float interpolate(float Tlocalx, float Tlocaly, int nCols, int nRows, fltPixel_t *Iin);

void __warp_image(fltPixel_t *Iin, int nCols, int nRows, float *W_xp, fltPixel_t *Iout);

void __steepest_descent(fltPixel_t *gradX_warped, fltPixel_t *gradY_warped, int nCols, int nRows,
                        fltPixel_t *I_steepest);

void __hessian(fltPixel_t *I_steepest, int nCols, int nRows, int np, float *H);

/* macro for (ideally) clean exits on failure */
#define K2_FAIL_FREE_EXIT() \
    if (error_img)          \
        free(error_img);    \
    if (img_dx)             \
        free(img_dx);       \
    if (img_dy)             \
        free(img_dy);       \
    if (nabla_Ix)           \
        free(nabla_Ix);     \
    if (nabla_Iy)           \
        free(nabla_Iy);     \
    if (I_steepest)         \
        free(I_steepest);   \
    if (sd_delta_p)         \
        free(sd_delta_p);   \
    if (delta_p)            \
        free(delta_p);      \
    if (H)                  \
        free(H);            \
    if (H_wsp)              \
        free(H_wsp);        \
    if (H_inv)              \
        free(H_inv);        \
    return -1;

/* Registers tmplt to img and produces a warped output in warped_tmplt to align
 * the images.  Note that the warp parameters are not zeroed such that you
 * can sart with the previous settings or reinitialize at each step.
 * RETURNS non-zero on error
 */
#if 0
int kernel2_lucas_kanade(fltPixel_t *img, fltPixel_t *tmplt, float *affine_warp, int num_iterations, int nRows,
                         int nCols, fltPixel_t *warped_tmplt)
{
    /* Variables --------------------------------------------------------------- */

    /* Input images */
    fltPixel_t *error_img = NULL;
    /* Gradients of input image */
    fltPixel_t *img_dx = NULL;
    fltPixel_t *img_dy = NULL;
    /* Warped input gradients */
    fltPixel_t *nabla_Ix = NULL;
    fltPixel_t *nabla_Iy = NULL;
    /* Steepest descent */
    fltPixel_t *I_steepest = NULL;
    fltPixel_t *sd_delta_p = NULL;
    fltPixel_t *delta_p    = NULL;
    /* Hessian */
    float *H     = NULL;
    float *H_wsp = NULL;
    float *H_inv = NULL;

    /* Alias for consistency with original matlab code */
    float *IWxp = warped_tmplt;

    int iteration = 0;
    int N_p       = 6;

    PRINT_STAT_STRING("kernel", "lucas_kanade");
    PRINT_STAT_INT("rows", nRows);
    PRINT_STAT_INT("columns", nCols);

    /* Allocate and check ------------------------------------------------------ */
    error_img  = calloc(nRows * nCols, sizeof(fltPixel_t));
    img_dx     = calloc(nRows * nCols, sizeof(fltPixel_t));
    img_dy     = calloc(nRows * nCols, sizeof(fltPixel_t));
    nabla_Ix   = calloc(nRows * nCols, sizeof(fltPixel_t));
    nabla_Iy   = calloc(nRows * nCols, sizeof(fltPixel_t));
    I_steepest = calloc(6 * nRows * nCols, sizeof(fltPixel_t));
    sd_delta_p = calloc(6 * 1, sizeof(fltPixel_t));
    delta_p    = calloc(6 * 1, sizeof(fltPixel_t));
    H          = calloc(6 * 6, sizeof(float));
    H_wsp      = calloc(6 * 6, sizeof(float));
    H_inv      = calloc(6 * 6, sizeof(float));

    if (!error_img || !img_dx || !img_dy || !nabla_Ix || !nabla_Iy || !I_steepest || !sd_delta_p || !delta_p || !H ||
        !H_wsp || !H_inv) {

        fprintf(stderr, "ERROR: Allocation failed.\n");
        K2_FAIL_FREE_EXIT();
    }

    /* Start computation -------------------------------------------------------- */
    /* From here, this will follow the Matlab forward-additive implementation from
      Lucas-Kanade 20 Years On
      Iain Matthews, Simon Baker, Carnegie Mellon University, Pittsburgh
      $Id: affine_fa.m,v 1.1.1.1 2003/08/20 03:07:35 iainm Exp $
    */

    /* Pre-computable things --------------------------------------------------- */

    /* 3a) Compute gradients */
    if (__gradientXY(img, nCols, nRows, img_dx, img_dy)) {
        fprintf(stderr, "ERROR: gradient failed.\n");
        K2_FAIL_FREE_EXIT();
    }

    /* 4) Jacobian (defined in steepest descent step) */
    /* nothing to do...                               */

    /* p - affine parameters */
    /* warp_p[0] =  horizontal compression */
    /* warp_p[1] =  horizontal distortion */
    /* warp_p[2] =  vertical distortion */
    /* warp_p[3] =  vertical compression */
    /* warp_p[4] =  horizontal translation */
    /* warp_p[5] =  vertical translation */

    /* Main loop --------------------------------------------------------------- */
    for (iteration = 0; iteration < num_iterations; iteration++) {
        printf(",\n\t\"warp matrix\": [[%8e, %8e, %8e], [%8e, %8e, %8e]]", affine_warp[0], affine_warp[1],
               affine_warp[2], affine_warp[3], affine_warp[4], affine_warp[5]);

        /* 1) Warp image with current parameters */
        tic();
        __warp_image(img, nRows, nCols, affine_warp, IWxp);

        PRINT_STAT_COUNT_DOUBLE(iteration, "1)time_warp", toc());

        /* 2) compute error image */
        tic();
        if (__subtract(tmplt, IWxp, nRows, nCols, error_img)) {
            fprintf(stderr, "ERROR: Subtraction failed.\n");
            K2_FAIL_FREE_EXIT();
        }
        PRINT_STAT_COUNT_DOUBLE(iteration, "2)time_subtract", toc());

        /* 3b) Warp the gradient I with W(x;p) */
        tic();
        __warp_image(img_dx, nRows, nCols, affine_warp, nabla_Ix);
        __warp_image(img_dy, nRows, nCols, affine_warp, nabla_Iy);
        PRINT_STAT_COUNT_DOUBLE(iteration, "3b)time_warp", toc());

        /* 4) Jacobian and ...
           5) Compute the steepest descent images Gradient * Jacobian */
        tic();
        __steepest_descent(nabla_Ix, nabla_Iy, nRows, nCols, I_steepest);
        PRINT_STAT_COUNT_DOUBLE(iteration, "4+5)time_steepest_descent", toc());

        /* 6) Compute the Hessian matrix and inverse */
        tic();
        __hessian(I_steepest, nRows, nCols, N_p, H);
        __invert_gauss_jordan(H, H_wsp, 6, 6, H_inv);
        PRINT_STAT_COUNT_DOUBLE(iteration, "6)time_hessian", toc());

        /* 7) Update based on steepest descent */
        tic();
        __sd_update(I_steepest, error_img, N_p, nRows, nCols, sd_delta_p);
        PRINT_STAT_COUNT_DOUBLE(iteration, "7)time_sd_update", toc());

        /* 8) compute gradient descent parameter updates */
        tic();
        __mult(H_inv, sd_delta_p, 6, 1, 6, delta_p);
        PRINT_STAT_COUNT_DOUBLE(iteration, "8)time_sd_param_updates", toc());

        /* reuse sd_delta_p to store the reshaped delta_p */
        __reshape(delta_p, 6, 1, 2, 3, sd_delta_p);

        /* 9) update warp parameters */
        tic();
        __add(affine_warp, sd_delta_p, 2, 3, affine_warp);
        PRINT_STAT_COUNT_DOUBLE(iteration, "9)time_warp_param_updates", toc());
    }

    tic();
    __warp_image(img, nRows, nCols, affine_warp, IWxp);
    PRINT_STAT_DOUBLE("final)time_warp", toc());

    if (error_img)
        free(error_img);
    if (img_dx)
        free(img_dx);
    if (img_dy)
        free(img_dy);
    if (nabla_Ix)
        free(nabla_Ix);
    if (nabla_Iy)
        free(nabla_Iy);
    if (I_steepest)
        free(I_steepest);
    if (sd_delta_p)
        free(sd_delta_p);
    if (delta_p)
        free(delta_p);
    if (H)
        free(H);
    if (H_wsp)
        free(H_wsp);
    if (H_inv)
        free(H_inv);

    return 0;
}
#endif

#if 0
int
__sd_update(fltPixel_t * VI_dW_dp, fltPixel_t * error_img, int N_p, int nRows, int nCols, fltPixel_t * sd_delta_p)
{
  int i, x, y;

  if(!VI_dW_dp || !error_img || !sd_delta_p)
    return -1;

  if(N_p < 0 || nRows < 0 || nCols < 0)
    return -2;

  if(N_p == 0 || nRows == 0 || nCols == 0)
    return 0;

  for(i = 0; i < N_p; i++) {
    sd_delta_p[i] = 0.0;
  }

  for(i = 0; i < N_p; i++) {
    for (y = 0; y < nRows; y++) {
      for (x = 0; x < nCols; x++) {
        sd_delta_p[i] += error_img[y * nCols + x] * VI_dW_dp[y * N_p * nCols + (x + i * nCols)];
      }
    }
  }

  return 0;
}
#else
int __sd_update(fltPixel_t *VI_dW_dp, fltPixel_t *error_img, int N_p, int nRows, int nCols, fltPixel_t *sd_delta_p)
{
    int i, x, y;

    if (!VI_dW_dp || !error_img || !sd_delta_p)
        return -1;

    if (N_p < 0 || nRows < 0 || nCols < 0)
        return -2;

    if (N_p == 0 || nRows == 0 || nCols == 0)
        return 0;

    for (i = 0; i < N_p; i++) {
        sd_delta_p[i] = 0.0;
    }

    for (y = 0; y < nRows; y++) {
        for (i = 0; i < N_p; i++) {
            for (x = 0; x < nCols; x++) {
                fltPixel_t tmp = error_img[y * nCols + x] * VI_dW_dp[y * N_p * nCols + (x + i * nCols)];
                sd_delta_p[i] += tmp;
            }
        }
    }

    return 0;
}
#endif

void __warp_image(fltPixel_t *Iin, int nCols, int nRows, float *W_xp, fltPixel_t *Iout)
{
    int   x, y;
    float Tlocalx, Tlocaly;
    float compa0, compa1, compb0, compb1;
    int   index = 0;

#if 0
  printf("==== TB ====\n");
  printf("W_xp[0] = %f\n", W_xp[0]);
  printf("W_xp[1] = %f\n", W_xp[1]);
  printf("W_xp[2] = %f\n", W_xp[2]);
  printf("W_xp[3] = %f\n", W_xp[3]);
  printf("W_xp[4] = %f\n", W_xp[4]);
  printf("W_xp[5] = %f\n", W_xp[5]);
#endif

    compb0 = W_xp[2];
    compb1 = W_xp[5];
#if 0
  printf("compb0 = %f\n", compb0);
  printf("compb1 = %f\n", compb1);
#endif
    for (y = 0; y < nRows; y++) {
        compa0 = W_xp[1] * ((float)y) + compb0;
        compa1 = (1.0 + W_xp[4]) * ((float)y) + compb1;
#if 0
    printf("compa0 = %f\n", compa0); 
    printf("compa1 = %f\n", compa1);
#endif

        for (x = 0; x < nCols; x++) {
            Tlocalx = (1.0 + W_xp[0]) * ((float)x) + compa0;
            Tlocaly = W_xp[3] * ((float)x) + compa1;
#if 0
      printf("Tlocalx = %f\n", Tlocalx); 
      printf("Tlocaly = %f\n", Tlocaly);
#endif
            Iout[index] = interpolate(Tlocalx, Tlocaly, nCols, nRows, Iin);
            index++;
        }
#if 0
    printf("\n");
#endif
    }
}

void __steepest_descent(fltPixel_t *gradX_warped, fltPixel_t *gradY_warped, int nCols, int nRows,
                        fltPixel_t *I_steepest)
{
    int   k;
    int   x, y;
    float Jacobian_x[6], Jacobian_y[6];
    int   index, j_index;

    for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
            index = y * nCols + x;

            Jacobian_x[0] = (float)x;
            Jacobian_x[1] = 0.0;
            Jacobian_x[2] = (float)y;
            Jacobian_x[3] = 0.0;
            Jacobian_x[4] = 1.0;
            Jacobian_x[5] = 0.0;

            Jacobian_y[0] = 0.0;
            Jacobian_y[1] = (float)x;
            Jacobian_y[2] = 0.0;
            Jacobian_y[3] = (float)y;
            Jacobian_y[4] = 0.0;
            Jacobian_y[5] = 1.0;

            for (k = 0; k < 6; k++) {
                j_index             = (6 * y * nCols) + (nCols * k) + x;
                I_steepest[j_index] = (Jacobian_x[k] * gradX_warped[index]) + (Jacobian_y[k] * gradY_warped[index]);
            }
        }
    }
}

#if 0
void
__hessian (fltPixel_t *I_steepest, int nCols, int nRows, int np, float *H)
{
  int i, j;
  int x, y;

  for (i = 0; i < np; i++) {
    for (j = 0; j < np; j++) {
      H[i * np + j] = 0;
    }
  }

  /* compare each image in the 6-wide I_steepest to each other image */
  for (i = 0; i < np; i++) {
    for (j = 0; j < np; j++) {
      /* sum the element-wise product of the images */
      double total = 0.0;
      for (y = 0; y < nRows; y++) {
        for (x = 0; x < nCols; x++) {
	        int index1 = (np * y * nCols) + (nCols * i) + x;
	        int index2 = (np * y * nCols) + (nCols * j) + x;
          total += ((double)I_steepest[index1]) * ((double)I_steepest[index2]);
	      }
      }
      H[np * j + i] = total;
    }
  }
}
#else
void __hessian(fltPixel_t *I_steepest, int nCols, int nRows, int np, float *H)
{
    int i, j;
    int x, y;

    for (i = 0; i < np; i++) {
        for (j = 0; j < np; j++) {
            H[i * np + j] = 0;
        }
    }
    for (y = 0; y < nRows; y++) {
        /* compare each image in the 6-wide I_steepest to each other image */
        for (i = 0; i < np; i++) {
            for (j = 0; j < np; j++) {
                /* sum the element-wise product of the images */
                double total = 0.0;
                for (x = 0; x < nCols; x++) {
                    int index1 = (np * y * nCols) + (nCols * i) + x;
                    int index2 = (np * y * nCols) + (nCols * j) + x;
                    total += ((double)I_steepest[index1]) * ((double)I_steepest[index2]);
                }
                H[np * j + i] += total;
            }
        }
    }
}
#endif

float interpolate(float Tlocalx, float Tlocaly, int nCols, int nRows, fltPixel_t *Iin)
{
    /* Linear interpolation variables */
    int   xBas0, xBas1, yBas0, yBas1;
    float perc[4] = {0, 0, 0, 0};
    float xCom, yCom, xComi, yComi;
    float color[4] = {0, 0, 0, 0};
    float result;
    /* Rounded location */
    float fTlocalx, fTlocaly;

    /* Determine the coordinates of the pixel(s) which will become the current pixel
     * (using linear interpolation) */
    fTlocalx = floor(Tlocalx);
    fTlocaly = floor(Tlocaly);
#if 0  
  printf("floor(Tlocalx) = %f\n", fTlocalx);
  printf("floor(Tlocaly) = %f\n", fTlocaly);
#endif
    xBas0 = (int)fTlocalx;
    yBas0 = (int)fTlocaly;
    xBas1 = xBas0 + 1;
    yBas1 = yBas0 + 1;
#if 0
  printf("xBas1 = %d\n", xBas1);
  printf("yBas1 = %d\n", yBas1);
#endif
    /* Linear interpolation constants (percentages) */
    xCom    = Tlocalx - fTlocalx;
    yCom    = Tlocaly - fTlocaly;
    xComi   = (1 - xCom);
    yComi   = (1 - yCom);
    perc[0] = xComi * yComi;
    perc[1] = xComi * yCom;
    perc[2] = xCom * yComi;
    perc[3] = xCom * yCom;
#if 0
  printf("xCom = %f\n", xCom);
  printf("yCom = %f\n", yCom);
  printf("perc[0] = %f\n", perc[0]);
  printf("perc[1] = %f\n", perc[1]);
  printf("perc[2] = %f\n", perc[2]);
  printf("perc[3] = %f\n", perc[3]);
  printf("xBas0 = %d\n", xBas0);
  printf("yBas0 = %d\n", yBas0);
  printf("xBas1 = %d\n", xBas1);
  printf("yBas1 = %d\n", yBas1);
#endif

    /* if all pixels are outside image, return 0, else in-image pixels are interpolated with 0 */
    if (xBas0 < -1) {
        return 0;
    }

    if (yBas0 < -1) {
        return 0;
    }

    if (xBas1 > nCols) {
        return 0;
    }

    if (yBas1 > nRows) {
        return 0;
    }

    /* sample values from the image, fill with 0 if the sample is just outside the edge */
    color[0] = (yBas0 < 0 || xBas0 < 0) ? 0 : Iin[yBas0 * nCols + xBas0];
    color[1] = (yBas1 > (nRows - 1) || xBas0 < 0) ? 0 : Iin[yBas1 * nCols + xBas0];
    color[2] = (yBas0 < 0 || xBas1 > (nCols - 1)) ? 0 : Iin[yBas0 * nCols + xBas1];
    color[3] = (yBas1 > (nRows - 1) || xBas1 > (nCols - 1)) ? 0 : Iin[yBas1 * nCols + xBas1];
    /* *** THIS WAR S A BUG! *** */
    /* color[3] = (yBas1 > (nRows-1) || xBas1 > (nCols-1)) ? 0 : Iin[yBas1 * nCols + xBas1]; */

#if 0
  printf("[0] = %d\n", yBas0 * nCols + xBas0);
  printf("[1] = %d\n", yBas1 * nCols + xBas0);
  printf("[2] = %d\n", yBas0 * nCols + xBas1);
  printf("[3] = %d\n", yBas1 * nCols + xBas1);

  printf("color[0] = %f\n", color[0]);
  printf("color[1] = %f\n", color[1]);
  printf("color[2] = %f\n", color[2]);
  printf("color[3] = %f\n", color[3]);
#endif
    result = color[0] * perc[0] + color[1] * perc[1] + color[2] * perc[2] + color[3] * perc[3];
#if 0
  printf("result = %f\n", result);
#endif
    return result;
}

#endif /* _WAMI_LUCAS_KANADE_PV_HPP_ */
