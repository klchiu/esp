// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough32_stratus.h"

#define DRV_NAME	"gemmenough32_stratus"

/* <<--regs-->> */
#define GEMMENOUGH32_NINPUTS_REG 0x58
#define GEMMENOUGH32_D3_REG 0x54
#define GEMMENOUGH32_D2_REG 0x50
#define GEMMENOUGH32_D1_REG 0x4c
#define GEMMENOUGH32_ST_OFFSET_REG 0x48
#define GEMMENOUGH32_LD_OFFSET1_REG 0x44
#define GEMMENOUGH32_LD_OFFSET2_REG 0x40

struct gemmenough32_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough32_driver;

static struct of_device_id gemmenough32_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH32_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough32_stratus",
	},
	{ },
};

static int gemmenough32_devs;

static inline struct gemmenough32_stratus_device *to_gemmenough32(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough32_stratus_device, esp);
}

static void gemmenough32_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough32_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH32_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH32_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH32_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH32_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH32_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH32_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH32_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough32_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough32_stratus_device *gemmenough32 = to_gemmenough32(esp); */
	/* struct gemmenough32_stratus_access *a = arg; */

	return true;
}

static int gemmenough32_probe(struct platform_device *pdev)
{
	struct gemmenough32_stratus_device *gemmenough32;
	struct esp_device *esp;
	int rc;

	gemmenough32 = kzalloc(sizeof(*gemmenough32), GFP_KERNEL);
	if (gemmenough32 == NULL)
		return -ENOMEM;
	esp = &gemmenough32->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough32_devs;
	esp->driver = &gemmenough32_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough32_devs++;
	return 0;
 err:
	kfree(gemmenough32);
	return rc;
}

static int __exit gemmenough32_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough32_stratus_device *gemmenough32 = to_gemmenough32(esp);

	esp_device_unregister(esp);
	kfree(gemmenough32);
	return 0;
}

static struct esp_driver gemmenough32_driver = {
	.plat = {
		.probe		= gemmenough32_probe,
		.remove		= gemmenough32_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough32_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough32_xfer_input_ok,
	.prep_xfer	= gemmenough32_prep_xfer,
	.ioctl_cm	= GEMMENOUGH32_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough32_stratus_access),
};

static int __init gemmenough32_init(void)
{
	return esp_driver_register(&gemmenough32_driver);
}

static void __exit gemmenough32_exit(void)
{
	esp_driver_unregister(&gemmenough32_driver);
}

module_init(gemmenough32_init)
module_exit(gemmenough32_exit)

MODULE_DEVICE_TABLE(of, gemmenough32_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough32_stratus driver");
