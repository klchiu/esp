// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _NY_ACC_STRATUS_H_
#define _NY_ACC_STRATUS_H_

#ifdef __KERNEL__
    #include <linux/ioctl.h>
    #include <linux/types.h>
#else
    #include <sys/ioctl.h>
    #include <stdint.h>
    #ifndef __user
        #define __user
    #endif
#endif /* __KERNEL__ */

#include <esp.h>
#include <esp_accelerator.h>

#include "C_data.h"

struct ny_acc_stratus_access {
    struct esp_access esp;

    unsigned int batch;
    unsigned int num_load;
    unsigned int num_store;
    unsigned int delay;
    unsigned int src_dst_offset_0;
    unsigned int src_dst_offset_1;
    unsigned int src_dst_offset_2;
    unsigned int src_dst_offset_3;
    unsigned int src_dst_offset_4;

    unsigned int src_offset; // Input offset (bytes) used for P2P setup
    unsigned int dst_offset; // Output offset (bytes) used for P2P setup
};

#define NY_ACC_STRATUS_IOC_ACCESS _IOW('S', 0, struct ny_acc_stratus_access)

uint32_t i;

unsigned long long time_s;

struct timespec t_test_1, t_test_2;
struct timespec t_sw_1, t_sw_2;

#endif /* _NY_ACC_STRATUS_H_ */
