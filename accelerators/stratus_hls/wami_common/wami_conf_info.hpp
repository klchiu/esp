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
        : num_img(0)
        , num_row(0)
        , num_col(0)
        , pad(0)
        , kern_id(0)
        , batch(0)
        , src_dst_offset_0(0)
        , src_dst_offset_1(0)
        , src_dst_offset_2(0)
        , src_dst_offset_3(0)
        , src_dst_offset_4(0)
        , is_p2p(0)
        , p2p_config_0(0)
        , p2p_config_1(0)
    {
    }

    conf_info_t(uint32_t ni, uint32_t nr, uint32_t nc, uint32_t p, uint32_t ki, uint32_t b, uint32_t ba0, uint32_t ba1,
                uint32_t ba2 = 0, uint32_t ba3 = 0, uint32_t ba4 = 0, uint32_t p2p = 0, uint32_t p2p_c0 = 0,
                uint32_t p2p_c1 = 0)
        : num_img(ni)
        , num_row(nr)
        , num_col(nc)
        , pad(p)
        , kern_id(ki)
        , batch(b)
        , src_dst_offset_0(ba0)
        , src_dst_offset_1(ba1)
        , src_dst_offset_2(ba2)
        , src_dst_offset_3(ba3)
        , src_dst_offset_4(ba4)
        , is_p2p(p2p)
        , p2p_config_0(p2p_c0)
        , p2p_config_1(p2p_c1)
    {
    }

    bool operator==(const conf_info_t &rhs) const
    {
        return (rhs.num_img == num_img) && (rhs.num_row == num_row) && (rhs.num_col == num_col) && (rhs.pad == pad) &&
               (rhs.kern_id == kern_id) && (rhs.batch == batch) && (rhs.src_dst_offset_0 == src_dst_offset_0) &&
               (rhs.src_dst_offset_1 == src_dst_offset_1) && (rhs.src_dst_offset_2 == src_dst_offset_2) &&
               (rhs.src_dst_offset_3 == src_dst_offset_3) && (rhs.src_dst_offset_4 == src_dst_offset_4) &&
               (rhs.is_p2p == is_p2p) && (rhs.p2p_config_0 == p2p_config_0) && (rhs.p2p_config_1 == p2p_config_1);
    }

    friend void sc_trace(sc_trace_file *tf, const conf_info_t &v,
                         const std::string &NAME) // VCD dumping function
    {
        ;
    }

    friend ostream &operator<<(ostream &os, conf_info_t const &conf_info)
    {
        os << "{ num_img = " << conf_info.num_img << ", num_row = " << conf_info.num_row
           << ", num_col = " << conf_info.num_col << ", pad = " << conf_info.pad
           << ", kern_id = " << (uint32_t)conf_info.kern_id << ", batch = " << conf_info.batch
           << ", src_dst_offset_0 = 0x" << std::hex << conf_info.src_dst_offset_0 << std::dec << " ("
           << conf_info.src_dst_offset_0 << ")"
           << ", src_dst_offset_1 = 0x" << std::hex << conf_info.src_dst_offset_1 << std::dec << " ("
           << conf_info.src_dst_offset_1 << ")"
           << ", src_dst_offset_2 = 0x" << std::hex << conf_info.src_dst_offset_2 << std::dec << " ("
           << conf_info.src_dst_offset_2 << ")"
           << ", src_dst_offset_3 = 0x" << std::hex << conf_info.src_dst_offset_3 << std::dec << " ("
           << conf_info.src_dst_offset_3 << ")"
           << ", src_dst_offset_4 = 0x" << std::hex << conf_info.src_dst_offset_4 << std::dec << " ("
           << conf_info.src_dst_offset_4 << ")"
           << ", is_p2p = " << conf_info.is_p2p << ", p2p_config_0 = " << conf_info.p2p_config_0
           << ", p2p_config_1 = " << conf_info.p2p_config_1

           << "}";
        return os;
    }

    uint32_t num_img; // Number of images
    uint32_t num_row; // Number of rows for each image
    uint32_t num_col; // Number of columns for each image
    uint32_t pad;     // Some accelerators may require padding information
    uint32_t kern_id;
    uint32_t batch;
    uint32_t src_dst_offset_0; // Data base address in main memory
    uint32_t src_dst_offset_1;
    uint32_t src_dst_offset_2;
    uint32_t src_dst_offset_3;
    uint32_t src_dst_offset_4;
    uint32_t is_p2p;
    uint32_t p2p_config_0;
    uint32_t p2p_config_1;
};

#endif // __WAMI_CONF_INFO_HPP__
