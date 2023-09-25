#ifndef __CORDIC_HPP__
#define __CORDIC_HPP__

#include "esp_templates.hpp"

#include "cordic_consts.hpp" // This defines several macros for tables that are needed for CtoS to synthesize

// This implements the standard CORDIC algorithm
// c is the target angle, c is assigned 0 in standard CORDIC
template <unsigned char N, unsigned int L, unsigned int W>
void cordic_func(sc_bv<2> mode, sc_fixed<L, W> x, sc_fixed<L, W> y,
                 sc_fixed<L, W> z, sc_fixed<L, W> &x_out, sc_fixed<L, W> &y_out,
                 sc_fixed<L, W> &z_out)
{
    DEFINE_ATAN_TABLE;

    sc_fixed<L, W> x_shift, y_shift;

CORDIC_LOOP:
    for (int i = 0; i < N; i++) {
        x_shift = x >> i;
        y_shift = y >> i;

        if ((mode[0] == true && y < 0) || (mode[1] == true && z >= 0)) {
            x = x - y_shift;
            y = y + x_shift;
            z = z - atan_table[i];
        } else {
            x = x + y_shift;
            y = y - x_shift;
            z = z + atan_table[i];
        }
    }

    x_out = x;
    y_out = y;
    z_out = z;
}

// This implements a specialized CORDIC algorithm for computing
// target is compared against x, z accumulates the acos(c) result with constant grandularity
// For now, the number of iterations N is fixed so that the Xp lookup table can be
// fixed. The entire Xp lookup table is dependent on N.
// Valid input range is [0,1]
//
// an input of c=1 will not trigger the optimized loop triggered by the
// 'reduce_stage' ftriggered by the
// 'reduce_stage' flag of this function.
template <unsigned int L, unsigned int W>
void cordic_constangle_x_func(sc_fixed<L, W> &x_out, sc_fixed<L, W> &y_out,
                              sc_fixed<L, W> &z_out, sc_fixed<L, W> c)
{
    DEFINE_ATAN_TABLE;
    DEFINE_CONSTANGLE_INIT_TABLES;
    DEFINE_XP_N24_CONSTANGLE_TABLE;
    // If N is changed, then the xp table must be recalculated.
    const unsigned int                   N  = 24;
    const sc_ufixed<64, 24> *xp = xp_n24_constangle_table;

    bool reduce_stage = true;

    bool sigma; // false: sigma = -1, true: sigma = 1

    sc_fixed<L, W>  x, y, z;
    sc_fixed<L, W>  P, U, G, H, S, v;
    sc_ufixed<L, 0> T;
    sc_fixed<L, W>  x_shift, y_shift, y_prev;

    x = x_init_constangle_table[(N <= X_INIT_CONSTANGLE_N) ? (N - 1) : (X_INIT_CONSTANGLE_N - 1)];
    y = y_init_constangle_table[(N <= Y_INIT_CONSTANGLE_N) ? (N - 1) : (Y_INIT_CONSTANGLE_N - 1)];
    z = z_init_constangle_table[(N <= Z_INIT_CONSTANGLE_N) ? (N - 1) : (Z_INIT_CONSTANGLE_N - 1)];

    P     = c >> 2;
    U     = c >> 5;
    G     = -(c * sc_fixed<32, 1>(0.34375));
    H     = -(c * sc_fixed<32, 1>(.058594));
    T     = c;
    S     = (c << 1);
    sigma = false;

    for (unsigned int i = 1; i <= N; i++) {
        //////////////////////////
        // Sigma Equations      //
        //////////////////////////

        if (reduce_stage) {
            if (T[L - 1] == 1 && T[L - 2] == 1 && i <= N) {
                sigma = false;
            } else {
                reduce_stage = false;
                v            = xp[i - 1] - S + P;
                sigma        = (v >= 0);
            }
        } else {
            sigma = (v >= 0);
        }

        //////////////////////////
        // CORDIC Equations     //
        //////////////////////////

        x_shift = x >> i;
        y_shift = y >> i;
        y_prev  = y;

        if (sigma) {
            x = x - y_shift;
            y = y + x_shift;
            z = z + atan_table[i];

        } else {
            x = x + y_shift;
            y = y - x_shift;
            z = z - atan_table[i];
        }

        //////////////////////////
        // Recurrance Equations //
        //////////////////////////

        if (reduce_stage) {
            if (i <= N >> 1) {
                P = (P >> 1) + U;
            } else {
                P = P >> 1;
            }

            U = U >> 3;
            T = T << 2;
            S = S << 1;
        } else {
            if (sigma) {
                v = (v << 1) - (y_prev << 1) + G;
            } else {
                v = (v << 1) + (y_prev << 1) + G;
            }
        }

        if (i <= N >> 1) {
            G = (G >> 1) + H;
        } else {
            G = G >> 1;
        }
        H = H >> 3;
    }

    x_out = x;
    y_out = y;
    z_out = z;
}

#endif // __CORDIC_HPP__
