// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __GEMMENOUGH256_CONF_INFO_HPP__
#define __GEMMENOUGH256_CONF_INFO_HPP__

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
        this->ninputs = 1;
        this->d3 = 256;
        this->d2 = 256;
        this->d1 = 256;
        this->st_offset = 131072;
        this->ld_offset1 = 0;
        this->ld_offset2 = 65536;
    }

    conf_info_t(
        /* <<--ctor-args-->> */
        int32_t ninputs, 
        int32_t d3, 
        int32_t d2, 
        int32_t d1, 
        int32_t st_offset, 
        int32_t ld_offset1, 
        int32_t ld_offset2
        )
    {
        /* <<--ctor-custom-->> */
        this->ninputs = ninputs;
        this->d3 = d3;
        this->d2 = d2;
        this->d1 = d1;
        this->st_offset = st_offset;
        this->ld_offset1 = ld_offset1;
        this->ld_offset2 = ld_offset2;
    }

    // equals operator
    inline bool operator==(const conf_info_t &rhs) const
    {
        /* <<--eq-->> */
        if (ninputs != rhs.ninputs) return false;
        if (d3 != rhs.d3) return false;
        if (d2 != rhs.d2) return false;
        if (d1 != rhs.d1) return false;
        if (st_offset != rhs.st_offset) return false;
        if (ld_offset1 != rhs.ld_offset1) return false;
        if (ld_offset2 != rhs.ld_offset2) return false;
        return true;
    }

    // assignment operator
    inline conf_info_t& operator=(const conf_info_t& other)
    {
        /* <<--assign-->> */
        ninputs = other.ninputs;
        d3 = other.d3;
        d2 = other.d2;
        d1 = other.d1;
        st_offset = other.st_offset;
        ld_offset1 = other.ld_offset1;
        ld_offset2 = other.ld_offset2;
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
        os << "ninputs = " << conf_info.ninputs << ", ";
        os << "d3 = " << conf_info.d3 << ", ";
        os << "d2 = " << conf_info.d2 << ", ";
        os << "d1 = " << conf_info.d1 << ", ";
        os << "st_offset = " << conf_info.st_offset << ", ";
        os << "ld_offset1 = " << conf_info.ld_offset1 << ", ";
        os << "ld_offset2 = " << conf_info.ld_offset2 << "";
        os << "}";
        return os;
    }

        /* <<--params-->> */
        int32_t ninputs;
        int32_t d3;
        int32_t d2;
        int32_t d1;
        int32_t st_offset;
        int32_t ld_offset1;
        int32_t ld_offset2;
};

#endif // __GEMMENOUGH256_CONF_INFO_HPP__
