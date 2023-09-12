// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "birukee_rtl.h"

#define DRV_NAME	"birukee_rtl"

/* <<--regs-->> */
#define BIRUKEE_OUTPUT_REG 0x48
#define BIRUKEE_INPUT2_REG 0x44
#define BIRUKEE_INPUT1_REG 0x40

struct birukee_rtl_device {
	struct esp_device esp;
};

static struct esp_driver birukee_driver;

static struct of_device_id birukee_device_ids[] = {
	{
		.name = "SLD_BIRUKEE_RTL",
	},
	{
		.name = "eb_087",
	},
	{
		.compatible = "sld,birukee_rtl",
	},
	{ },
};

static int birukee_devs;

static inline struct birukee_rtl_device *to_birukee(struct esp_device *esp)
{
	return container_of(esp, struct birukee_rtl_device, esp);
}

static void birukee_prep_xfer(struct esp_device *esp, void *arg)
{
	struct birukee_rtl_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->output, esp->iomem + BIRUKEE_OUTPUT_REG);
	iowrite32be(a->input2, esp->iomem + BIRUKEE_INPUT2_REG);
	iowrite32be(a->input1, esp->iomem + BIRUKEE_INPUT1_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool birukee_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct birukee_rtl_device *birukee = to_birukee(esp); */
	/* struct birukee_rtl_access *a = arg; */

	return true;
}

static int birukee_probe(struct platform_device *pdev)
{
	struct birukee_rtl_device *birukee;
	struct esp_device *esp;
	int rc;

	birukee = kzalloc(sizeof(*birukee), GFP_KERNEL);
	if (birukee == NULL)
		return -ENOMEM;
	esp = &birukee->esp;
	esp->module = THIS_MODULE;
	esp->number = birukee_devs;
	esp->driver = &birukee_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	birukee_devs++;
	return 0;
 err:
	kfree(birukee);
	return rc;
}

static int __exit birukee_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct birukee_rtl_device *birukee = to_birukee(esp);

	esp_device_unregister(esp);
	kfree(birukee);
	return 0;
}

static struct esp_driver birukee_driver = {
	.plat = {
		.probe		= birukee_probe,
		.remove		= birukee_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = birukee_device_ids,
		},
	},
	.xfer_input_ok	= birukee_xfer_input_ok,
	.prep_xfer	= birukee_prep_xfer,
	.ioctl_cm	= BIRUKEE_RTL_IOC_ACCESS,
	.arg_size	= sizeof(struct birukee_rtl_access),
};

static int __init birukee_init(void)
{
	return esp_driver_register(&birukee_driver);
}

static void __exit birukee_exit(void)
{
	esp_driver_unregister(&birukee_driver);
}

module_init(birukee_init)
module_exit(birukee_exit)

MODULE_DEVICE_TABLE(of, birukee_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("birukee_rtl driver");
