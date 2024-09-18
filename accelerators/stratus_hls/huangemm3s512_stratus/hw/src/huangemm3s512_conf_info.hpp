// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __HUANGEMM3S512_CONF_INFO_HPP__
#define __HUANGEMM3S512_CONF_INFO_HPP__

#include <systemc.h>

//
// Configuration parameters for the accelerator.
//
class conf_info_t
{
public:

    //
    // constructors
    //
    conf_info_t()
    {
        /* <<--ctor-->> */
        // this->rows = 10;
        // this->cols = 1;
        // this->loaded_cols = 32678;

        this->rows = 4;
        this->cols = 4;
        this->loaded_cols = 4;

        // this->rows = 10;
        // this->cols = 1;
        // this->loaded_cols = 32678;
    }

    conf_info_t(
        /* <<--ctor-args-->> */
        int32_t rows, 
        int32_t cols, 
        int32_t loaded_cols
        )
    {
        /* <<--ctor-custom-->> */
        this->rows = rows;
        this->cols = cols;
        this->loaded_cols = loaded_cols;
    }

    // equals operator
    inline bool operator==(const conf_info_t &rhs) const
    {
        /* <<--eq-->> */
        if (rows != rhs.rows) return false;
        if (cols != rhs.cols) return false;
        if (loaded_cols != rhs.loaded_cols) return false;
        return true;
    }

    // assignment operator
    inline conf_info_t& operator=(const conf_info_t& other)
    {
        /* <<--assign-->> */
        rows = other.rows;
        cols = other.cols;
        loaded_cols = other.loaded_cols;
        return *this;
    }

    // VCD dumping function
    friend void sc_trace(sc_trace_file *tf, const conf_info_t &v, const std::string &NAME)
    {}

    // redirection operator
    friend ostream& operator << (ostream& os, conf_info_t const &conf_info)
    {
        os << "{";
        /* <<--print-->> */
        os << "rows = " << conf_info.rows << ", ";
        os << "cols = " << conf_info.cols << ", ";
        os << "loaded_cols = " << conf_info.loaded_cols << "";
        os << "}";
        return os;
    }

        /* <<--params-->> */
        int32_t rows;
        int32_t cols;
        int32_t loaded_cols;
};

#endif // __HUANGEMM3S512_CONF_INFO_HPP__
