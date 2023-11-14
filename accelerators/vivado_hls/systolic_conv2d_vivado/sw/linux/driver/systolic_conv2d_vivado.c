// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "systolic_conv2d_vivado.h"

#define DRV_NAME	"systolic_conv2d_vivado"

/* <<--regs-->> */
#define SYSTOLIC_CONV2D_MATRIX_C_DIM_REG 0x48
#define SYSTOLIC_CONV2D_MATRIX_A_DIM_REG 0x44
#define SYSTOLIC_CONV2D_MATRIX_B_DIM_REG 0x40

struct systolic_conv2d_vivado_device {
	struct esp_device esp;
};

static struct esp_driver systolic_conv2d_driver;

static struct of_device_id systolic_conv2d_device_ids[] = {
	{
		.name = "SLD_SYSTOLIC_CONV2D_VIVADO",
	},
	{
		.name = "eb_087",
	},
	{
		.compatible = "sld,systolic_conv2d_vivado",
	},
	{ },
};

static int systolic_conv2d_devs;

static inline struct systolic_conv2d_vivado_device *to_systolic_conv2d(struct esp_device *esp)
{
	return container_of(esp, struct systolic_conv2d_vivado_device, esp);
}

static void systolic_conv2d_prep_xfer(struct esp_device *esp, void *arg)
{
	struct systolic_conv2d_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->matrix_C_dim, esp->iomem + SYSTOLIC_CONV2D_MATRIX_C_DIM_REG);
	iowrite32be(a->matrix_A_dim, esp->iomem + SYSTOLIC_CONV2D_MATRIX_A_DIM_REG);
	iowrite32be(a->matrix_B_dim, esp->iomem + SYSTOLIC_CONV2D_MATRIX_B_DIM_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool systolic_conv2d_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct systolic_conv2d_vivado_device *systolic_conv2d = to_systolic_conv2d(esp); */
	/* struct systolic_conv2d_vivado_access *a = arg; */

	return true;
}

static int systolic_conv2d_probe(struct platform_device *pdev)
{
	struct systolic_conv2d_vivado_device *systolic_conv2d;
	struct esp_device *esp;
	int rc;

	systolic_conv2d = kzalloc(sizeof(*systolic_conv2d), GFP_KERNEL);
	if (systolic_conv2d == NULL)
		return -ENOMEM;
	esp = &systolic_conv2d->esp;
	esp->module = THIS_MODULE;
	esp->number = systolic_conv2d_devs;
	esp->driver = &systolic_conv2d_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	systolic_conv2d_devs++;
	return 0;
 err:
	kfree(systolic_conv2d);
	return rc;
}

static int __exit systolic_conv2d_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct systolic_conv2d_vivado_device *systolic_conv2d = to_systolic_conv2d(esp);

	esp_device_unregister(esp);
	kfree(systolic_conv2d);
	return 0;
}

static struct esp_driver systolic_conv2d_driver = {
	.plat = {
		.probe		= systolic_conv2d_probe,
		.remove		= systolic_conv2d_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = systolic_conv2d_device_ids,
		},
	},
	.xfer_input_ok	= systolic_conv2d_xfer_input_ok,
	.prep_xfer	= systolic_conv2d_prep_xfer,
	.ioctl_cm	= SYSTOLIC_CONV2D_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct systolic_conv2d_vivado_access),
};

static int __init systolic_conv2d_init(void)
{
	return esp_driver_register(&systolic_conv2d_driver);
}

static void __exit systolic_conv2d_exit(void)
{
	esp_driver_unregister(&systolic_conv2d_driver);
}

module_init(systolic_conv2d_init)
module_exit(systolic_conv2d_exit)

MODULE_DEVICE_TABLE(of, systolic_conv2d_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("systolic_conv2d_vivado driver");
