// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough8_stratus.h"

#define DRV_NAME	"gemmenough8_stratus"

/* <<--regs-->> */
#define GEMMENOUGH8_NINPUTS_REG 0x58
#define GEMMENOUGH8_D3_REG 0x54
#define GEMMENOUGH8_D2_REG 0x50
#define GEMMENOUGH8_D1_REG 0x4c
#define GEMMENOUGH8_ST_OFFSET_REG 0x48
#define GEMMENOUGH8_LD_OFFSET1_REG 0x44
#define GEMMENOUGH8_LD_OFFSET2_REG 0x40

struct gemmenough8_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough8_driver;

static struct of_device_id gemmenough8_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH8_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough8_stratus",
	},
	{ },
};

static int gemmenough8_devs;

static inline struct gemmenough8_stratus_device *to_gemmenough8(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough8_stratus_device, esp);
}

static void gemmenough8_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough8_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH8_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH8_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH8_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH8_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH8_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH8_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH8_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough8_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough8_stratus_device *gemmenough8 = to_gemmenough8(esp); */
	/* struct gemmenough8_stratus_access *a = arg; */

	return true;
}

static int gemmenough8_probe(struct platform_device *pdev)
{
	struct gemmenough8_stratus_device *gemmenough8;
	struct esp_device *esp;
	int rc;

	gemmenough8 = kzalloc(sizeof(*gemmenough8), GFP_KERNEL);
	if (gemmenough8 == NULL)
		return -ENOMEM;
	esp = &gemmenough8->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough8_devs;
	esp->driver = &gemmenough8_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough8_devs++;
	return 0;
 err:
	kfree(gemmenough8);
	return rc;
}

static int __exit gemmenough8_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough8_stratus_device *gemmenough8 = to_gemmenough8(esp);

	esp_device_unregister(esp);
	kfree(gemmenough8);
	return 0;
}

static struct esp_driver gemmenough8_driver = {
	.plat = {
		.probe		= gemmenough8_probe,
		.remove		= gemmenough8_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough8_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough8_xfer_input_ok,
	.prep_xfer	= gemmenough8_prep_xfer,
	.ioctl_cm	= GEMMENOUGH8_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough8_stratus_access),
};

static int __init gemmenough8_init(void)
{
	return esp_driver_register(&gemmenough8_driver);
}

static void __exit gemmenough8_exit(void)
{
	esp_driver_unregister(&gemmenough8_driver);
}

module_init(gemmenough8_init)
module_exit(gemmenough8_exit)

MODULE_DEVICE_TABLE(of, gemmenough8_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough8_stratus driver");
