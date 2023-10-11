// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "systolic_gemm_vivado.h"

#define DRV_NAME	"systolic_gemm_vivado"

/* <<--regs-->> */
#define SYSTOLIC_GEMM_MAC_VEC_REG 0x48
#define SYSTOLIC_GEMM_MAC_LEN_REG 0x44
#define SYSTOLIC_GEMM_MAC_N_REG 0x40

struct systolic_gemm_vivado_device {
	struct esp_device esp;
};

static struct esp_driver systolic_gemm_driver;

static struct of_device_id systolic_gemm_device_ids[] = {
	{
		.name = "SLD_SYSTOLIC_GEMM_VIVADO",
	},
	{
		.name = "eb_087",
	},
	{
		.compatible = "sld,systolic_gemm_vivado",
	},
	{ },
};

static int systolic_gemm_devs;

static inline struct systolic_gemm_vivado_device *to_systolic_gemm(struct esp_device *esp)
{
	return container_of(esp, struct systolic_gemm_vivado_device, esp);
}

static void systolic_gemm_prep_xfer(struct esp_device *esp, void *arg)
{
	struct systolic_gemm_vivado_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->mac_vec, esp->iomem + SYSTOLIC_GEMM_MAC_VEC_REG);
	iowrite32be(a->mac_len, esp->iomem + SYSTOLIC_GEMM_MAC_LEN_REG);
	iowrite32be(a->mac_n, esp->iomem + SYSTOLIC_GEMM_MAC_N_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);

}

static bool systolic_gemm_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct systolic_gemm_vivado_device *systolic_gemm = to_systolic_gemm(esp); */
	/* struct systolic_gemm_vivado_access *a = arg; */

	return true;
}

static int systolic_gemm_probe(struct platform_device *pdev)
{
	struct systolic_gemm_vivado_device *systolic_gemm;
	struct esp_device *esp;
	int rc;

	systolic_gemm = kzalloc(sizeof(*systolic_gemm), GFP_KERNEL);
	if (systolic_gemm == NULL)
		return -ENOMEM;
	esp = &systolic_gemm->esp;
	esp->module = THIS_MODULE;
	esp->number = systolic_gemm_devs;
	esp->driver = &systolic_gemm_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	systolic_gemm_devs++;
	return 0;
 err:
	kfree(systolic_gemm);
	return rc;
}

static int __exit systolic_gemm_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct systolic_gemm_vivado_device *systolic_gemm = to_systolic_gemm(esp);

	esp_device_unregister(esp);
	kfree(systolic_gemm);
	return 0;
}

static struct esp_driver systolic_gemm_driver = {
	.plat = {
		.probe		= systolic_gemm_probe,
		.remove		= systolic_gemm_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = systolic_gemm_device_ids,
		},
	},
	.xfer_input_ok	= systolic_gemm_xfer_input_ok,
	.prep_xfer	= systolic_gemm_prep_xfer,
	.ioctl_cm	= SYSTOLIC_GEMM_VIVADO_IOC_ACCESS,
	.arg_size	= sizeof(struct systolic_gemm_vivado_access),
};

static int __init systolic_gemm_init(void)
{
	return esp_driver_register(&systolic_gemm_driver);
}

static void __exit systolic_gemm_exit(void)
{
	esp_driver_unregister(&systolic_gemm_driver);
}

module_init(systolic_gemm_init)
module_exit(systolic_gemm_exit)

MODULE_DEVICE_TABLE(of, systolic_gemm_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("systolic_gemm_vivado driver");
