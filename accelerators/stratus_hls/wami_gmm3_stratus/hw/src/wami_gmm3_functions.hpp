// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_GMM3_FUNCTIONS_HPP__
#define __WAMI_GMM3_FUNCTIONS_HPP__

#include "wami_gmm3.hpp"

#include "../../../wami_common/lib_math/div_func.hpp"
#include "../../../wami_common/lib_math/sqrt_func.hpp"
#include "../../../wami_common/lib_math/cordic/exp_cordic.hpp"


FPDATA helper_abs(FPDATA x)
{
    FPDATA ret;
    if (x < FPDATA(0.0)) {
        ret = x * FPDATA(-1.0);
    } else {
        ret = x;
    }
    return ret;
}


void wami_gmm3::kernel_gmm(uint32_t nRows, uint32_t nCols, uint32_t nModels)
{
    size_t num_pixels = nRows * nCols;
    int    row, col, k, num_foreground = 0;

    FPDATA STDEV_THRESH         = FPDATA(2.5);
    FPDATA INIT_STDEV           = FPDATA(80.0);
    FPDATA alpha                = FPDATA(0.1); /* Learning rate */
    FPDATA INIT_WEIGHT          = FPDATA(0.01);
    FPDATA BACKGROUND_THRESH    = FPDATA(0.9);
    FPDATA ONE_OVER_SQRT_TWO_PI = FPDATA(0.398942280401433); // memset(foreground, 0, sizeof(uint8_t) * num_pixels);

    // for (k = 0; k < 20; k++) {
    //     plm_B0_foreground[k] = plm_A0_sigma[k];
    // }

    // FPDATA tempA = FPDATA(300.33);
    // FPDATA tempB = FPDATA(2.354);
    // FPDATA tempC = FPDATA(-44.3);
    // FPDATA tempD = helper_abs(tempC);

    // fprintf(stderr, "tempA = %f\n", (float)tempA);
    // fprintf(stderr, "tempB = %f\n", (float)tempB);
    // fprintf(stderr, "tempC = %f\n", (float)tempC);
    // fprintf(stderr, "tempD = %f\n", (float)tempD);

    // fprintf(stderr, "debug -- nRows = %d, nCols = %d, nModels = %d\n", nRows, nCols, nModels);

    for (row = 0; row < nRows; ++row) {
        for (col = 0; col < nCols; ++col) {

            const uint16_t pixel_temp      = plm_A0_gmm_img[row * nCols + col];
            FPDATA         pixel           = FPDATA(pixel_temp);
            int            match           = -1;
            FPDATA         sum             = FPDATA(0.0);
            FPDATA         norm            = FPDATA(0.0);
            int            sorted_position = 0;
            int            index_c         = row * nCols * nModels + col * nModels;

            for (k = 0; k < nModels; ++k) {
                FPDATA temp_mu    = int2fp<FPDATA, FPDATA_WL>(plm_A0_mu[index_c + k]);
                FPDATA temp_sigma = int2fp<FPDATA, FPDATA_WL>(plm_A0_sigma[index_c + k]);
                FPDATA temp0      = pixel - temp_mu;
                FPDATA temp1      = temp_sigma * STDEV_THRESH;

                // fprintf(stderr, "=============== %d, %d\n", index_c, row * nCols * nModels + col * nModels);

                // fprintf(stderr, "pixel = %f\n", (float)pixel);
                // fprintf(stderr, "temp_mu = %f\n", (float)temp_mu);
                // fprintf(stderr, "temp_sigma = %f\n", (float)temp_sigma);
                // fprintf(stderr, "temp0 = %f\n", (float)temp0);
                // fprintf(stderr, "temp1 = %f\n", (float)temp1);

                if (helper_abs(temp0) < temp1) {
                    match = k;

                    // fprintf(stderr, "debug -- index_c = %d, k = %d\n", row * nCols * nModels + col * nModels, k);
                    // fprintf(stderr, "need help: temp0 = %f, temp1 = %f\n", (float)temp0, (float)temp1);
                    break;
                }
                // fprintf(stderr, "NO help: temp0 = %f, temp1 = %f\n", (float)temp0, (float)temp1);
                // fprintf(stderr, "debug -- match = %d, k = %d, nModels = %d\n", match, k, nModels);
            }

            /* Update the weights for all models */
            for (k = 0; k < nModels; ++k) {
                if (k == match) {
                    FPDATA temp_weight = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + match]);
                    /* A model matched, so update its corresponding weight. */
                    temp_weight += alpha * (FPDATA(1.0) - temp_weight);
                } else {
                    FPDATA temp_weight = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + k]);
                    /* Non-matching models have their weights reduced */
                    temp_weight *= (FPDATA(1.0) - alpha);
                }
            }

            // fprintf(stderr, "[kernle_gmm]: debug 1\n");

            if (match < 0) {
                /*
                 * No distribution matched; replace the least likely distribution.
                 * We keep the entries sorted by significance, so the last entry
                 * is also the least likely.  We do this after updating weights
                 * above so that the initial weight is not immediately down-weighted,
                 * although that means that one update above was wasted. That
                 * update could be avoided.
                 */
                FPDATA_WORD temp_mu                  = fp2bv<FPDATA, FPDATA_WL>(pixel);
                FPDATA_WORD temp_sigma               = fp2bv<FPDATA, FPDATA_WL>(INIT_STDEV);
                FPDATA_WORD temp_weight              = fp2bv<FPDATA, FPDATA_WL>(INIT_WEIGHT);
                plm_A0_mu[index_c + nModels - 1]     = temp_mu;
                plm_A0_sigma[index_c + nModels - 1]  = temp_sigma;
                plm_A0_weight[index_c + nModels - 1] = temp_weight;
            }
            // fprintf(stderr, "[kernle_gmm]: debug 2, nModels = %d\n", nModels);
            /* Normalize weights */
            for (k = 0; k < nModels; ++k) {
                FPDATA temp_weight = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + k]);
                sum                = sum + temp_weight;
                // fprintf(stderr, "[kernle_gmm]: debug 3, sum = %f, weight = %f\n", (float)sum, (float)temp_weight);
            }
            // assert(sum != 0.0f);

            // if (sum == FPDATA(0.0)) {
            //     ESP_REPORT_ERROR("!!!!! sum = 0");
            // }

            // norm = FPDATA(1.0) / sum;
            // norm = FPDATA(1.0);
            FPDATA one = FPDATA(1.0);
            norm       = div_func<FPDATA_WL, FPDATA_IL, FPDATA_WL, FPDATA_IL, FPDATA_WL, FPDATA_IL>(one, sum);

            for (k = 0; k < nModels; ++k) {
                FPDATA temp_weight = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + k]);
                temp_weight *= norm;
                FPDATA_WORD temp_weight2   = fp2bv<FPDATA, FPDATA_WL>(temp_weight);
                plm_A0_weight[index_c + k] = temp_weight2;
            }

            // if(plm_A0_weight[index_c + k] != 1){
            //     fprintf(stderr, "weight[%d] = %d\n", index_c + k, plm_A0_weight[index_c + k]);
            // }
            // fprintf(stderr, "[kernle_gmm]: debug 5, norm = %f\n", (float)norm);

            /* Update mu and sigma for the matched distribution, if any */
            if (match >= 0) {
                FPDATA mu_k    = int2fp<FPDATA, FPDATA_WL>(plm_A0_mu[index_c + match]);
                FPDATA sigma_k = int2fp<FPDATA, FPDATA_WL>(plm_A0_sigma[index_c + match]);
                // if (sigma_k == FPDATA(0.0)) {
                //     ESP_REPORT_ERROR("!!!!! sigma_k = 0");
                // }

                // cout << "---in bin = " << sigma_k.to_bin() << "\tsigma_k = " << sigma_k << endl;
                // cout << "---in bin = "  << "\tPLM_sigma_k = " << plm_A0_sigma[index_c + match] << endl;
                sc_fixed<FPDATA_WL, FPDATA_IL, SC_RND> x = sc_fixed<FPDATA_WL, FPDATA_IL, SC_RND>(1.0);
                sc_fixed<FPDATA_WL, FPDATA_IL, SC_RND> y = sigma_k;
                sc_fixed<FPDATA_WL, FPDATA_IL, SC_RND> q;
                q                  = div_func<FPDATA_WL, FPDATA_IL, FPDATA_WL, FPDATA_IL, FPDATA_WL, FPDATA_IL>(x, y);
                FPDATA sigma_k_inv = q;

                // fprintf(stderr, "sigma_k = %f\n", (float)sigma_k);
                // fprintf(stderr, "sigma_k_inv = %f\n", (float)sigma_k_inv);

                /*
                 * C89 does not include a single-precision expf() exponential function,
                 * so we instead use the double-precision variant exp(). Same for sqrt()
                 * below.
                 */

                // [humu]: check how to deal with exp and sqrt
                // FPDATA rho = alpha * (ONE_OVER_SQRT_TWO_PI * sigma_k_inv) *
                //              exp(FPDATA(-1.0) * (pixel - mu_k) * (pixel - mu_k) / (FPDATA(2.0) * sigma_k * sigma_k));
                FPDATA rho = alpha * (ONE_OVER_SQRT_TWO_PI * sigma_k_inv);

                FPDATA_WORD temp_mu        = fp2bv<FPDATA, FPDATA_WL>((FPDATA(1.0) - rho) * mu_k + rho * pixel);
                plm_A0_mu[index_c + match] = temp_mu;

                FPDATA temp_mu_match = int2fp<FPDATA, FPDATA_WL>(plm_A0_mu[index_c + match]);

                FPDATA temp_match = sqrt((FPDATA(1.0) - rho) * sigma_k * sigma_k +
                                         rho * (pixel - temp_mu_match) * (pixel - temp_mu_match));
                // FPDATA temp_match = rho * (pixel - temp_mu_match) * (pixel - temp_mu_match);

                plm_A0_sigma[index_c + match] = fp2bv<FPDATA, FPDATA_WL>(temp_match);

                // assert(plm_A0_sigma[index_c + match] > 0);
            }

            /*
             * weight and sigma for the matched (or new) distribution are the only
             * values that may have changed, so we find the correct location of that
             * new value in the sorted list.  Matches lead to more evidence, and thus
             * higher weight and lower sigma, so we only need to sort "higher".
             */
            sorted_position = 0;
            if (match != 0) {
                int    sort_from             = (match >= 0) ? match : nModels - 1;
                FPDATA temp_weight_sort_from = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + sort_from]);
                FPDATA temp_sigma_sort_from  = int2fp<FPDATA, FPDATA_WL>(plm_A0_sigma[index_c + sort_from]);

                // if (temp_sigma_sort_from == FPDATA(0.0)) {
                //     ESP_REPORT_ERROR("!!!!! temp_sigma_sort_from = 0");
                // }

                // FPDATA new_significance = temp_weight_sort_from / temp_sigma_sort_from;
                FPDATA new_significance = temp_weight_sort_from;

                FPDATA other_significance;

                // fprintf(stderr, "[kernle_gmm]: debug 6, new_significance = %f\n", (float)new_significance);

                for (k = sort_from - 1; k >= 0; --k) {
                    FPDATA temp_weight_k = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + k]);
                    FPDATA temp_sigma_k  = int2fp<FPDATA, FPDATA_WL>(plm_A0_sigma[index_c + k]);

                    if (temp_sigma_k == FPDATA(0.0)) {
                        temp_sigma_k = FPDATA(0.005434); // [humu]: check how to avoid divided by 0
                        // ESP_REPORT_ERROR("!!!!! temp_sigma_k = 0");
                    }

                    // other_significance = temp_weight_k / temp_sigma_k;
                    other_significance = temp_weight_k;

                    if (new_significance <= other_significance) {
                        break;
                    }
                }
                // fprintf(stderr, "[kernle_gmm]: debug 7, other_significance = %f\n", (float)other_significance);

                if (k == 0) {
                    if (other_significance >= new_significance) {
                        sorted_position = 1;
                    } else {
                        sorted_position = 0;
                    }
                } else {
                    sorted_position = k + 1;
                }
                // fprintf(stderr, "[kernle_gmm]: debug 8\n");

                for (k = sort_from; k > sorted_position; --k) {
                    plm_A0_mu[index_c + k]     = plm_A0_mu[index_c + k - 1];
                    plm_A0_sigma[index_c + k]  = plm_A0_sigma[index_c + k - 1];
                    plm_A0_weight[index_c + k] = plm_A0_weight[index_c + k - 1];
                }
                plm_A0_mu[index_c + sorted_position]     = plm_A0_mu[index_c + sort_from];
                plm_A0_sigma[index_c + sorted_position]  = plm_A0_sigma[index_c + sort_from];
                plm_A0_weight[index_c + sorted_position] = plm_A0_weight[index_c + sort_from];
            }
            // fprintf(stderr, "[kernle_gmm]: debug 9\n");

            /* Now, we need to determine if this pixel is foreground or background. */
            {
                FPDATA cumsum = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + 0]);
                int    B      = 0;

                while (B < nModels - 1 && cumsum <= BACKGROUND_THRESH) {
                    FPDATA temp_weight_B = int2fp<FPDATA, FPDATA_WL>(plm_A0_weight[index_c + ++B]);
                    cumsum += temp_weight_B;
                }
                plm_B0_foreground[row * nCols + col] = (sorted_position > B);
                num_foreground += plm_B0_foreground[row * nCols + col];
            }
            // fprintf(stderr, "[kernle_gmm]: debug 10\n");
        }
    }

    // return num_foreground;
}

#endif // __WAMI_GMM3_FUNCTIONS_HPP__
