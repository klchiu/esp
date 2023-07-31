#ifndef __ATAN2_CORDIC_HPP__
#define __ATAN2_CORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "cordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> atan2_cordic_func(sc_fixed<L, W> x, sc_fixed<L, W> y)
{
    sc_fixed<L, W> x_res, y_res, z_res;

    cordic_func<N, L, W>("01", x, y, 0, x_res, y_res, z_res);

    return z_res;
}

#endif // __ATAN2_CORDIC_HPP__
