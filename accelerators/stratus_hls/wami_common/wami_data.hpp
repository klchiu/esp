#ifndef _WAMI_DATA_HPP_
#define _WAMI_DATA_HPP_

#include <systemc.h>

#include "wami_config.hpp"

#include "wami_C_data.hpp"


#define PXL_WORDL   32
#define PXL64_WORDL 64
#define PXL_IWORDL  14

#define GRAYS32_WORDL 32
#define GRAYS64_WORDL 64
#define GRAYS_IWORDL  34

#define GRAD32_WORDL 32
#define GRAD64_WORDL 64
#define GRAD_IWORDL  34

#define WARP32_WORDL 32
#define WARP64_WORDL 64
#define WARP_IWORDL  16

#define SUB32_WORDL 32
#define SUB64_WORDL 64
#define SUB_IWORDL  13

#define DESC32_WORDL 32
#define DESC64_WORDL 64
#define DESC_IWORDL  21

#define HESS32_WORDL 32
#define HESS64_WORDL 64
#define HESS_IWORDL  51

#define INVGJ32_WORDL 32
#define INVGJ64_WORDL 64
#define INVGJ_IWORDL  2

#define LARGE_WORDL  64
#define LARGE_IWORDL 51

#define SDUPD32_WORDL 32
#define SDUPD64_WORDL 64
#define SDUPD_IWORDL  50

#define MULT32_WORDL 32
#define MULT64_WORDL 64
#define MULT_IWORDL  10

#define RESHAPE32_WORDL 32
#define RESHAPE64_WORDL 64
#define RESHAPE_IWORDL  10

#define ADD32_WORDL 32
#define ADD64_WORDL 64
#define ADD_IWORDL  16

#define TRAIN32_WORDL 32
#define TRAIN64_WORDL 64
#define TRAIN_IWORDL  15

#define GMM32_WORDL 32
#define GMM64_WORDL 64
#define GMM_IWORDL  32

#define INT_WORDL  32
#define INT_IWORDL 32

// TODO: To be defined in the Makefile
//#define CPP_FLOAT_POINT
//#define SC_FIXED_POINT_FAST
//#define SC_FIXED_POINT
//#define CTOS_SC_FIXED_POINT

// [humu]: this can be removed
// #ifdef CPP_FLOAT_POINT // fastest implementation (PV-r)
//     #error "Not supported!"
// #endif

// #if defined(SC_FIXED_POINT_FAST) // SystemC sc_fixed_fast (PV-r)
//     #define SC_FIXED sc_dt::sc_fixed_fast
//     #define Q_MODE   sc_dt::SC_RND

// #elif defined(SC_FIXED_POINT) // SystemC sc_fixed (PV-r)
//     #define SC_FIXED sc_dt::sc_fixed
//     #define Q_MODE   sc_dt::SC_RND

// #elif defined(CTOS_SC_FIXED_POINT) // CtoS sc_fixed (HLS-r)
//     #include <ctos_fx.h>
//     #define SC_FIXED ctos_sc_dt::sc_fixed
//     #define Q_MODE   ctos_sc_dt::SC_RND

// #else
//     #error "Floating/Fixed point implementation is not specified!"
// #endif
// ----------


