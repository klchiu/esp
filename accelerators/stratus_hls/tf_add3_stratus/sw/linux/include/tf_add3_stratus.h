// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _TF_ADD3_STRATUS_H_
#define _TF_ADD3_STRATUS_H_

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

struct tf_add3_stratus_access {
    struct esp_access esp;

    unsigned int wami_num_img;
    unsigned int wami_num_col;
    unsigned int wami_num_row;
    unsigned int wami_pad;
    unsigned int wami_kern_id;
    unsigned int wami_batch;
    unsigned int wami_src_dst_offset_0;
    unsigned int wami_src_dst_offset_1;
    unsigned int wami_src_dst_offset_2;
    unsigned int wami_src_dst_offset_3;
    unsigned int wami_src_dst_offset_4;
    unsigned int wami_is_p2p;
    unsigned int wami_p2p_config_0;
    unsigned int wami_p2p_config_1;

    unsigned int src_offset; // Input offset (bytes) used for P2P setup
    unsigned int dst_offset; // Output offset (bytes) used for P2P setup
};

#define TF_ADD3_STRATUS_IOC_ACCESS _IOW('S', 0, struct tf_add3_stratus_access)

#endif /* _TF_ADD3_STRATUS_H_ */