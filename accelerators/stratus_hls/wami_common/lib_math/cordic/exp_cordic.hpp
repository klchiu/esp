#ifndef __EXP_CORDIC_HPP__
#define __EXP_CORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp"
#include "hcordic.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
sc_fixed<L, W> exp_cordic_func(sc_fixed<L, W> z_in)
{
    DEFINE_INV_AH_TABLE;
    sc_fixed<L, W> x_res, y_res, z_res;
    hcordic_func<N, L, W>("10", inv_ah_table[(N < INV_A_N) ? (N) : (INV_A_N - 1)], 0, z_in, x_res, y_res, z_res);
    return x_res + y_res;
}

#endif // __EXP_CORDIC_HPP__
