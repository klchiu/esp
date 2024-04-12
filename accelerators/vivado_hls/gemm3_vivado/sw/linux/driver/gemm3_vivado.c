// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemm3_vivado.h"

#define DRV_NAME	"gemm3_vivado"

/* <<--regs-->> */
#define GEMM3_M3_OFFSET_REG 0x54
#define GEMM3_D3_REG 0x50
#define GEMM3_D2_REG 0x4c
#define GEMM3_D1_REG 0x48
#define GEMM3_M2_OFFSET_REG 0x44
#define GEMM3_M1_OFFSET_REG 0x40

struct gemm3_vivado_device {
	struct esp_device esp;
};

static struct esp_driver gemm3_driver;

static struct of_device_id gemm3_device_ids[] = {
	{
		.name = "SLD_GEMM3_VIVADO",
	},
	{
		.name = "eb_093",
	},
	{
		.compatible = "sld,gemm3_vivado",
	},
	{ },
};

static int gemm3_devs;

static inline struct gemm3_vivado_device *to_gemm3(struct esp_device *esp)
{
	return container_of(esp, struct gemm3_vivado_device, esp);
}

static void gemm3_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemm3_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->m3_offset, esp->iomem + GEMM3_M3_OFFSET_REG);
	iowrite32be(a->d3, esp->iomem + GEMM3_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMM3_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMM3_D1_REG);
	iowrite32be(a->m2_offset, esp->iomem + GEMM3_M2_OFFSET_REG);
	iowrite32be(a->m1_offset, esp->iomem + GEMM3_M1_OFFSET_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemm3_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemm3_vivado_device *gemm3 = to_gemm3(esp); */
	/* struct gemm3_vivado_access *a = arg; */

	return true;
}

static int gemm3_probe(struct platform_device *pdev)
{
	struct gemm3_vivado_device *gemm3;
	struct esp_device *esp;
	int rc;

	gemm3 = kzalloc(sizeof(*gemm3), GFP_KERNEL);
	if (gemm3 == NULL)
		return -ENOMEM;
	esp = &gemm3->esp;
	esp->module = THIS_MODULE;
	esp->number = gemm3_devs;
	esp->driver = &gemm3_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemm3_devs++;
	return 0;
 err:
	kfree(gemm3);
	return rc;
}

static int __exit gemm3_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemm3_vivado_device *gemm3 = to_gemm3(esp);

	esp_device_unregister(esp);
	kfree(gemm3);
	return 0;
}

static struct esp_driver gemm3_driver = {
	.plat = {
		.probe		= gemm3_probe,
		.remove		= gemm3_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemm3_device_ids,
		},
	},
	.xfer_input_ok	= gemm3_xfer_input_ok,
	.prep_xfer	= gemm3_prep_xfer,
	.ioctl_cm	= GEMM3_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct gemm3_vivado_access),
};

static int __init gemm3_init(void)
{
	return esp_driver_register(&gemm3_driver);
}

static void __exit gemm3_exit(void)
{
	esp_driver_unregister(&gemm3_driver);
}

module_init(gemm3_init)
module_exit(gemm3_exit)

MODULE_DEVICE_TABLE(of, gemm3_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemm3_vivado driver");
