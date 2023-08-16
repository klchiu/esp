// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __NIGHTNF_CONF_INFO_HPP__
#define __NIGHTNF_CONF_INFO_HPP__

#include <systemc.h>

//
// Configuration parameters for the accelerator.
//
class conf_info_t
{
  public:
    uint32_t n_Images; // Rows of input image
    uint32_t n_Rows;   // Rows of input image
    uint32_t n_Cols;   // Columns of input image
    uint32_t do_dwt;   // Optional DWT step
    uint32_t is_p2p;
    uint32_t p2p_config_0;

    //
    // constructors
    //
    conf_info_t()
        : n_Images(0)
        , n_Rows(0)
        , n_Cols(0)
        , do_dwt(0)
        , is_p2p(0)
        , p2p_config_0(0)
    {
    }

    conf_info_t(uint32_t nimages, uint32_t rows, uint32_t cols, uint32_t do_dwt, uint32_t is_p2p, uint32_t p2p_config_0)
        : n_Images(nimages)
        , n_Rows(rows)
        , n_Cols(cols)
        , do_dwt(do_dwt)
        , is_p2p(is_p2p)
        , p2p_config_0(p2p_config_0)
    {
    }

    // equals operator
    inline bool operator==(const conf_info_t &rhs) const
    {
        return (rhs.n_Images == n_Images) && (rhs.n_Rows == n_Rows) && (rhs.n_Cols == n_Cols) && (rhs.do_dwt == do_dwt)
         && (rhs.is_p2p == is_p2p)  && (rhs.p2p_config_0 == p2p_config_0);
    }

    // assignment operator
    inline conf_info_t &operator=(const conf_info_t &other)
    {
        n_Images = other.n_Images;
        n_Rows   = other.n_Rows;
        n_Cols   = other.n_Cols;
        do_dwt   = other.do_dwt;
        is_p2p   = other.is_p2p;
        p2p_config_0   = other.p2p_config_0;
        return *this;
    }

    // VCD dumping function
    friend void sc_trace(sc_trace_file *tf, const conf_info_t &v, const std::string &NAME) {}

    // redirection operator
    friend ostream &operator<<(ostream &os, conf_info_t const &conf_info)
    {
        os << "{ n_Images = " << conf_info.n_Images << ", n_Rows = " << conf_info.n_Rows
           << ", n_Cols = " << conf_info.n_Cols << ", do_dwt = " << conf_info.do_dwt <<
           ", is_p2p = " << conf_info.is_p2p <<
           ", p2p_config_0 = " << conf_info.p2p_config_0 <<
            "}";
        return os;
    }
};

#endif // __NIGHTNF_CONF_INFO_HPP__
