// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "huangemm3s10000_stratus.h"

#define DRV_NAME	"huangemm3s10000_stratus"

/* <<--regs-->> */
// #define HUANGEMM3s10000_HUANGEMM3s10000_N_REG 0x48
// #define HUANGEMM3s10000_HUANGEMM3s10000_VEC_REG 0x44
// #define HUANGEMM3s10000_HUANGEMM3s10000_LEN_REG 0x40

#define HUANGEMM3s10000_ROWS_REG 0x48
#define HUANGEMM3s10000_COLS_REG 0x44
#define HUANGEMM3s10000_LOADED_COLS_REG 0x40

struct huangemm3s10000_stratus_device {
	struct esp_device esp;
};

static struct esp_driver huangemm3s10000_driver;

static struct of_device_id huangemm3s10000_device_ids[] = {
	{
		.name = "SLD_HUANGEMM3s10000_STRATUS",
	},
	{
		.name = "eb_088",
	},
	{
		.compatible = "sld,huangemm3s10000_stratus",
	},
	{ },
};

static int huangemm3s10000_devs;

static inline struct huangemm3s10000_stratus_device *to_huangemm3s10000(struct esp_device *esp)
{
	return container_of(esp, struct huangemm3s10000_stratus_device, esp);
}

static void huangemm3s10000_prep_xfer(struct esp_device *esp, void *arg)
{
	struct huangemm3s10000_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->rows, esp->iomem + HUANGEMM3s10000_ROWS_REG);
	iowrite32be(a->cols, esp->iomem + HUANGEMM3s10000_COLS_REG);
	iowrite32be(a->loaded_cols, esp->iomem + HUANGEMM3s10000_LOADED_COLS_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool huangemm3s10000_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct huangemm3s10000_stratus_device *huangemm3s10000 = to_huangemm3s10000(esp); */
	/* struct huangemm3s10000_stratus_access *a = arg; */

	return true;
}

static int huangemm3s10000_probe(struct platform_device *pdev)
{
	struct huangemm3s10000_stratus_device *huangemm3s10000;
	struct esp_device *esp;
	int rc;

	huangemm3s10000 = kzalloc(sizeof(*huangemm3s10000), GFP_KERNEL);
	if (huangemm3s10000 == NULL)
		return -ENOMEM;
	esp = &huangemm3s10000->esp;
	esp->module = THIS_MODULE;
	esp->number = huangemm3s10000_devs;
	esp->driver = &huangemm3s10000_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	huangemm3s10000_devs++;
	return 0;
 err:
	kfree(huangemm3s10000);
	return rc;
}

static int __exit huangemm3s10000_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct huangemm3s10000_stratus_device *huangemm3s10000 = to_huangemm3s10000(esp);

	esp_device_unregister(esp);
	kfree(huangemm3s10000);
	return 0;
}

static struct esp_driver huangemm3s10000_driver = {
	.plat = {
		.probe		= huangemm3s10000_probe,
		.remove		= huangemm3s10000_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = huangemm3s10000_device_ids,
		},
	},
	.xfer_input_ok	= huangemm3s10000_xfer_input_ok,
	.prep_xfer	= huangemm3s10000_prep_xfer,
	.ioctl_cm	= HUANGEMM3s10000_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct huangemm3s10000_stratus_access),
};

static int __init huangemm3s10000_init(void)
{
	return esp_driver_register(&huangemm3s10000_driver);
}

static void __exit huangemm3s10000_exit(void)
{
	esp_driver_unregister(&huangemm3s10000_driver);
}

module_init(huangemm3s10000_init)
module_exit(huangemm3s10000_exit)

MODULE_DEVICE_TABLE(of, huangemm3s10000_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("huangemm3s10000_stratus driver");
