// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _SYSTOLIC_CONV2D_VIVADO_H_
#define _SYSTOLIC_CONV2D_VIVADO_H_

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

struct systolic_conv2d_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned matrix_C_dim;
	unsigned matrix_A_dim;
	unsigned matrix_B_dim;
	unsigned src_offset;
	unsigned dst_offset;
};

#define SYSTOLIC_CONV2D_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct systolic_conv2d_vivado_access)

#endif /* _SYSTOLIC_CONV2D_VIVADO_H_ */
