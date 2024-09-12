#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemm16_stratus.h"

#define DRV_NAME	"gemm16_stratus"

/* <<--regs-->> */
#define GEMM16_TRANSPOSE_REG 0x60
#define GEMM16_DO_RELU_REG 0x5c
#define GEMM16_ST_OFFSET_REG 0x58
#define GEMM16_LD_OFFSET2_REG 0x54
#define GEMM16_LD_OFFSET1_REG 0x50
#define GEMM16_D3_REG 0x4c
#define GEMM16_D2_REG 0x48
#define GEMM16_D1_REG 0x44
#define GEMM16_NINPUTS_REG 0x40

struct gemm16_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemm16_driver;

static struct of_device_id gemm16_device_ids[] = {
	{
		.name = "SLD_GEMM16_STRATUS",
	},
	{
		.name = "eb_051",
	},
	{
		.compatible = "sld,gemm16_stratus",
	},
	{ },
};

static int gemm16_devs;

static inline struct gemm16_stratus_device *to_gemm16(struct esp_device *esp)
{
	return container_of(esp, struct gemm16_stratus_device, esp);
}

static void gemm16_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemm16_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->do_relu, esp->iomem + GEMM16_DO_RELU_REG);
	iowrite32be(a->transpose, esp->iomem + GEMM16_TRANSPOSE_REG);
	iowrite32be(a->ninputs, esp->iomem + GEMM16_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMM16_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMM16_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMM16_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMM16_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMM16_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMM16_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool gemm16_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemm16_stratus_device *gemm16 = to_gemm16(esp); */
	/* struct gemm16_stratus_access *a = arg; */

	return true;
}

static int gemm16_probe(struct platform_device *pdev)
{
	struct gemm16_stratus_device *gemm16;
	struct esp_device *esp;
	int rc;

	gemm16 = kzalloc(sizeof(*gemm16), GFP_KERNEL);
	if (gemm16 == NULL)
		return -ENOMEM;
	esp = &gemm16->esp;
	esp->module = THIS_MODULE;
	esp->number = gemm16_devs;
	esp->driver = &gemm16_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemm16_devs++;
	return 0;
 err:
	kfree(gemm16);
	return rc;
}

static int __exit gemm16_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemm16_stratus_device *gemm16 = to_gemm16(esp);

	esp_device_unregister(esp);
	kfree(gemm16);
	return 0;
}

static struct esp_driver gemm16_driver = {
	.plat = {
		.probe		= gemm16_probe,
		.remove		= gemm16_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemm16_device_ids,
		},
	},
	.xfer_input_ok	= gemm16_xfer_input_ok,
	.prep_xfer	= gemm16_prep_xfer,
	.ioctl_cm	= GEMM16_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemm16_stratus_access),
};

static int __init gemm16_init(void)
{
	return esp_driver_register(&gemm16_driver);
}

static void __exit gemm16_exit(void)
{
	esp_driver_unregister(&gemm16_driver);
}

module_init(gemm16_init)
module_exit(gemm16_exit)

MODULE_DEVICE_TABLE(of, gemm16_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemm16_stratus driver");