#define PXL_CTOS_FLOAT       SC_FIXED<PXL_WORDL, PXL_IWORDL, Q_MODE>
#define PXL64_CTOS_FLOAT     SC_FIXED<PXL64_WORDL, PXL_IWORDL, Q_MODE>
#define GRAYS32_CTOS_FLOAT   SC_FIXED<GRAYS32_WORDL, GRAYS_IWORDL, Q_MODE>
#define GRAYS64_CTOS_FLOAT   SC_FIXED<GRAYS64_WORDL, GRAYS_IWORDL, Q_MODE>
#define GRAD32_CTOS_FLOAT    SC_FIXED<GRAD32_WORDL, GRAD_IWORDL, Q_MODE>
#define GRAD64_CTOS_FLOAT    SC_FIXED<GRAD64_WORDL, GRAD_IWORDL, Q_MODE>
#define WARP32_CTOS_FLOAT    SC_FIXED<WARP32_WORDL, WARP_IWORDL, Q_MODE>
#define WARP64_CTOS_FLOAT    SC_FIXED<WARP64_WORDL, WARP_IWORDL, Q_MODE>
#define SUB_CTOS_FLOAT       SC_FIXED<SUB_WORDL, SUB_IWORDL, Q_MODE>
#define SUB64_CTOS_FLOAT     SC_FIXED<SUB64_WORDL, SUB_IWORDL, Q_MODE>
#define DESC32_CTOS_FLOAT    SC_FIXED<DESC32_WORDL, DESC_IWORDL, Q_MODE>
#define DESC64_CTOS_FLOAT    SC_FIXED<DESC64_WORDL, DESC_IWORDL, Q_MODE>
#define HESS32_CTOS_FLOAT    SC_FIXED<HESS32_WORDL, HESS_IWORDL, Q_MODE>
#define HESS64_CTOS_FLOAT    SC_FIXED<HESS64_WORDL, HESS_IWORDL, Q_MODE>
#define INVGJ32_CTOS_FLOAT   SC_FIXED<INVGJ32_WORDL, INVGJ_IWORDL, Q_MODE>
#define INVGJ64_CTOS_FLOAT   SC_FIXED<INVGJ64_WORDL, INVGJ_IWORDL, Q_MODE>
#define LARGE_CTOS_FLOAT     SC_FIXED<LARGE_WORDL, LARGE_IWORDL, Q_MODE>
#define SDUPD32_CTOS_FLOAT   SC_FIXED<SDUPD32_WORDL, SDUPD_IWORDL, Q_MODE>
#define SDUPD64_CTOS_FLOAT   SC_FIXED<SDUPD64_WORDL, SDUPD_IWORDL, Q_MODE>
#define MULT32_CTOS_FLOAT    SC_FIXED<MULT32_WORDL, MULT_IWORDL, Q_MODE>
#define MULT64_CTOS_FLOAT    SC_FIXED<MULT64_WORDL, MULT_IWORDL, Q_MODE>
#define RESHAPE32_CTOS_FLOAT SC_FIXED<RESHAPE32_WORDL, RESHAPE_IWORDL, Q_MODE>
#define RESHAPE64_CTOS_FLOAT SC_FIXED<RESHAPE64_WORDL, RESHAPE_IWORDL, Q_MODE>
#define ADD32_CTOS_FLOAT     SC_FIXED<ADD32_WORDL, ADD_IWORDL, Q_MODE>
#define ADD64_CTOS_FLOAT     SC_FIXED<ADD64_WORDL, ADD_IWORDL, Q_MODE>
#define TRAIN32_CTOS_FLOAT   SC_FIXED<TRAIN32_WORDL, TRAIN_IWORDL, Q_MODE>
#define TRAIN64_CTOS_FLOAT   SC_FIXED<TRAIN64_WORDL, TRAIN_IWORDL, Q_MODE>
#define GMM32_CTOS_FLOAT     SC_FIXED<GMM32_WORDL, GMM_IWORDL, Q_MODE>
#define GMM64_CTOS_FLOAT     SC_FIXED<GMM64_WORDL, GMM_IWORDL, Q_MODE>
#define INT_CTOS_FLOAT       SC_FIXED<INT_WORDL, INT_IWORDL, Q_MODE>

//#define ONE_CTOS_FLOAT SC_FIXED<GMM64_WORDL, GMM_IWORDL, Q_MODE>(1.0)
#define ONE_CTOS_FLOAT SC_FIXED<2, 2, Q_MODE>(1.0)

// data conversion: from sc_fixed to sc_bv
template <typename T, size_t N, size_t WL, size_t IL> void to_sc_bv(sc_dt::sc_bv<N> &data_out, T data_in)
{
    // TODO: some assertions has to be put here to check conversion correctness
    size_t FL_source = WL - IL;
    size_t FL_target = N - IL;

    // TODO: so far this is the easiest way to preserv the sign
    if (data_in[WL - 1] == 1) {
TO_SC_BV_SIGN_LOOP:
        for (uint32_t i = 0; i < N; i++)
            data_out[i] = 1;
    }

    // transfer the fractional part
TO_SC_BV_F_LOOP:
    for (uint32_t i = 0; i < FL_target; i++)
        data_out[i] = data_in[i];

    // transfer the integer part
TO_SC_BV_I_LOOP:
    for (uint32_t i = 0; i < IL; i++)
        data_out[N - i - 1] = data_in[WL - i - 1];
}

