// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmenough512_stratus.h"

#define DRV_NAME	"gemmenough512_stratus"

/* <<--regs-->> */
#define GEMMENOUGH512_NINPUTS_REG 0x58
#define GEMMENOUGH512_D3_REG 0x54
#define GEMMENOUGH512_D2_REG 0x50
#define GEMMENOUGH512_D1_REG 0x4c
#define GEMMENOUGH512_ST_OFFSET_REG 0x48
#define GEMMENOUGH512_LD_OFFSET1_REG 0x44
#define GEMMENOUGH512_LD_OFFSET2_REG 0x40

struct gemmenough512_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmenough512_driver;

static struct of_device_id gemmenough512_device_ids[] = {
	{
		.name = "SLD_GEMMENOUGH512_STRATUS",
	},
	{
		.name = "eb_201",
	},
	{
		.compatible = "sld,gemmenough512_stratus",
	},
	{ },
};

static int gemmenough512_devs;

static inline struct gemmenough512_stratus_device *to_gemmenough512(struct esp_device *esp)
{
	return container_of(esp, struct gemmenough512_stratus_device, esp);
}

static void gemmenough512_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmenough512_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->ninputs, esp->iomem + GEMMENOUGH512_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMENOUGH512_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMENOUGH512_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMENOUGH512_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMENOUGH512_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMENOUGH512_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMENOUGH512_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool gemmenough512_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmenough512_stratus_device *gemmenough512 = to_gemmenough512(esp); */
	/* struct gemmenough512_stratus_access *a = arg; */

	return true;
}

static int gemmenough512_probe(struct platform_device *pdev)
{
	struct gemmenough512_stratus_device *gemmenough512;
	struct esp_device *esp;
	int rc;

	gemmenough512 = kzalloc(sizeof(*gemmenough512), GFP_KERNEL);
	if (gemmenough512 == NULL)
		return -ENOMEM;
	esp = &gemmenough512->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmenough512_devs;
	esp->driver = &gemmenough512_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmenough512_devs++;
	return 0;
 err:
	kfree(gemmenough512);
	return rc;
}

static int __exit gemmenough512_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmenough512_stratus_device *gemmenough512 = to_gemmenough512(esp);

	esp_device_unregister(esp);
	kfree(gemmenough512);
	return 0;
}

static struct esp_driver gemmenough512_driver = {
	.plat = {
		.probe		= gemmenough512_probe,
		.remove		= gemmenough512_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmenough512_device_ids,
		},
	},
	.xfer_input_ok	= gemmenough512_xfer_input_ok,
	.prep_xfer	= gemmenough512_prep_xfer,
	.ioctl_cm	= GEMMENOUGH512_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmenough512_stratus_access),
};

static int __init gemmenough512_init(void)
{
	return esp_driver_register(&gemmenough512_driver);
}

static void __exit gemmenough512_exit(void)
{
	esp_driver_unregister(&gemmenough512_driver);
}

module_init(gemmenough512_init)
module_exit(gemmenough512_exit)

MODULE_DEVICE_TABLE(of, gemmenough512_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmenough512_stratus driver");
