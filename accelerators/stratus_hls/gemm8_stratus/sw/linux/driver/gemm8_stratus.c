#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemm8_stratus.h"

#define DRV_NAME	"gemm8_stratus"

/* <<--regs-->> */
#define GEMM8_TRANSPOSE_REG 0x60
#define GEMM8_DO_RELU_REG 0x5c
#define GEMM8_ST_OFFSET_REG 0x58
#define GEMM8_LD_OFFSET2_REG 0x54
#define GEMM8_LD_OFFSET1_REG 0x50
#define GEMM8_D3_REG 0x4c
#define GEMM8_D2_REG 0x48
#define GEMM8_D1_REG 0x44
#define GEMM8_NINPUTS_REG 0x40

struct gemm8_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemm8_driver;

static struct of_device_id gemm8_device_ids[] = {
	{
		.name = "SLD_GEMM8_STRATUS",
	},
	{
		.name = "eb_051",
	},
	{
		.compatible = "sld,gemm8_stratus",
	},
	{ },
};

static int gemm8_devs;

static inline struct gemm8_stratus_device *to_gemm8(struct esp_device *esp)
{
	return container_of(esp, struct gemm8_stratus_device, esp);
}

static void gemm8_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemm8_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->do_relu, esp->iomem + GEMM8_DO_RELU_REG);
	iowrite32be(a->transpose, esp->iomem + GEMM8_TRANSPOSE_REG);
	iowrite32be(a->ninputs, esp->iomem + GEMM8_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMM8_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMM8_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMM8_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMM8_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMM8_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMM8_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool gemm8_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemm8_stratus_device *gemm8 = to_gemm8(esp); */
	/* struct gemm8_stratus_access *a = arg; */

	return true;
}

static int gemm8_probe(struct platform_device *pdev)
{
	struct gemm8_stratus_device *gemm8;
	struct esp_device *esp;
	int rc;

	gemm8 = kzalloc(sizeof(*gemm8), GFP_KERNEL);
	if (gemm8 == NULL)
		return -ENOMEM;
	esp = &gemm8->esp;
	esp->module = THIS_MODULE;
	esp->number = gemm8_devs;
	esp->driver = &gemm8_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemm8_devs++;
	return 0;
 err:
	kfree(gemm8);
	return rc;
}

static int __exit gemm8_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemm8_stratus_device *gemm8 = to_gemm8(esp);

	esp_device_unregister(esp);
	kfree(gemm8);
	return 0;
}

static struct esp_driver gemm8_driver = {
	.plat = {
		.probe		= gemm8_probe,
		.remove		= gemm8_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemm8_device_ids,
		},
	},
	.xfer_input_ok	= gemm8_xfer_input_ok,
	.prep_xfer	= gemm8_prep_xfer,
	.ioctl_cm	= GEMM8_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemm8_stratus_access),
};

static int __init gemm8_init(void)
{
	return esp_driver_register(&gemm8_driver);
}

static void __exit gemm8_exit(void)
{
	esp_driver_unregister(&gemm8_driver);
}

module_init(gemm8_init)
module_exit(gemm8_exit)

MODULE_DEVICE_TABLE(of, gemm8_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemm8_stratus driver");
