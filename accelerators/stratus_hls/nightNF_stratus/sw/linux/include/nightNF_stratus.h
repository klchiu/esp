// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _NIGHTNF_STRATUS_H_
#define _NIGHTNF_STRATUS_H_

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

struct nightNF_stratus_access {
    struct esp_access esp;
    unsigned int      nimages;    // Number of images to be processed
    unsigned int      rows;       // Rows of input matrix. Rows of output vector.
    unsigned int      cols;       // Cols of input matrix. Cols of input vector.
    unsigned int      do_dwt;     // Enable/disable di DWT stage.
    unsigned          src_offset; // Input offset (bytes) used for P2P setup
    unsigned          dst_offset; // Output offset (bytes) used for P2P setup
};

#define NIGHTNF_STRATUS_IOC_ACCESS _IOW('S', 0, struct nightNF_stratus_access)

#endif /* _NIGHTNF_STRATUS_H_ */
