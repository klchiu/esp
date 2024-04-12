// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _GEMM3_VIVADO_H_
#define _GEMM3_VIVADO_H_

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

struct gemm3_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned m3_offset;
	unsigned d3;
	unsigned d2;
	unsigned d1;
	unsigned m2_offset;
	unsigned m1_offset;
	unsigned src_offset;
	unsigned dst_offset;
};

#define GEMM3_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct gemm3_vivado_access)

#endif /* _GEMM3_VIVADO_H_ */
