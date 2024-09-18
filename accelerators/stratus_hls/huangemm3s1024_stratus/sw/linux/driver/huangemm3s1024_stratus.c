// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "huangemm3s1024_stratus.h"

#define DRV_NAME	"huangemm3s1024_stratus"

/* <<--regs-->> */
// #define HUANGEMM3S1024_HUANGEMM3S1024_N_REG 0x48
// #define HUANGEMM3S1024_HUANGEMM3S1024_VEC_REG 0x44
// #define HUANGEMM3S1024_HUANGEMM3S1024_LEN_REG 0x40

#define HUANGEMM3S1024_ROWS_REG 0x48
#define HUANGEMM3S1024_COLS_REG 0x44
#define HUANGEMM3S1024_LOADED_COLS_REG 0x40

struct huangemm3s1024_stratus_device {
	struct esp_device esp;
};

static struct esp_driver huangemm3s1024_driver;

static struct of_device_id huangemm3s1024_device_ids[] = {
	{
		.name = "SLD_HUANGEMM3S1024_STRATUS",
	},
	{
		.name = "eb_238",
	},
	{
		.compatible = "sld,huangemm3s1024_stratus",
	},
	{ },
};

static int huangemm3s1024_devs;

static inline struct huangemm3s1024_stratus_device *to_huangemm3s1024(struct esp_device *esp)
{
	return container_of(esp, struct huangemm3s1024_stratus_device, esp);
}

static void huangemm3s1024_prep_xfer(struct esp_device *esp, void *arg)
{
	struct huangemm3s1024_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->rows, esp->iomem + HUANGEMM3S1024_ROWS_REG);
	iowrite32be(a->cols, esp->iomem + HUANGEMM3S1024_COLS_REG);
	iowrite32be(a->loaded_cols, esp->iomem + HUANGEMM3S1024_LOADED_COLS_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool huangemm3s1024_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct huangemm3s1024_stratus_device *huangemm3s1024 = to_huangemm3s1024(esp); */
	/* struct huangemm3s1024_stratus_access *a = arg; */

	return true;
}

static int huangemm3s1024_probe(struct platform_device *pdev)
{
	struct huangemm3s1024_stratus_device *huangemm3s1024;
	struct esp_device *esp;
	int rc;

	huangemm3s1024 = kzalloc(sizeof(*huangemm3s1024), GFP_KERNEL);
	if (huangemm3s1024 == NULL)
		return -ENOMEM;
	esp = &huangemm3s1024->esp;
	esp->module = THIS_MODULE;
	esp->number = huangemm3s1024_devs;
	esp->driver = &huangemm3s1024_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	huangemm3s1024_devs++;
	return 0;
 err:
	kfree(huangemm3s1024);
	return rc;
}

static int __exit huangemm3s1024_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct huangemm3s1024_stratus_device *huangemm3s1024 = to_huangemm3s1024(esp);

	esp_device_unregister(esp);
	kfree(huangemm3s1024);
	return 0;
}

static struct esp_driver huangemm3s1024_driver = {
	.plat = {
		.probe		= huangemm3s1024_probe,
		.remove		= huangemm3s1024_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = huangemm3s1024_device_ids,
		},
	},
	.xfer_input_ok	= huangemm3s1024_xfer_input_ok,
	.prep_xfer	= huangemm3s1024_prep_xfer,
	.ioctl_cm	= HUANGEMM3S1024_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct huangemm3s1024_stratus_access),
};

static int __init huangemm3s1024_init(void)
{
	return esp_driver_register(&huangemm3s1024_driver);
}

static void __exit huangemm3s1024_exit(void)
{
	esp_driver_unregister(&huangemm3s1024_driver);
}

module_init(huangemm3s1024_init)
module_exit(huangemm3s1024_exit)

MODULE_DEVICE_TABLE(of, huangemm3s1024_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("huangemm3s1024_stratus driver");
