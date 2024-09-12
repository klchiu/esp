#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemm128_stratus.h"

#define DRV_NAME	"gemm128_stratus"

/* <<--regs-->> */
#define GEMM128_TRANSPOSE_REG 0x60
#define GEMM128_DO_RELU_REG 0x5c
#define GEMM128_ST_OFFSET_REG 0x58
#define GEMM128_LD_OFFSET2_REG 0x54
#define GEMM128_LD_OFFSET1_REG 0x50
#define GEMM128_D3_REG 0x4c
#define GEMM128_D2_REG 0x48
#define GEMM128_D1_REG 0x44
#define GEMM128_NINPUTS_REG 0x40

struct gemm128_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemm128_driver;

static struct of_device_id gemm128_device_ids[] = {
	{
		.name = "SLD_GEMM128_STRATUS",
	},
	{
		.name = "eb_051",
	},
	{
		.compatible = "sld,gemm128_stratus",
	},
	{ },
};

static int gemm128_devs;

static inline struct gemm128_stratus_device *to_gemm128(struct esp_device *esp)
{
	return container_of(esp, struct gemm128_stratus_device, esp);
}

static void gemm128_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemm128_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->do_relu, esp->iomem + GEMM128_DO_RELU_REG);
	iowrite32be(a->transpose, esp->iomem + GEMM128_TRANSPOSE_REG);
	iowrite32be(a->ninputs, esp->iomem + GEMM128_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMM128_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMM128_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMM128_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMM128_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMM128_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMM128_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool gemm128_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemm128_stratus_device *gemm128 = to_gemm128(esp); */
	/* struct gemm128_stratus_access *a = arg; */

	return true;
}

static int gemm128_probe(struct platform_device *pdev)
{
	struct gemm128_stratus_device *gemm128;
	struct esp_device *esp;
	int rc;

	gemm128 = kzalloc(sizeof(*gemm128), GFP_KERNEL);
	if (gemm128 == NULL)
		return -ENOMEM;
	esp = &gemm128->esp;
	esp->module = THIS_MODULE;
	esp->number = gemm128_devs;
	esp->driver = &gemm128_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemm128_devs++;
	return 0;
 err:
	kfree(gemm128);
	return rc;
}

static int __exit gemm128_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemm128_stratus_device *gemm128 = to_gemm128(esp);

	esp_device_unregister(esp);
	kfree(gemm128);
	return 0;
}

static struct esp_driver gemm128_driver = {
	.plat = {
		.probe		= gemm128_probe,
		.remove		= gemm128_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemm128_device_ids,
		},
	},
	.xfer_input_ok	= gemm128_xfer_input_ok,
	.prep_xfer	= gemm128_prep_xfer,
	.ioctl_cm	= GEMM128_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemm128_stratus_access),
};

static int __init gemm128_init(void)
{
	return esp_driver_register(&gemm128_driver);
}

static void __exit gemm128_exit(void)
{
	esp_driver_unregister(&gemm128_driver);
}

module_init(gemm128_init)
module_exit(gemm128_exit)

MODULE_DEVICE_TABLE(of, gemm128_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemm128_stratus driver");
