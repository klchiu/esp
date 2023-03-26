#ifndef __COS_CORDIC_HPP__
#define __COS_CORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "cordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> cos_cordic_func(sc_fixed<L, W> z)
{
    DEFINE_INV_A_TABLE;
    sc_fixed<L, W> x_res, y_res, z_res;
    cordic_func<N, L, W>("10", inv_a_table[(N < INV_A_N) ? (N) : (INV_A_N - 1)], 0, z, x_res, y_res, z_res);
    return x_res;
}

#endif // __COS_CORDIC_HPP__
