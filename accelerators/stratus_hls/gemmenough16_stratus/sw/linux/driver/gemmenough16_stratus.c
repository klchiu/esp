// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough16_stratus.h"

#define DRV_NAME	"gemmenough16_stratus"

/* <<--regs-->> */
#define GEMMENOUGH16_NINPUTS_REG 0x58
#define GEMMENOUGH16_D3_REG 0x54
#define GEMMENOUGH16_D2_REG 0x50
#define GEMMENOUGH16_D1_REG 0x4c
#define GEMMENOUGH16_ST_OFFSET_REG 0x48
#define GEMMENOUGH16_LD_OFFSET1_REG 0x44
#define GEMMENOUGH16_LD_OFFSET2_REG 0x40

struct gemmenough16_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough16_driver;

static struct of_device_id gemmenough16_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH16_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough16_stratus",
	},
	{ },
};

static int gemmenough16_devs;

static inline struct gemmenough16_stratus_device *to_gemmenough16(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough16_stratus_device, esp);
}

static void gemmenough16_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough16_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH16_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH16_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH16_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH16_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH16_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH16_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH16_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough16_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough16_stratus_device *gemmenough16 = to_gemmenough16(esp); */
	/* struct gemmenough16_stratus_access *a = arg; */

	return true;
}

static int gemmenough16_probe(struct platform_device *pdev)
{
	struct gemmenough16_stratus_device *gemmenough16;
	struct esp_device *esp;
	int rc;

	gemmenough16 = kzalloc(sizeof(*gemmenough16), GFP_KERNEL);
	if (gemmenough16 == NULL)
		return -ENOMEM;
	esp = &gemmenough16->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough16_devs;
	esp->driver = &gemmenough16_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough16_devs++;
	return 0;
 err:
	kfree(gemmenough16);
	return rc;
}

static int __exit gemmenough16_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough16_stratus_device *gemmenough16 = to_gemmenough16(esp);

	esp_device_unregister(esp);
	kfree(gemmenough16);
	return 0;
}

static struct esp_driver gemmenough16_driver = {
	.plat = {
		.probe		= gemmenough16_probe,
		.remove		= gemmenough16_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough16_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough16_xfer_input_ok,
	.prep_xfer	= gemmenough16_prep_xfer,
	.ioctl_cm	= GEMMENOUGH16_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough16_stratus_access),
};

static int __init gemmenough16_init(void)
{
	return esp_driver_register(&gemmenough16_driver);
}

static void __exit gemmenough16_exit(void)
{
	esp_driver_unregister(&gemmenough16_driver);
}

module_init(gemmenough16_init)
module_exit(gemmenough16_exit)

MODULE_DEVICE_TABLE(of, gemmenough16_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough16_stratus driver");
