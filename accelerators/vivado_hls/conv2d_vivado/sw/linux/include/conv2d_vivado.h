// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#ifndef _CONV2D_VIVADO_H_
#define _CONV2D_VIVADO_H_

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

struct conv2d_vivado_access {
	struct esp_access esp;
	/* <<--regs-->> */
	unsigned do_relu;
	unsigned stride;
	unsigned feature_map_width;
	unsigned n_channels;
	unsigned n_filters;
	unsigned batch_size;
	unsigned filter_dim;
	unsigned is_padded;
	unsigned pool_type;
	unsigned feature_map_height;
	unsigned src_offset;
	unsigned dst_offset;
};

#define CONV2D_VIVADO_IOC_ACCESS	_IOW ('S', 0, struct conv2d_vivado_access)

#endif /* _CONV2D_VIVADO_H_ */