template <typename T, size_t N, size_t WL> void to_sc_bv(sc_dt::sc_bv<N> &data_out, T data_in)
{
TO_SC_BV_LOOP:
    for (uint32_t i = 0; i < N; i++)
        data_out[N - i - 1] = data_in[WL - i - 1];
}

template <typename T, size_t N> void to_sc_bv(sc_dt::sc_bv<N> &data_out, T data_in)
{
TO_SC_BV_LOOP:
    for (uint32_t i = 0; i < N; i++)
        data_out[i] = data_in[i];
}

// data conversion: from sc_bv to sc_fixed
#ifdef SC_INCLUDE_FX
template <typename T, size_t N, size_t WL, size_t IL> void to_sc_fx(T &data_out, sc_bv<N> data_in)
{
    // TODO: some assertions has to be put here to check conversion correctness
    size_t FL_source = WL - IL;
    size_t FL_target = N - IL;

    // TODO: so far this is the easiest way to preserv the sign
    if (data_in[N - 1] == 1) {
TO_SC_FX_SIGN_LOOP:
        for (uint32_t i = 0; i < WL; i++)
            data_out[WL - 1 - i] = 1;
    }

TO_SC_FX_F_LOOP:
    for (uint32_t i = 0; i < FL_target; i++)
        data_out[i] = data_in[i].to_bool();

TO_SC_FX_I_LOOP:
    for (uint32_t i = 0; i < IL; i++)
        data_out[WL - i - 1] = data_in[N - i - 1].to_bool();
}

template <typename T, size_t N> void to_sc_fx(T &data_out, sc_bv<N> data_in)
{
TO_SC_FX_LOOP:
    for (uint32_t i = 0; i < N; i++)
        data_out[i] = data_in[i].to_bool();
}

template <typename T, size_t N, size_t WL> void to_sc_fx(T &data_out, sc_bv<N> data_in)
{
TO_SC_FX_LOOP:
    for (uint32_t i = 0; i < N; i++)
        data_out[WL - i - 1] = data_in[N - i - 1].to_bool();
}
#else

template <typename T, size_t N, size_t WL, size_t IL> void to_sc_fx(T &data_out, sc_bv<N> data_in)
{
    // TODO: some assertions has to be put here to check conversion correctness
    size_t FL_source = WL - IL;
    size_t FL_target = N - IL;

    // TODO: so far this is the easiest way to preserv the sign
    if (data_in[N - 1] == 1) {
TO_SC_FX_SIGN_LOOP:
        for (uint32_t i = 0; i < WL; i++)
            data_out.assign_bit(i, 1);
    }

TO_SC_FX_F_LOOP:
    for (uint32_t i = 0; i < FL_target; i++)
        data_out.assign_bit(i, data_in[i].to_bool());

TO_SC_FX_I_LOOP:
    for (uint32_t i = 0; i < IL; i++)
        data_out.assign_bit(WL - i - 1, data_in[N - i - 1].to_bool());
}

template <typename T, size_t N, size_t WL> void to_sc_fx(T &data_out, sc_bv<N> data_in)
{
TO_SC_FX_LOOP:
    for (uint32_t i = 0; i < N; i++)
        data_out.assign_bit(WL - i - 1, data_in[N - i - 1].to_bool());
}

template <typename T, size_t N> void to_sc_fx(T &data_out, sc_bv<N> data_in)
{
TO_SC_FX_LOOP:
    for (uint32_t i = 0; i < N; i++)
        data_out.assign_bit(i, data_in[i].to_bool());
}
#endif

// // == containers =======================================================
// class dma_info_t
// {
//   public:
//     dma_info_t()
//         : index(0)
//         , length(0)
//     {
//     }

//     dma_info_t(uint32_t i, uint32_t l)
//         : index(i)
//         , length(l)
//     {
//     }

//     bool operator==(const dma_info_t &rhs) const { return ((rhs.index == index) && (rhs.length == length)); }

//     friend void sc_trace(sc_trace_file *tf, const dma_info_t &v,
//                          const std::string &NAME) // VCD dumping function
//     {
//         ;
//     }

