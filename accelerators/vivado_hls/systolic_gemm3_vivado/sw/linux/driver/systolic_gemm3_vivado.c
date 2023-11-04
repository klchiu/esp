// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "systolic_gemm3_vivado.h"

#define DRV_NAME	"systolic_gemm3_vivado"

/* <<--regs-->> */
#define SYSTOLIC_GEMM3_MATRIX_C_DIM_REG 0x48
#define SYSTOLIC_GEMM3_MATRIX_A_DIM_REG 0x44
#define SYSTOLIC_GEMM3_MATRIX_B_DIM_REG 0x40

struct systolic_gemm3_vivado_device {
	struct esp_device esp;
};

static struct esp_driver systolic_gemm3_driver;

static struct of_device_id systolic_gemm3_device_ids[] = {
	{
		.name = "SLD_SYSTOLIC_GEMM3_VIVADO",
	},
	{
		.name = "eb_083",
	},
	{
		.compatible = "sld,systolic_gemm3_vivado",
	},
	{ },
};

static int systolic_gemm3_devs;

static inline struct systolic_gemm3_vivado_device *to_systolic_gemm3(struct esp_device *esp)
{
	return container_of(esp, struct systolic_gemm3_vivado_device, esp);
}

static void systolic_gemm3_prep_xfer(struct esp_device *esp, void *arg)
{
	struct systolic_gemm3_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->matrix_C_dim, esp->iomem + SYSTOLIC_GEMM3_MATRIX_C_DIM_REG);
	iowrite32be(a->matrix_A_dim, esp->iomem + SYSTOLIC_GEMM3_MATRIX_A_DIM_REG);
	iowrite32be(a->matrix_B_dim, esp->iomem + SYSTOLIC_GEMM3_MATRIX_B_DIM_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool systolic_gemm3_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct systolic_gemm3_vivado_device *systolic_gemm3 = to_systolic_gemm3(esp); */
	/* struct systolic_gemm3_vivado_access *a = arg; */

	return true;
}

static int systolic_gemm3_probe(struct platform_device *pdev)
{
	struct systolic_gemm3_vivado_device *systolic_gemm3;
	struct esp_device *esp;
	int rc;

	systolic_gemm3 = kzalloc(sizeof(*systolic_gemm3), GFP_KERNEL);
	if (systolic_gemm3 == NULL)
		return -ENOMEM;
	esp = &systolic_gemm3->esp;
	esp->module = THIS_MODULE;
	esp->number = systolic_gemm3_devs;
	esp->driver = &systolic_gemm3_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	systolic_gemm3_devs++;
	return 0;
 err:
	kfree(systolic_gemm3);
	return rc;
}

static int __exit systolic_gemm3_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct systolic_gemm3_vivado_device *systolic_gemm3 = to_systolic_gemm3(esp);

	esp_device_unregister(esp);
	kfree(systolic_gemm3);
	return 0;
}

static struct esp_driver systolic_gemm3_driver = {
	.plat = {
		.probe		= systolic_gemm3_probe,
		.remove		= systolic_gemm3_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = systolic_gemm3_device_ids,
		},
	},
	.xfer_input_ok	= systolic_gemm3_xfer_input_ok,
	.prep_xfer	= systolic_gemm3_prep_xfer,
	.ioctl_cm	= SYSTOLIC_GEMM3_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct systolic_gemm3_vivado_access),
};

static int __init systolic_gemm3_init(void)
{
	return esp_driver_register(&systolic_gemm3_driver);
}

static void __exit systolic_gemm3_exit(void)
{
	esp_driver_unregister(&systolic_gemm3_driver);
}

module_init(systolic_gemm3_init)
module_exit(systolic_gemm3_exit)

MODULE_DEVICE_TABLE(of, systolic_gemm3_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("systolic_gemm3_vivado driver");
