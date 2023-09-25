#ifndef _WAMI_GMM_PV_HPP_
#define _WAMI_GMM_PV_HPP_

#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>

#include "../wami_utils_forpv.hpp"

#include "../wami_params_forpv.hpp"

#if 0
void wami_gmm(
    uint8_t foreground[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS],
    float mu[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    float sigma[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    float weights[WAMI_GMM_IMG_NUM_ROWS][WAMI_GMM_IMG_NUM_COLS][WAMI_GMM_NUM_MODELS],
    uint16_t (* const frames)[WAMI_GMM_IMG_NUM_COLS]);
#endif

int __wami_gmm(int nRows, int nCols, int nModels, float *mu, float *sigma, float *weight, uint8_t *foreground,
               uint16_t *frame);



static const float STDEV_THRESH      = 2.5f;
static const float INIT_STDEV        = 80.0f;
static const float alpha             = 0.1f; /* Learning rate */
static const float INIT_WEIGHT       = 0.01f;
static const float BACKGROUND_THRESH = 0.9f;
#define ONE_OVER_SQRT_TWO_PI (1.0f / sqrt(2.0f * M_PI))

int __wami_gmm(int nRows, int nCols, int nModels, float *mu, float *sigma, float *weight, uint8_t *foreground,
               uint16_t *frame)
{
    size_t num_pixels = nRows * nCols;
    int    row, col, k, num_foreground = 0;

    memset(foreground, 0, sizeof(uint8_t) * num_pixels);

    for (row = 0; row < nRows; ++row) {
        for (col = 0; col < nCols; ++col) {
            const uint16_t pixel = frame[row * nCols + col];
            int            match = -1;
            float          sum = 0.0f, norm = 0.0f;
            int            sorted_position = 0;

            for (k = 0; k < nModels; ++k) {
                /*
                 * C89 does not include fabsf(), so using the double-precision
                 * fabs() function will unnecessarily type-convert to double.
                 */
                if (fabs(pixel - mu[row * nCols * nModels + col * nModels + k]) /
                        sigma[row * nCols * nModels + col * nModels + k] <
                    STDEV_THRESH) {
                    match = k;
                    break;
                }
            }

            /* Update the weights for all models */
            for (k = 0; k < nModels; ++k) {
                if (k == match) {
                    /* A model matched, so update its corresponding weight. */
                    weight[row * nCols * nModels + col * nModels + match] +=
                        alpha * (1.0f - weight[row * nCols * nModels + col * nModels + match]);
                } else {
                    /* Non-matching models have their weights reduced */
                    weight[row * nCols * nModels + col * nModels + k] *= (1.0f - alpha);
                }
            }

            if (match < 0) {
                /*
                 * No distribution matched; replace the least likely distribution.
                 * We keep the entries sorted by significance, so the last entry
                 * is also the least likely.  We do this after updating weights
                 * above so that the initial weight is not immediately down-weighted,
                 * although that means that one update above was wasted. That
                 * update could be avoided.
                 */
                mu[row * nCols * nModels + col * nModels + nModels - 1]     = (float)pixel;
                sigma[row * nCols * nModels + col * nModels + nModels - 1]  = INIT_STDEV;
                weight[row * nCols * nModels + col * nModels + nModels - 1] = INIT_WEIGHT;
            }

            /* Normalize weights */
            for (k = 0; k < nModels; ++k) {
                sum += weight[row * nCols * nModels + col * nModels + k];
            }

            assert(sum != 0.0f);

            norm = 1.0f / sum;
            for (k = 0; k < nModels; ++k) {
                weight[row * nCols * nModels + col * nModels + k] *= norm;
            }

            /* Update mu and sigma for the matched distribution, if any */
            if (match >= 0) {
                const float mu_k        = mu[row * nCols * nModels + col * nModels + match];
                const float sigma_k     = sigma[row * nCols * nModels + col * nModels + match];
                const float sigma_k_inv = 1.0f / sigma_k;
                /*
                 * C89 does not include a single-precision expf() exponential function,
                 * so we instead use the double-precision variant exp(). Same for sqrt()
                 * below.
                 */
                const float rho = alpha * (ONE_OVER_SQRT_TWO_PI * sigma_k_inv) *
                                  exp(-1.0f * (pixel - mu_k) * (pixel - mu_k) / (2.0f * sigma_k * sigma_k));
                mu[row * nCols * nModels + col * nModels + match] = (1.0f - rho) * mu_k + rho * pixel;
                sigma[row * nCols * nModels + col * nModels + match] =
                    sqrt((1.0f - rho) * sigma_k * sigma_k +
                         rho * (pixel - mu[row * nCols * nModels + col * nModels + match]) *
                             (pixel - mu[row * nCols * nModels + col * nModels + match]));
                assert(sigma[row * nCols * nModels + col * nModels + match] > 0);
            }

            /*
             * weight and sigma for the matched (or new) distribution are the only
             * values that may have changed, so we find the correct location of that
             * new value in the sorted list.  Matches lead to more evidence, and thus
             * higher weight and lower sigma, so we only need to sort "higher".
             */
            sorted_position = 0;
            if (match != 0) {
                const int   sort_from        = (match >= 0) ? match : nModels - 1;
                const float new_significance = weight[row * nCols * nModels + col * nModels + sort_from] /
                                               sigma[row * nCols * nModels + col * nModels + sort_from];
                float other_significance, new_mu, new_sigma, new_weight;
                for (k = sort_from - 1; k >= 0; --k) {
                    other_significance = weight[row * nCols * nModels + col * nModels + k] /
                                         sigma[row * nCols * nModels + col * nModels + k];
                    if (new_significance <= other_significance) {
                        break;
                    }
                }
                if (k == 0) {
                    if (other_significance >= new_significance) {
                        sorted_position = 1;
                    } else {
                        sorted_position = 0;
                    }
                } else {
                    sorted_position = k + 1;
                }

                new_mu     = mu[row * nCols * nModels + col * nModels + sort_from];
                new_sigma  = sigma[row * nCols * nModels + col * nModels + sort_from];
                new_weight = weight[row * nCols * nModels + col * nModels + sort_from];
                for (k = sort_from; k > sorted_position; --k) {
                    mu[row * nCols * nModels + col * nModels + k] = mu[row * nCols * nModels + col * nModels + k - 1];
                    sigma[row * nCols * nModels + col * nModels + k] =
                        sigma[row * nCols * nModels + col * nModels + k - 1];
                    weight[row * nCols * nModels + col * nModels + k] =
                        weight[row * nCols * nModels + col * nModels + k - 1];
                }
                mu[row * nCols * nModels + col * nModels + sorted_position]     = new_mu;
                sigma[row * nCols * nModels + col * nModels + sorted_position]  = new_sigma;
                weight[row * nCols * nModels + col * nModels + sorted_position] = new_weight;
            }

            /* Now, we need to determine if this pixel is foreground or background. */
            {
                float cumsum = weight[row * nCols * nModels + col * nModels + 0];
                int   B      = 0;
                while (B < nModels - 1 && cumsum <= BACKGROUND_THRESH) {
                    cumsum += weight[row * nCols * nModels + col * nModels + ++B];
                }
                foreground[row * nCols + col] = (sorted_position > B);
                num_foreground += foreground[row * nCols + col];
            }
        }
    }
    return num_foreground;
}


#endif /* _WAMI_GMM_PV_HPP_ */
