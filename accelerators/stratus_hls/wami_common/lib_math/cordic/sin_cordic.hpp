#ifndef __SIN_CORDIC_HPP__
#define __SIN_CORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "cordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> sin_cordic_func(sc_fixed<L, W> z)
{
    DEFINE_INV_A_TABLE;
    sc_fixed<L, W> x_res, y_res, z_res;
    cordic_func<N, L, W>("10", inv_a_table[(N < INV_A_N) ? (N) : (INV_A_N - 1)], 0, z, x_res, y_res, z_res);
    return y_res;
}

// this version assumes the input is multipled by pi and extends the range
template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> sin_pi_extend_cordic_func(sc_fixed<L, W> z)
{
    DEFINE_INV_A_TABLE;
    sc_fixed<L, W> x_res, y_res, z_res;

    bool is_cos = (z[L - W - 1] == 1);
    bool is_neg = (z[L - W] == 1);

    z = z & ~sc_fixed<L, W>(-0.5); // Mask to perform z modulo 0.5
    z = z * SC_FIXED_PI;

    cordic_func<N, L, W>("10", inv_a_table[(N < INV_A_N) ? (N) : (INV_A_N - 1)], 0, z, x_res, y_res, z_res);
    if (is_neg) {
        if (is_cos)
            return -x_res;
        else
            return -y_res;
    } else {
        if (is_cos)
            return x_res;
        else
            return y_res;
    }
}

#endif // __SIN_CORDIC_HPP__