//     friend ostream &operator<<(ostream &os, dma_info_t const &dma_info)
//     {
//         os << "{" << dma_info.index << "," << dma_info.length << "}";
//         return os;
//     }

//     uint32_t index;  // memory-offset w.r.t. base address
//     uint32_t length; // DMA-burst size
// };

// //
// // Configuration parameters for the accelerator.
// //
// class conf_info_t
// {
//   public:
//     conf_info_t()
//         : src_dst_offset_0(0)
//         , num_img(0)
//         , num_col(0)
//         , num_row(0)
//         , pad(0)
//         , kern_id(0)
//         , batch(0)
//         , src_dst_offset_1(0)
//         , src_dst_offset_2(0)
//         , src_dst_offset_3(0)
//         , src_dst_offset_4(0)
//     {
//     }

//     conf_info_t(uint32_t ba, uint32_t ni, uint32_t nc, uint32_t nr, uint32_t p, uint8_t ki, uint32_t ba1 = 0,
//                 uint32_t ba2 = 0, uint32_t ba3 = 0, uint32_t ba4 = 0, uint32_t b = 1)
//         : src_dst_offset_0(ba)
//         , num_img(ni)
//         , num_col(nc)
//         , num_row(nr)
//         , pad(p)
//         , kern_id(ki)
//         , batch(b)
//         , src_dst_offset_1(ba1)
//         , src_dst_offset_2(ba2)
//         , src_dst_offset_3(ba3)
//         , src_dst_offset_4(ba4)
//     {
//     }

//     bool operator==(const conf_info_t &rhs) const
//     {
//         return (rhs.src_dst_offset_0 == src_dst_offset_0) && (rhs.src_dst_offset_1 == src_dst_offset_1) &&
//                (rhs.src_dst_offset_2 == src_dst_offset_2) && (rhs.src_dst_offset_3 == src_dst_offset_3) &&
//                (rhs.src_dst_offset_4 == src_dst_offset_4) && (rhs.num_img == num_img) && (rhs.num_col == num_col) &&
//                (rhs.num_row == num_row) && (rhs.pad == pad) && (rhs.batch == batch) && (rhs.kern_id == kern_id);
//     }

//     friend void sc_trace(sc_trace_file *tf, const conf_info_t &v,
//                          const std::string &NAME) // VCD dumping function
//     {
//         ;
//     }

//     friend ostream &operator<<(ostream &os, conf_info_t const &conf_info)
//     {
//         os << "{ src_dst_offset_0 = 0x" << std::hex << conf_info.src_dst_offset_0 << std::dec << " ("
//            << conf_info.src_dst_offset_0 << ")"
//            << ", src_dst_offset_1 = 0x" << std::hex << conf_info.src_dst_offset_1 << std::dec << " ("
//            << conf_info.src_dst_offset_1 << ")"
//            << ", src_dst_offset_2 = 0x" << std::hex << conf_info.src_dst_offset_2 << std::dec << " ("
//            << conf_info.src_dst_offset_2 << ")"
//            << ", src_dst_offset_3 = 0x" << std::hex << conf_info.src_dst_offset_3 << std::dec << " ("
//            << conf_info.src_dst_offset_3 << ")"
//            << ", src_dst_offset_4 = 0x" << std::hex << conf_info.src_dst_offset_4 << std::dec << " ("
//            << conf_info.src_dst_offset_4 << ")"
//            << ", num_img = " << conf_info.num_img << ", num_col = " << conf_info.num_col
//            << ", num_row = " << conf_info.num_row << ", pad = " << conf_info.pad << ", batch = " << conf_info.batch
//            << ", kern_id = " << (uint32_t)conf_info.kern_id << "}";
//         return os;
//     }

//     uint32_t src_dst_offset_0; // Data base address in main memory
//     uint32_t num_img;          // Number of images
//     uint32_t num_col;          // Number of columns for each image
//     uint32_t num_row;          // Number of rows for each image
//     uint32_t pad;              // Some accelerators may require padding information
//     uint8_t  kern_id;
//     uint32_t batch;
//     uint32_t src_dst_offset_1;
//     uint32_t src_dst_offset_2;
//     uint32_t src_dst_offset_3;
//     uint32_t src_dst_offset_4;
// };

#endif  // _WAMI_DATA_HPP_
