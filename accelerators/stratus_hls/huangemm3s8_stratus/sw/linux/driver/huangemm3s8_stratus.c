// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "huangemm3s8_stratus.h"

#define DRV_NAME	"huangemm3s8_stratus"

/* <<--regs-->> */
// #define HUANGEMM3S8_HUANGEMM3S8_N_REG 0x48
// #define HUANGEMM3S8_HUANGEMM3S8_VEC_REG 0x44
// #define HUANGEMM3S8_HUANGEMM3S8_LEN_REG 0x40

#define HUANGEMM3S8_ROWS_REG 0x48
#define HUANGEMM3S8_COLS_REG 0x44
#define HUANGEMM3S8_LOADED_COLS_REG 0x40

struct huangemm3s8_stratus_device {
	struct esp_device esp;
};

static struct esp_driver huangemm3s8_driver;

static struct of_device_id huangemm3s8_device_ids[] = {
	{
		.name = "SLD_HUANGEMM3S8_STRATUS",
	},
	{
		.name = "eb_231",
	},
	{
		.compatible = "sld,huangemm3s8_stratus",
	},
	{ },
};

static int huangemm3s8_devs;

static inline struct huangemm3s8_stratus_device *to_huangemm3s8(struct esp_device *esp)
{
	return container_of(esp, struct huangemm3s8_stratus_device, esp);
}

static void huangemm3s8_prep_xfer(struct esp_device *esp, void *arg)
{
	struct huangemm3s8_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->rows, esp->iomem + HUANGEMM3S8_ROWS_REG);
	iowrite32be(a->cols, esp->iomem + HUANGEMM3S8_COLS_REG);
	iowrite32be(a->loaded_cols, esp->iomem + HUANGEMM3S8_LOADED_COLS_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool huangemm3s8_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct huangemm3s8_stratus_device *huangemm3s8 = to_huangemm3s8(esp); */
	/* struct huangemm3s8_stratus_access *a = arg; */

	return true;
}

static int huangemm3s8_probe(struct platform_device *pdev)
{
	struct huangemm3s8_stratus_device *huangemm3s8;
	struct esp_device *esp;
	int rc;

	huangemm3s8 = kzalloc(sizeof(*huangemm3s8), GFP_KERNEL);
	if (huangemm3s8 == NULL)
		return -ENOMEM;
	esp = &huangemm3s8->esp;
	esp->module = THIS_MODULE;
	esp->number = huangemm3s8_devs;
	esp->driver = &huangemm3s8_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	huangemm3s8_devs++;
	return 0;
 err:
	kfree(huangemm3s8);
	return rc;
}

static int __exit huangemm3s8_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct huangemm3s8_stratus_device *huangemm3s8 = to_huangemm3s8(esp);

	esp_device_unregister(esp);
	kfree(huangemm3s8);
	return 0;
}

static struct esp_driver huangemm3s8_driver = {
	.plat = {
		.probe		= huangemm3s8_probe,
		.remove		= huangemm3s8_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = huangemm3s8_device_ids,
		},
	},
	.xfer_input_ok	= huangemm3s8_xfer_input_ok,
	.prep_xfer	= huangemm3s8_prep_xfer,
	.ioctl_cm	= HUANGEMM3S8_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct huangemm3s8_stratus_access),
};

static int __init huangemm3s8_init(void)
{
	return esp_driver_register(&huangemm3s8_driver);
}

static void __exit huangemm3s8_exit(void)
{
	esp_driver_unregister(&huangemm3s8_driver);
}

module_init(huangemm3s8_init)
module_exit(huangemm3s8_exit)

MODULE_DEVICE_TABLE(of, huangemm3s8_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("huangemm3s8_stratus driver");
