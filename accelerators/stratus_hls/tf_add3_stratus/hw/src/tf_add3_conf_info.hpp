// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __TF_ADD3_CONF_INFO_HPP__
#define __TF_ADD3_CONF_INFO_HPP__

#include <systemc.h>
#include "fpdata.hpp"

//
// Configuration parameters for the accelerator.
//
class conf_info_t
{

  public:
    conf_info_t()
        : length(0)
        , src_dst_offset_0(0)
        , src_dst_offset_1(0)
        , src_dst_offset_2(0)
        , chunk_size(64)
    {
    }

    conf_info_t(uint32_t len, uint32_t ba0, uint32_t ba1, uint32_t ba2, uint32_t ba3)
        : length(len)
        , src_dst_offset_0(ba0)
        , src_dst_offset_1(ba1)
        , src_dst_offset_2(ba2)
        , chunk_size(ba3)
    {
    }

    bool operator==(const conf_info_t &rhs) const
    {
        return (rhs.length == length) && (rhs.src_dst_offset_0 == src_dst_offset_0) &&
        (rhs.src_dst_offset_1 == src_dst_offset_1) && (rhs.src_dst_offset_2 == src_dst_offset_2) && (rhs.chunk_size == chunk_size);
    }

    friend void sc_trace(sc_trace_file *tf, const conf_info_t &v,
                         const std::string &NAME) // VCD dumping function
    {
        ;
    }

    friend ostream &operator<<(ostream &os, conf_info_t const &conf_info)
    {
        os << "{ length = " << conf_info.length << " "
           << ", src_dst_offset_0 = 0x" << std::hex << conf_info.src_dst_offset_0 << std::dec << " ("
           << conf_info.src_dst_offset_0 << ")"
           << ", src_dst_offset_1 = 0x" << std::hex << conf_info.src_dst_offset_1 << std::dec << " ("
           << conf_info.src_dst_offset_1 << ")"
           << ", src_dst_offset_2 = 0x" << std::hex << conf_info.src_dst_offset_2 << std::dec << " ("
           << conf_info.src_dst_offset_2 << ")"
           << ", chunk_size = " << conf_info.chunk_size << " "
           << "}";
        return os;
    }

    uint32_t length;
    uint32_t src_dst_offset_0;
    uint32_t src_dst_offset_1;
    uint32_t src_dst_offset_2;
    uint32_t chunk_size;
};

#endif // __TF_ADD3_CONF_INFO_HPP__
