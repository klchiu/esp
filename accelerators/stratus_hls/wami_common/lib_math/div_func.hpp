#ifndef __DIV_FUNC_HPP__
#define __DIV_FUNC_HPP__

#include "esp_templates.hpp"

template <int N_W, int N_I, int D_W, int D_I, int Q_W, int Q_I>
sc_ufixed<Q_W, Q_I> udiv_func(sc_ufixed<N_W, N_I> num, sc_ufixed<D_W, D_I> den)
{
    // [Note]: for now <N_W, N_I>, <D_W, D_I>, <Q_W, Q_I> should be the same

    // cout << "in bin = " << num.to_bin() << "\tnum = " << num << endl;

    sc_ufixed<N_W, N_W> num_adj;
    sc_ufixed<D_W, D_W> den_adj;
    sc_ufixed<Q_W, Q_W> quotient;
    sc_ufixed<Q_W, Q_I> quo_ret;

    sc_ufixed<D_W + 1, D_W + 1> rem;

    int loop_count, max_loop;
    int num_first1, den_first1;
    int X_W, X_I;

    for (int i = 0; i < N_W; i++) {
        num_adj[i] = num[i];
        if (num[i] == 1) {
            num_first1 = i;
        }
    }
    for (int i = 0; i < D_W; i++) {
        den_adj[i] = den[i];
        if (den[i] == 1) {
            den_first1 = i;
        }
    }

    // Step 1: align the most-significant ones of num and den
    if (num_first1 > den_first1) {
        den_adj = den_adj << (num_first1 - den_first1);
        X_W     = N_W;
        X_I     = N_I;
    }
    if (num_first1 < den_first1) {
        num_adj = num_adj << (den_first1 - num_first1);
        X_W     = D_W;
        X_I     = D_I;
    }
    if (num_first1 == den_first1) {
        X_W = N_W;
        X_I = N_I;
    }
    max_loop = X_W - X_I;

    for (loop_count = 0; loop_count < max_loop; loop_count++) {

        // Step 2: if num > den, do the subtraction
        if (num_adj >= den_adj) {
            rem         = num_adj - den_adj;
            quotient[0] = 1;
            num_adj     = rem;
        }

        // Step 3: left shift num by 1
        num_adj = num_adj << 1;

        // Step 4: left shift quotient by 1
        quotient = quotient << 1;
    }

    // Step 5: shift back quotient
    if (num_first1 > den_first1) {
        quotient = quotient << (num_first1 - den_first1);
        quotient = quotient << (X_W - X_I - loop_count);
    }
    if (num_first1 < den_first1) {
        quotient = quotient >> (den_first1 - num_first1);
        quotient = quotient << (X_W - X_I - loop_count);
    }
    if (num_first1 == den_first1) {
        quotient = quotient << (X_W - X_I - loop_count);
    }

    for (int i = 0; i < Q_W; i++) {
        quo_ret[i] = quotient[i];
    }

    return quo_ret;
}

template <int N_W, int N_I, int D_W, int D_I, int Q_W, int Q_I>
sc_fixed<Q_W, Q_I> div_func(sc_fixed<N_W, N_I> num, sc_fixed<D_W, D_I> den)
{

    sc_ufixed<Q_W, Q_I> quotient;

    bool num_neg;
    bool den_neg;
    num_neg = num < 0;
    den_neg = den < 0;

    if (num_neg) {
        num = -num;
    }
    if (den_neg) {
        den = -den;
    }

    quotient = udiv_func<N_W, N_I, D_W, D_I, Q_W, Q_I>(num, den);
    if (num_neg ^ den_neg) {
        // fprintf(stderr, "udiv_func--> negative\n");
        return -quotient;
    } else {
        // fprintf(stderr, "udiv_func--> positive\n");
        return quotient;
    }
}



template <int N_L, int N_W, int D_L, int D_W, int Q_L, int Q_W>
sc_ufixed<Q_L, Q_W> udiv_func_old(sc_ufixed<N_L, N_W> num, sc_ufixed<D_L, D_W> den)
{
    sc_ufixed<N_L, N_L>         num_adj;
    sc_ufixed<D_L, D_L>         den_adj;
    sc_ufixed<D_L + 1, D_L + 1> rem;
    sc_ufixed<Q_L, Q_L>         quotient;

    bool num_neg;
    bool den_neg;

    num_adj = num << (N_L - N_W);

    den_adj = den << (D_L - D_W);

    rem      = 0;
    quotient = 0;

DIVIDE_LOOP:
    for (int i = 0; i < N_L + (D_L - D_W) + (Q_L - Q_W - N_L + N_W); i++) {
        quotient <<= 1;

        rem = rem << 1;
        rem.assign_bit(0, num_adj[N_L - 1]);
        num_adj <<= 1;
        if (den_adj <= rem) {
            rem -= den_adj;
            quotient += 1;
        }
    }

    return quotient >> (N_L - N_W + Q_L - Q_W - N_L + N_W);
}

template <int N_L, int N_W, int D_L, int D_W, int Q_L, int Q_W>
sc_fixed<Q_L, Q_W> div_func_old(sc_fixed<N_L, N_W> num, sc_fixed<D_L, D_W> den)
{

    sc_ufixed<Q_L, Q_W> quotient;

    bool num_neg;
    bool den_neg;
    num_neg = num < 0;
    den_neg = den < 0;

    if (num_neg) {
        num = -num;
    }
    if (den_neg) {
        den = -den;
    }

    quotient = udiv_func_old<N_L, N_W, D_L, D_W, Q_L, Q_W>(num, den);
    if (num_neg ^ den_neg)
        return -quotient;
    else
        return quotient;
}

#endif // __DIV_FUNC_HPP__
