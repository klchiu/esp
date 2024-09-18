// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough256_stratus.h"

#define DRV_NAME	"gemmenough256_stratus"

/* <<--regs-->> */
#define GEMMENOUGH256_NINPUTS_REG 0x58
#define GEMMENOUGH256_D3_REG 0x54
#define GEMMENOUGH256_D2_REG 0x50
#define GEMMENOUGH256_D1_REG 0x4c
#define GEMMENOUGH256_ST_OFFSET_REG 0x48
#define GEMMENOUGH256_LD_OFFSET1_REG 0x44
#define GEMMENOUGH256_LD_OFFSET2_REG 0x40

struct gemmenough256_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough256_driver;

static struct of_device_id gemmenough256_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH256_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough256_stratus",
	},
	{ },
};

static int gemmenough256_devs;

static inline struct gemmenough256_stratus_device *to_gemmenough256(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough256_stratus_device, esp);
}

static void gemmenough256_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough256_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH256_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH256_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH256_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH256_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH256_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH256_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH256_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough256_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough256_stratus_device *gemmenough256 = to_gemmenough256(esp); */
	/* struct gemmenough256_stratus_access *a = arg; */

	return true;
}

static int gemmenough256_probe(struct platform_device *pdev)
{
	struct gemmenough256_stratus_device *gemmenough256;
	struct esp_device *esp;
	int rc;

	gemmenough256 = kzalloc(sizeof(*gemmenough256), GFP_KERNEL);
	if (gemmenough256 == NULL)
		return -ENOMEM;
	esp = &gemmenough256->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough256_devs;
	esp->driver = &gemmenough256_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough256_devs++;
	return 0;
 err:
	kfree(gemmenough256);
	return rc;
}

static int __exit gemmenough256_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough256_stratus_device *gemmenough256 = to_gemmenough256(esp);

	esp_device_unregister(esp);
	kfree(gemmenough256);
	return 0;
}

static struct esp_driver gemmenough256_driver = {
	.plat = {
		.probe		= gemmenough256_probe,
		.remove		= gemmenough256_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough256_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough256_xfer_input_ok,
	.prep_xfer	= gemmenough256_prep_xfer,
	.ioctl_cm	= GEMMENOUGH256_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough256_stratus_access),
};

static int __init gemmenough256_init(void)
{
	return esp_driver_register(&gemmenough256_driver);
}

static void __exit gemmenough256_exit(void)
{
	esp_driver_unregister(&gemmenough256_driver);
}

module_init(gemmenough256_init)
module_exit(gemmenough256_exit)

MODULE_DEVICE_TABLE(of, gemmenough256_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough256_stratus driver");
