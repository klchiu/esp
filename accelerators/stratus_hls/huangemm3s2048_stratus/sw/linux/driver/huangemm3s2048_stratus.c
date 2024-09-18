// Copyright (c) 2011-2024 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "huangemm3s2048_stratus.h"

#define DRV_NAME	"huangemm3s2048_stratus"

/* <<--regs-->> */
// #define HUANGEMM3S2048_HUANGEMM3S2048_N_REG 0x48
// #define HUANGEMM3S2048_HUANGEMM3S2048_VEC_REG 0x44
// #define HUANGEMM3S2048_HUANGEMM3S2048_LEN_REG 0x40

#define HUANGEMM3S2048_ROWS_REG 0x48
#define HUANGEMM3S2048_COLS_REG 0x44
#define HUANGEMM3S2048_LOADED_COLS_REG 0x40

struct huangemm3s2048_stratus_device {
	struct esp_device esp;
};

static struct esp_driver huangemm3s2048_driver;

static struct of_device_id huangemm3s2048_device_ids[] = {
	{
		.name = "SLD_HUANGEMM3S2048_STRATUS",
	},
	{
		.name = "eb_239",
	},
	{
		.compatible = "sld,huangemm3s2048_stratus",
	},
	{ },
};

static int huangemm3s2048_devs;

static inline struct huangemm3s2048_stratus_device *to_huangemm3s2048(struct esp_device *esp)
{
	return container_of(esp, struct huangemm3s2048_stratus_device, esp);
}

static void huangemm3s2048_prep_xfer(struct esp_device *esp, void *arg)
{
	struct huangemm3s2048_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->rows, esp->iomem + HUANGEMM3S2048_ROWS_REG);
	iowrite32be(a->cols, esp->iomem + HUANGEMM3S2048_COLS_REG);
	iowrite32be(a->loaded_cols, esp->iomem + HUANGEMM3S2048_LOADED_COLS_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool huangemm3s2048_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct huangemm3s2048_stratus_device *huangemm3s2048 = to_huangemm3s2048(esp); */
	/* struct huangemm3s2048_stratus_access *a = arg; */

	return true;
}

static int huangemm3s2048_probe(struct platform_device *pdev)
{
	struct huangemm3s2048_stratus_device *huangemm3s2048;
	struct esp_device *esp;
	int rc;

	huangemm3s2048 = kzalloc(sizeof(*huangemm3s2048), GFP_KERNEL);
	if (huangemm3s2048 == NULL)
		return -ENOMEM;
	esp = &huangemm3s2048->esp;
	esp->module = THIS_MODULE;
	esp->number = huangemm3s2048_devs;
	esp->driver = &huangemm3s2048_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	huangemm3s2048_devs++;
	return 0;
 err:
	kfree(huangemm3s2048);
	return rc;
}

static int __exit huangemm3s2048_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct huangemm3s2048_stratus_device *huangemm3s2048 = to_huangemm3s2048(esp);

	esp_device_unregister(esp);
	kfree(huangemm3s2048);
	return 0;
}

static struct esp_driver huangemm3s2048_driver = {
	.plat = {
		.probe		= huangemm3s2048_probe,
		.remove		= huangemm3s2048_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = huangemm3s2048_device_ids,
		},
	},
	.xfer_input_ok	= huangemm3s2048_xfer_input_ok,
	.prep_xfer	= huangemm3s2048_prep_xfer,
	.ioctl_cm	= HUANGEMM3S2048_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct huangemm3s2048_stratus_access),
};

static int __init huangemm3s2048_init(void)
{
	return esp_driver_register(&huangemm3s2048_driver);
}

static void __exit huangemm3s2048_exit(void)
{
	esp_driver_unregister(&huangemm3s2048_driver);
}

module_init(huangemm3s2048_init)
module_exit(huangemm3s2048_exit)

MODULE_DEVICE_TABLE(of, huangemm3s2048_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("huangemm3s2048_stratus driver");
