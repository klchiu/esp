#ifndef __SQRT_FUNC_HPP__
#define __SQRT_FUNC_HPP__

#include "esp_templates.hpp"


#define INV_SQRT_2    0.707106781186547
#define SC_INV_SQRT_2 sc_ufixed<64, 0>(INV_SQRT_2)

template <int IN_W, int IN_I, int OUT_W, int OUT_I> sc_fixed<OUT_W, OUT_I> sqrt_func(sc_fixed<IN_W, IN_I> x)
{
    cout << "in bin = " << x.to_bin() << "\t x = " << x << endl;

    sc_ufixed<IN_W, IN_W>   x_adj;
    sc_ufixed<OUT_W, OUT_W> rem, divisor, root;

    x_adj   = x << (IN_W - IN_I);
    rem     = 0;
    root    = 0;
    divisor = 0;

    // cout << "in bin = " << x_adj.to_bin() << "\t x_adj = " << x_adj << endl;
    // cout << "in bin = " << rem.to_bin() << "\t rem = " << rem << endl;
    // cout << "in bin = " << divisor.to_bin() << "\t divisor = " << divisor << endl;
    // cout << "in bin = " << root.to_bin() << "\t root = " << root << endl;

    const unsigned LOOP_REP    = (IN_I >> 1) + (OUT_W - OUT_I);
    const int      FINAL_SHIFT = (OUT_W - OUT_I);

    // cout << "LOOP_REP = " << LOOP_REP << endl;
    // cout << "FINAL_SHIFT = " << FINAL_SHIFT << endl;

    // cout << "IN_W = " << IN_W << endl;
    // cout << "IN_I = " << IN_I << endl;
    // cout << "OUT_W = " << OUT_W << endl;
    // cout << "OUT_I = " << OUT_I << endl;

SQRT_LOOP_1:
    for (int i = 0; i < LOOP_REP; i++) {
        root <<= 1;

        rem    = (rem << 2);
        rem[1] = x_adj[IN_W - 1];
        rem[0] = x_adj[IN_W - 2];
        // rem[x_adj[IN_W - 1]] = 1;
        // rem[x_adj[IN_W - 2]] = 0;
        // rem.assign_bit(1, x_adj[IN_W - 1]);
        // rem.assign_bit(0, x_adj[IN_W - 2]);
        x_adj <<= 2;

        divisor = (root << 1) + 1;
        // cout << "----- loop: " << i << endl;
        // cout << "in bin = " << rem.to_bin() << "\t rem = " << rem << endl;
        // cout << "in bin = " << divisor.to_bin() << "\t divisor = " << divisor << endl;
        // cout << "in bin = " << root.to_bin() << "\t root = " << root << endl;

        if (divisor <= rem) {
            rem -= divisor;
            root += 1;
        }
    }

    if (IN_I & 0x1) // odd
    {
        return (root >> (FINAL_SHIFT - 1)) * SC_INV_SQRT_2;
    } else // even
    {
        if (FINAL_SHIFT >= 0) {
            return (root >> FINAL_SHIFT);
        } else {
            return (root << -FINAL_SHIFT);
        }
    }
}

// fx16_16_t sqrt_fx16_16_to_fx16_16(fx16_16_t v)
template <int IN_W, int IN_I, int OUT_W, int OUT_I> sc_fixed<OUT_W, OUT_I> sqrt_func2(sc_fixed<IN_W, IN_I> v)
{
    cout << "in bin = " << v.to_bin() << "\t v = " << v << endl;

    uint32_t t, q, b, r;
    r = v;
    b = 0x40000000;
    q = 0;
    while (b > 0x40) {
        t = q + b;
        if (r >= t) {
            r -= t;
            q = t + b; // equivalent to q += 2*b
        }
        r <<= 1;
        b >>= 1;
    }
    q >>= 8;
    return q;
}

#endif // __SQRT_FUNC_HPP__
