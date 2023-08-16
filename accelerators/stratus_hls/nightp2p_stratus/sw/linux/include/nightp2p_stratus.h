// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _NIGHTP2P_STRATUS_H_
#define _NIGHTP2P_STRATUS_H_

#ifdef __KERNEL__
    #include <linux/ioctl.h>
    #include <linux/types.h>
#else
    #include <stdint.h>
    #include <sys/ioctl.h>
    #ifndef __user
        #define __user
    #endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

#define MAX_NIMAGES 10000
#define MAX_ROWS    2048
#define MAX_COLS    2048

typedef float          fltPixel_t;
typedef unsigned short senPixel_t;
typedef int            algPixel_t;
typedef short pixel_t;

// typedef int32_t token_t;
typedef uint64_t token_t;


// struct nightp2p_test {
    // struct test_info               info;
    // struct nightp2p_stratus_access desc;
    char *                         test_infile;
    bool                           test_infile_is_raw;
    unsigned                       test_nimages;
    unsigned                       test_rows;
    unsigned                       test_cols;
    unsigned                       test_nbytespp;
    unsigned                       test_swapbytes;
    unsigned                       test_do_dwt;
    unsigned                       test_nbpp_in;
    unsigned                       test_nbpp_out;
    pixel_t *                      test_hbuf;
    int *                          test_sbuf_in;
    int *                          test_sbuf_out;
    bool                           test_verbose;
// };

struct nightvision_stratus_access {
    struct esp_access esp;
    unsigned int      nimages;    // Number of images to be processed
    unsigned int      rows;       // Rows of input matrix. Rows of output vector.
    unsigned int      cols;       // Cols of input matrix. Cols of input vector.
    unsigned int      do_dwt;     // Enable/disable di DWT stage.
    unsigned          src_offset; // Input offset (bytes) used for P2P setup
    unsigned          dst_offset; // Output offset (bytes) used for P2P setup
};

struct nightNF_stratus_access {
    struct esp_access esp;
    unsigned int      nimages; // Number of images to be processed
    unsigned int      rows;    // Rows of input matrix. Rows of output vector.
    unsigned int      cols;    // Cols of input matrix. Cols of input vector.
    unsigned int      do_dwt;  // Enable/disable di DWT stage.
    unsigned int      is_p2p;
    unsigned int      p2p_config_0;
    unsigned          src_offset; // Input offset (bytes) used for P2P setup
    unsigned          dst_offset; // Output offset (bytes) used for P2P setup
};

#define NIGHTNF_STRATUS_IOC_ACCESS     _IOW('S', 0, struct nightNF_stratus_access)
#define NIGHTHIST_STRATUS_IOC_ACCESS   _IOW('S', 0, struct nightvision_stratus_access)
#define NIGHTHISTEQ_STRATUS_IOC_ACCESS _IOW('S', 0, struct nightvision_stratus_access)
#define NIGHTDWT_STRATUS_IOC_ACCESS    _IOW('S', 0, struct nightvision_stratus_access)


algPixel_t *inputA;
algPixel_t *output;


uint32_t i;

unsigned long long time_s;

struct timespec t_test_1, t_test_2;

#endif /* _NIGHTP2P_STRATUS_H_ */
