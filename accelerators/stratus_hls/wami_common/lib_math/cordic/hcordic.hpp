#ifndef __HCORDIC_HPP__
#define __HCORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp"

template <unsigned char N, unsigned int L, unsigned int W>
void hcordic_func(sc_bv<2> mode, sc_fixed<L, W> x, sc_fixed<L, W> y,
                  sc_fixed<L, W> z, sc_fixed<L, W> &x_out, sc_fixed<L, W> &y_out,
                  sc_fixed<L, W> &z_out)
{
    DEFINE_ATANH_TABLE;

    // This provides the recursive calculation of the repeated iterations
    // The actual algorithm specifies indexs starting at 1, therefore
    // calculation of i and c are all offset by -1.
    // The common notation is to start at index = 1, and first c = 4,
    // and the subsequent c_next = 3 * c + 1
    // Instead we start c = 3, and c_next = 3 * (c + 1).
    sc_uint<6> c = 3;

    sc_fixed<L, W> x_shift, y_shift;

HCORDIC_LOOP:
    for (int i = 0; i < N; i++) {
        x_shift = (x >> i) >> 1;
        y_shift = (y >> i) >> 1;

        if ((mode[0] == true && y < 0) || (mode[1] == true && z >= 0)) {
            x = x + y_shift;
            y = y + x_shift;
            z = z - atanh_table[i];
        } else {
            x = x - y_shift;
            y = y - x_shift;
            z = z + atanh_table[i];
        }

        if (c.to_uint() == i) {
            c = 3 * (c + 1);
            if ((mode[0] == true && y < 0) || (mode[1] == true && z >= 0)) {
                x = x + y_shift;
                y = y + x_shift;
                z = z - atanh_table[i];
            } else {
                x = x - y_shift;
                y = y - x_shift;
                z = z + atanh_table[i];
            }
        }
    }

    x_out = x;
    y_out = y;
    z_out = z;
}

#endif // __HCORDIC_HPP__
