// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough128_stratus.h"

#define DRV_NAME	"gemmenough128_stratus"

/* <<--regs-->> */
#define GEMMENOUGH128_NINPUTS_REG 0x58
#define GEMMENOUGH128_D3_REG 0x54
#define GEMMENOUGH128_D2_REG 0x50
#define GEMMENOUGH128_D1_REG 0x4c
#define GEMMENOUGH128_ST_OFFSET_REG 0x48
#define GEMMENOUGH128_LD_OFFSET1_REG 0x44
#define GEMMENOUGH128_LD_OFFSET2_REG 0x40

struct gemmenough128_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough128_driver;

static struct of_device_id gemmenough128_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH128_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough128_stratus",
	},
	{ },
};

static int gemmenough128_devs;

static inline struct gemmenough128_stratus_device *to_gemmenough128(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough128_stratus_device, esp);
}

static void gemmenough128_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough128_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH128_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH128_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH128_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH128_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH128_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH128_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH128_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough128_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough128_stratus_device *gemmenough128 = to_gemmenough128(esp); */
	/* struct gemmenough128_stratus_access *a = arg; */

	return true;
}

static int gemmenough128_probe(struct platform_device *pdev)
{
	struct gemmenough128_stratus_device *gemmenough128;
	struct esp_device *esp;
	int rc;

	gemmenough128 = kzalloc(sizeof(*gemmenough128), GFP_KERNEL);
	if (gemmenough128 == NULL)
		return -ENOMEM;
	esp = &gemmenough128->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough128_devs;
	esp->driver = &gemmenough128_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough128_devs++;
	return 0;
 err:
	kfree(gemmenough128);
	return rc;
}

static int __exit gemmenough128_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough128_stratus_device *gemmenough128 = to_gemmenough128(esp);

	esp_device_unregister(esp);
	kfree(gemmenough128);
	return 0;
}

static struct esp_driver gemmenough128_driver = {
	.plat = {
		.probe		= gemmenough128_probe,
		.remove		= gemmenough128_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough128_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough128_xfer_input_ok,
	.prep_xfer	= gemmenough128_prep_xfer,
	.ioctl_cm	= GEMMENOUGH128_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough128_stratus_access),
};

static int __init gemmenough128_init(void)
{
	return esp_driver_register(&gemmenough128_driver);
}

static void __exit gemmenough128_exit(void)
{
	esp_driver_unregister(&gemmenough128_driver);
}

module_init(gemmenough128_init)
module_exit(gemmenough128_exit)

MODULE_DEVICE_TABLE(of, gemmenough128_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough128_stratus driver");
