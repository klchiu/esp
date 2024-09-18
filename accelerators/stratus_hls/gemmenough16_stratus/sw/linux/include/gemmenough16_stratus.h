// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _GEMMENOUGH16_STRATUS_H_
#define _GEMMENOUGH16_STRATUS_H_

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

struct gemmenough16_stratus_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned ninputs;
	unsigned d3;
	unsigned d2;
	unsigned d1;
	unsigned st_offset;
	unsigned ld_offset1;
	unsigned ld_offset2;
	unsigned src_offset;
	unsigned dst_offset;
};

#define GEMMENOUGH16_STRATUS_IOC_ACCESS	_IOW ('S', 0, struct gemmenough16_stratus_access)

#endif /* _GEMMENOUGH16_STRATUS_H_ */
