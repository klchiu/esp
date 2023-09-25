// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0

#ifndef __WAMI_CONF_INFO_HPP__
#define __WAMI_CONF_INFO_HPP__

#include <systemc.h>

//
// Configuration parameters for the accelerator.
//
class conf_info_t
{

  public:
    conf_info_t()
        : src_dst_offset_0(0)
        , num_img(0)
        , num_col(0)
        , num_row(0)
        , pad(0)
        , kern_id(0)
        , batch(0)
        , src_dst_offset_1(0)
        , src_dst_offset_2(0)
        , src_dst_offset_3(0)
        , src_dst_offset_4(0)
    {
    }

    conf_info_t(uint32_t ba, uint32_t ni, uint32_t nc, uint32_t nr, uint32_t p, uint32_t ki, uint32_t ba1 = 0,
                uint32_t ba2 = 0, uint32_t ba3 = 0, uint32_t ba4 = 0, uint32_t b = 1)
        : src_dst_offset_0(ba)
        , num_img(ni)
        , num_col(nc)
        , num_row(nr)
        , pad(p)
        , kern_id(ki)
        , batch(b)
        , src_dst_offset_1(ba1)
        , src_dst_offset_2(ba2)
        , src_dst_offset_3(ba3)
        , src_dst_offset_4(ba4)
    {
    }

    bool operator==(const conf_info_t &rhs) const
    {
        return (rhs.src_dst_offset_0 == src_dst_offset_0) && (rhs.src_dst_offset_1 == src_dst_offset_1) &&
               (rhs.src_dst_offset_2 == src_dst_offset_2) && (rhs.src_dst_offset_3 == src_dst_offset_3) &&
               (rhs.src_dst_offset_4 == src_dst_offset_4) && (rhs.num_img == num_img) && (rhs.num_col == num_col) &&
               (rhs.num_row == num_row) && (rhs.pad == pad) && (rhs.batch == batch) && (rhs.kern_id == kern_id);
    }

    friend void sc_trace(sc_trace_file *tf, const conf_info_t &v,
                         const std::string &NAME) // VCD dumping function
    {
        ;
    }

    friend ostream &operator<<(ostream &os, conf_info_t const &conf_info)
    {
        os << "{ src_dst_offset_0 = 0x" << std::hex << conf_info.src_dst_offset_0 << std::dec << " ("
           << conf_info.src_dst_offset_0 << ")"
           << ", src_dst_offset_1 = 0x" << std::hex << conf_info.src_dst_offset_1 << std::dec << " ("
           << conf_info.src_dst_offset_1 << ")"
           << ", src_dst_offset_2 = 0x" << std::hex << conf_info.src_dst_offset_2 << std::dec << " ("
           << conf_info.src_dst_offset_2 << ")"
           << ", src_dst_offset_3 = 0x" << std::hex << conf_info.src_dst_offset_3 << std::dec << " ("
           << conf_info.src_dst_offset_3 << ")"
           << ", src_dst_offset_4 = 0x" << std::hex << conf_info.src_dst_offset_4 << std::dec << " ("
           << conf_info.src_dst_offset_4 << ")"
           << ", num_img = " << conf_info.num_img << ", num_col = " << conf_info.num_col
           << ", num_row = " << conf_info.num_row << ", pad = " << conf_info.pad << ", batch = " << conf_info.batch
           << ", kern_id = " << (uint32_t)conf_info.kern_id << "}";
        return os;
    }

    uint32_t src_dst_offset_0; // Data base address in main memory
    uint32_t num_img;          // Number of images
    uint32_t num_col;          // Number of columns for each image
    uint32_t num_row;          // Number of rows for each image
    uint32_t pad;              // Some accelerators may require padding information
    uint32_t kern_id;
    uint32_t batch;
    uint32_t src_dst_offset_1;
    uint32_t src_dst_offset_2;
    uint32_t src_dst_offset_3;
    uint32_t src_dst_offset_4;
};

#endif // __WAMI_CONF_INFO_HPP__
