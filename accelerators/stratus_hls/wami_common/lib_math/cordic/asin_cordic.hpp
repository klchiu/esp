#ifndef __ASIN_CORDIC_HPP__
#define __ASIN_CORDIC_HPP__

/*
 * arcsin function implemented with CORDIC method.
 * Expected input: -1 < x < 1
 * Output: -pi/2 < y < pi/2
 * Output error is expected to increase as you approach the outer limits of range
 */

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "acos_cordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> asin_cordic_func(sc_fixed<L, W> c)
{
    sc_fixed<L, W> z_res;
    z_res = acos_cordic_func<N, L, W>(c);
    return (SC_FIXED_PI >> 1) - z_res;
}

#endif // __ASIN_CORDIC_HPP__
