// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough1024_stratus.h"

#define DRV_NAME	"gemmenough1024_stratus"

/* <<--regs-->> */
#define GEMMENOUGH1024_NINPUTS_REG 0x58
#define GEMMENOUGH1024_D3_REG 0x54
#define GEMMENOUGH1024_D2_REG 0x50
#define GEMMENOUGH1024_D1_REG 0x4c
#define GEMMENOUGH1024_ST_OFFSET_REG 0x48
#define GEMMENOUGH1024_LD_OFFSET1_REG 0x44
#define GEMMENOUGH1024_LD_OFFSET2_REG 0x40

struct gemmenough1024_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough1024_driver;

static struct of_device_id gemmenough1024_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH1024_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough1024_stratus",
	},
	{ },
};

static int gemmenough1024_devs;

static inline struct gemmenough1024_stratus_device *to_gemmenough1024(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough1024_stratus_device, esp);
}

static void gemmenough1024_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough1024_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH1024_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH1024_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH1024_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH1024_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH1024_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH1024_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH1024_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough1024_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough1024_stratus_device *gemmenough1024 = to_gemmenough1024(esp); */
	/* struct gemmenough1024_stratus_access *a = arg; */

	return true;
}

static int gemmenough1024_probe(struct platform_device *pdev)
{
	struct gemmenough1024_stratus_device *gemmenough1024;
	struct esp_device *esp;
	int rc;

	gemmenough1024 = kzalloc(sizeof(*gemmenough1024), GFP_KERNEL);
	if (gemmenough1024 == NULL)
		return -ENOMEM;
	esp = &gemmenough1024->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough1024_devs;
	esp->driver = &gemmenough1024_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough1024_devs++;
	return 0;
 err:
	kfree(gemmenough1024);
	return rc;
}

static int __exit gemmenough1024_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough1024_stratus_device *gemmenough1024 = to_gemmenough1024(esp);

	esp_device_unregister(esp);
	kfree(gemmenough1024);
	return 0;
}

static struct esp_driver gemmenough1024_driver = {
	.plat = {
		.probe		= gemmenough1024_probe,
		.remove		= gemmenough1024_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough1024_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough1024_xfer_input_ok,
	.prep_xfer	= gemmenough1024_prep_xfer,
	.ioctl_cm	= GEMMENOUGH1024_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough1024_stratus_access),
};

static int __init gemmenough1024_init(void)
{
	return esp_driver_register(&gemmenough1024_driver);
}

static void __exit gemmenough1024_exit(void)
{
	esp_driver_unregister(&gemmenough1024_driver);
}

module_init(gemmenough1024_init)
module_exit(gemmenough1024_exit)

MODULE_DEVICE_TABLE(of, gemmenough1024_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough1024_stratus driver");
