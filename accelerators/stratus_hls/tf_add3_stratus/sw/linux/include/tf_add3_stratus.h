// Copyright (c) 2011-2023 Columbia University, System Level Design Group
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
	/* <<--regs-->> */
	unsigned tf_length;
	unsigned tf_src_dst_offset_0;	// output
	unsigned tf_src_dst_offset_1;	// input 1
	unsigned tf_src_dst_offset_2;	// input 2
	unsigned src_offset;
	unsigned dst_offset;
};

#define TF_ADD3_STRATUS_IOC_ACCESS	_IOW ('S', 0, struct tf_add3_stratus_access)


// Accelerator-specific data
uint32_t length;
uint32_t base_addr_0;
uint32_t base_addr_1;
uint32_t base_addr_2;

float *output_0;
float *input_1;
float *input_2;
float *gold_0;

#endif /* _TF_ADD3_STRATUS_H_ */
