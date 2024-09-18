// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough64_stratus.h"

#define DRV_NAME	"gemmenough64_stratus"

/* <<--regs-->> */
#define GEMMENOUGH64_NINPUTS_REG 0x58
#define GEMMENOUGH64_D3_REG 0x54
#define GEMMENOUGH64_D2_REG 0x50
#define GEMMENOUGH64_D1_REG 0x4c
#define GEMMENOUGH64_ST_OFFSET_REG 0x48
#define GEMMENOUGH64_LD_OFFSET1_REG 0x44
#define GEMMENOUGH64_LD_OFFSET2_REG 0x40

struct gemmenough64_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough64_driver;

static struct of_device_id gemmenough64_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH64_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough64_stratus",
	},
	{ },
};

static int gemmenough64_devs;

static inline struct gemmenough64_stratus_device *to_gemmenough64(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough64_stratus_device, esp);
}

static void gemmenough64_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough64_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH64_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH64_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH64_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH64_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH64_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH64_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH64_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough64_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough64_stratus_device *gemmenough64 = to_gemmenough64(esp); */
	/* struct gemmenough64_stratus_access *a = arg; */

	return true;
}

static int gemmenough64_probe(struct platform_device *pdev)
{
	struct gemmenough64_stratus_device *gemmenough64;
	struct esp_device *esp;
	int rc;

	gemmenough64 = kzalloc(sizeof(*gemmenough64), GFP_KERNEL);
	if (gemmenough64 == NULL)
		return -ENOMEM;
	esp = &gemmenough64->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough64_devs;
	esp->driver = &gemmenough64_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough64_devs++;
	return 0;
 err:
	kfree(gemmenough64);
	return rc;
}

static int __exit gemmenough64_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough64_stratus_device *gemmenough64 = to_gemmenough64(esp);

	esp_device_unregister(esp);
	kfree(gemmenough64);
	return 0;
}

static struct esp_driver gemmenough64_driver = {
	.plat = {
		.probe		= gemmenough64_probe,
		.remove		= gemmenough64_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough64_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough64_xfer_input_ok,
	.prep_xfer	= gemmenough64_prep_xfer,
	.ioctl_cm	= GEMMENOUGH64_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough64_stratus_access),
};

static int __init gemmenough64_init(void)
{
	return esp_driver_register(&gemmenough64_driver);
}

static void __exit gemmenough64_exit(void)
{
	esp_driver_unregister(&gemmenough64_driver);
}

module_init(gemmenough64_init)
module_exit(gemmenough64_exit)

MODULE_DEVICE_TABLE(of, gemmenough64_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough64_stratus driver");
