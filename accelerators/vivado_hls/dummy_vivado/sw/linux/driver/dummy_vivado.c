// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "dummy_vivado.h"

#define DRV_NAME	"dummy_vivado"

/* <<--regs-->> */
#define DUMMY_OUTPUT_REG 0x48
#define DUMMY_INPUT2_REG 0x44
#define DUMMY_INPUT1_REG 0x40

struct dummy_vivado_device {
	struct esp_device esp;
};

static struct esp_driver dummy_driver;

static struct of_device_id dummy_device_ids[] = {
	{
		.name = "SLD_DUMMY_VIVADO",
	},
	{
		.name = "eb_088",
	},
	{
		.compatible = "sld,dummy_vivado",
	},
	{ },
};

static int dummy_devs;

static inline struct dummy_vivado_device *to_dummy(struct esp_device *esp)
{
	return container_of(esp, struct dummy_vivado_device, esp);
}

static void dummy_prep_xfer(struct esp_device *esp, void *arg)
{
	struct dummy_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->output, esp->iomem + DUMMY_OUTPUT_REG);
	iowrite32be(a->input2, esp->iomem + DUMMY_INPUT2_REG);
	iowrite32be(a->input1, esp->iomem + DUMMY_INPUT1_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool dummy_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct dummy_vivado_device *dummy = to_dummy(esp); */
	/* struct dummy_vivado_access *a = arg; */

	return true;
}

static int dummy_probe(struct platform_device *pdev)
{
	struct dummy_vivado_device *dummy;
	struct esp_device *esp;
	int rc;

	dummy = kzalloc(sizeof(*dummy), GFP_KERNEL);
	if (dummy == NULL)
		return -ENOMEM;
	esp = &dummy->esp;
	esp->module = THIS_MODULE;
	esp->number = dummy_devs;
	esp->driver = &dummy_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	dummy_devs++;
	return 0;
 err:
	kfree(dummy);
	return rc;
}

static int __exit dummy_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct dummy_vivado_device *dummy = to_dummy(esp);

	esp_device_unregister(esp);
	kfree(dummy);
	return 0;
}

static struct esp_driver dummy_driver = {
	.plat = {
		.probe		= dummy_probe,
		.remove		= dummy_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = dummy_device_ids,
		},
	},
	.xfer_input_ok	= dummy_xfer_input_ok,
	.prep_xfer	= dummy_prep_xfer,
	.ioctl_cm	= DUMMY_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct dummy_vivado_access),
};

static int __init dummy_init(void)
{
	return esp_driver_register(&dummy_driver);
}

static void __exit dummy_exit(void)
{
	esp_driver_unregister(&dummy_driver);
}

module_init(dummy_init)
module_exit(dummy_exit)

MODULE_DEVICE_TABLE(of, dummy_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("dummy_vivado driver");
