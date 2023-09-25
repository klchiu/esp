#ifndef __COS_SIN_CORDIC_HPP__
#define __COS_SIN_CORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "cordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
void cos_sin_cordic_func(sc_fixed<L, W> z, sc_fixed<L, W> &x_res,
                         sc_fixed<L, W> &y_res)
{
    DEFINE_INV_A_TABLE;
    sc_fixed<L, W> z_res;
    cordic_func<N, L, W>("10", inv_a_table[(N < INV_A_N) ? (N) : (INV_A_N - 1)], 0, z, x_res, y_res, z_res);
}

// this version assumes the input is multipled by pi and extends the range
// Since it is periodic, just need to guarantee 1 integer bit
template <unsigned char N, unsigned int L, unsigned int W>
void cos_sin_pi_extend_cordic_func(sc_fixed<L, W, sc_dt::SC_TRN, sc_dt::SC_WRAP> z,
                                   sc_fixed<L, W> &x_res, sc_fixed<L, W> &y_res)
{
    DEFINE_INV_A_TABLE;
    sc_fixed<L, W> x_out, y_out, z_out;

    bool swap    = (z[L - W - 1] == 1);
    bool sin_neg = (z[L - W] == 1);                       // sin result is negative
    bool cos_neg = ((z[L - W] ^ z[L - W - 1]) == 1);      // cos result is negative
    z            = z & ~sc_fixed<L, W>(-0.5); // Mask to perform z modulo 0.5
    z            = z * SC_FIXED_PI;

    cordic_func<N, L, W>("10", inv_a_table[(N < INV_A_N) ? (N) : (INV_A_N - 1)], 0, z, x_out, y_out, z_out);
    if (swap) {
        if (cos_neg) {
            x_res = -y_out;
        } else {
            x_res = y_out;
        }

        if (sin_neg) {
            y_res = -x_out;
        } else {
            y_res = x_out;
        }
    } else {
        if (cos_neg) {
            x_res = -x_out;
        } else {
            x_res = x_out;
        }

        if (sin_neg) {
            y_res = -y_out;
        } else {
            y_res = y_out;
        }
    }
}

#endif // __COS_SIN_CORDIC_HPP__
