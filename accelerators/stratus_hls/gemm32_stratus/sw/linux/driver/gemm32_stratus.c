#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemm32_stratus.h"

#define DRV_NAME	"gemm32_stratus"

/* <<--regs-->> */
#define GEMM32_TRANSPOSE_REG 0x60
#define GEMM32_DO_RELU_REG 0x5c
#define GEMM32_ST_OFFSET_REG 0x58
#define GEMM32_LD_OFFSET2_REG 0x54
#define GEMM32_LD_OFFSET1_REG 0x50
#define GEMM32_D3_REG 0x4c
#define GEMM32_D2_REG 0x48
#define GEMM32_D1_REG 0x44
#define GEMM32_NINPUTS_REG 0x40

struct gemm32_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemm32_driver;

static struct of_device_id gemm32_device_ids[] = {
	{
		.name = "SLD_GEMM32_STRATUS",
	},
	{
		.name = "eb_051",
	},
	{
		.compatible = "sld,gemm32_stratus",
	},
	{ },
};

static int gemm32_devs;

static inline struct gemm32_stratus_device *to_gemm32(struct esp_device *esp)
{
	return container_of(esp, struct gemm32_stratus_device, esp);
}

static void gemm32_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemm32_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->do_relu, esp->iomem + GEMM32_DO_RELU_REG);
	iowrite32be(a->transpose, esp->iomem + GEMM32_TRANSPOSE_REG);
	iowrite32be(a->ninputs, esp->iomem + GEMM32_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMM32_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMM32_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMM32_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMM32_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMM32_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMM32_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool gemm32_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemm32_stratus_device *gemm32 = to_gemm32(esp); */
	/* struct gemm32_stratus_access *a = arg; */

	return true;
}

static int gemm32_probe(struct platform_device *pdev)
{
	struct gemm32_stratus_device *gemm32;
	struct esp_device *esp;
	int rc;

	gemm32 = kzalloc(sizeof(*gemm32), GFP_KERNEL);
	if (gemm32 == NULL)
		return -ENOMEM;
	esp = &gemm32->esp;
	esp->module = THIS_MODULE;
	esp->number = gemm32_devs;
	esp->driver = &gemm32_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemm32_devs++;
	return 0;
 err:
	kfree(gemm32);
	return rc;
}

static int __exit gemm32_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemm32_stratus_device *gemm32 = to_gemm32(esp);

	esp_device_unregister(esp);
	kfree(gemm32);
	return 0;
}

static struct esp_driver gemm32_driver = {
	.plat = {
		.probe		= gemm32_probe,
		.remove		= gemm32_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemm32_device_ids,
		},
	},
	.xfer_input_ok	= gemm32_xfer_input_ok,
	.prep_xfer	= gemm32_prep_xfer,
	.ioctl_cm	= GEMM32_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemm32_stratus_access),
};

static int __init gemm32_init(void)
{
	return esp_driver_register(&gemm32_driver);
}

static void __exit gemm32_exit(void)
{
	esp_driver_unregister(&gemm32_driver);
}

module_init(gemm32_init)
module_exit(gemm32_exit)

MODULE_DEVICE_TABLE(of, gemm32_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemm32_stratus driver");
