#ifndef __ACOS_CORDIC_HPP__
#define __ACOS_CORDIC_HPP__
/*
 * arccos function implemented with CORDIC method.
 * Expected input: -1 < x < 1
 * Output: 0 < y < pi
 * Output error is expected to increase as you approach the outer limits of range
 */

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "cordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> acos_cordic_func(sc_fixed<L, W> c)
{
    sc_fixed<L, W> x_res, y_res, z_res;
    bool                       is_neg = c < 0;

    if (is_neg) {
        c = -c;
    }
    if (c != 1) {
        cordic_constangle_x_func<L, W>(x_res, y_res, z_res, c);
    } else {
        z_res = 0;
    }

    if (is_neg) {
        z_res = SC_FIXED_PI - z_res;
    }
    return z_res;
}

#endif // __ACOS_CORDIC_HPP__
