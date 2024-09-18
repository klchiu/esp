// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough2048_stratus.h"

#define DRV_NAME	"gemmenough2048_stratus"

/* <<--regs-->> */
#define GEMMENOUGH2048_NINPUTS_REG 0x58
#define GEMMENOUGH2048_D3_REG 0x54
#define GEMMENOUGH2048_D2_REG 0x50
#define GEMMENOUGH2048_D1_REG 0x4c
#define GEMMENOUGH2048_ST_OFFSET_REG 0x48
#define GEMMENOUGH2048_LD_OFFSET1_REG 0x44
#define GEMMENOUGH2048_LD_OFFSET2_REG 0x40

struct gemmenough2048_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough2048_driver;

static struct of_device_id gemmenough2048_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH2048_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough2048_stratus",
	},
	{ },
};

static int gemmenough2048_devs;

static inline struct gemmenough2048_stratus_device *to_gemmenough2048(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough2048_stratus_device, esp);
}

static void gemmenough2048_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough2048_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH2048_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH2048_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH2048_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH2048_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH2048_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH2048_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH2048_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough2048_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough2048_stratus_device *gemmenough2048 = to_gemmenough2048(esp); */
	/* struct gemmenough2048_stratus_access *a = arg; */

	return true;
}

static int gemmenough2048_probe(struct platform_device *pdev)
{
	struct gemmenough2048_stratus_device *gemmenough2048;
	struct esp_device *esp;
	int rc;

	gemmenough2048 = kzalloc(sizeof(*gemmenough2048), GFP_KERNEL);
	if (gemmenough2048 == NULL)
		return -ENOMEM;
	esp = &gemmenough2048->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough2048_devs;
	esp->driver = &gemmenough2048_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough2048_devs++;
	return 0;
 err:
	kfree(gemmenough2048);
	return rc;
}

static int __exit gemmenough2048_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough2048_stratus_device *gemmenough2048 = to_gemmenough2048(esp);

	esp_device_unregister(esp);
	kfree(gemmenough2048);
	return 0;
}

static struct esp_driver gemmenough2048_driver = {
	.plat = {
		.probe		= gemmenough2048_probe,
		.remove		= gemmenough2048_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough2048_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough2048_xfer_input_ok,
	.prep_xfer	= gemmenough2048_prep_xfer,
	.ioctl_cm	= GEMMENOUGH2048_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough2048_stratus_access),
};

static int __init gemmenough2048_init(void)
{
	return esp_driver_register(&gemmenough2048_driver);
}

static void __exit gemmenough2048_exit(void)
{
	esp_driver_unregister(&gemmenough2048_driver);
}

module_init(gemmenough2048_init)
module_exit(gemmenough2048_exit)

MODULE_DEVICE_TABLE(of, gemmenough2048_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough2048_stratus driver");
