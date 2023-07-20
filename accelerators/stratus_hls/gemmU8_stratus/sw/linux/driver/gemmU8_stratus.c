#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmU8_stratus.h"

#define DRV_NAME	"gemmU8_stratus"

/* <<--regs-->> */
#define GEMMU8_TRANSPOSE_REG 0x60
#define GEMMU8_DO_RELU_REG 0x5c
#define GEMMU8_ST_OFFSET_REG 0x58
#define GEMMU8_LD_OFFSET2_REG 0x54
#define GEMMU8_LD_OFFSET1_REG 0x50
#define GEMMU8_D3_REG 0x4c
#define GEMMU8_D2_REG 0x48
#define GEMMU8_D1_REG 0x44
#define GEMMU8_NINPUTS_REG 0x40

struct gemmU8_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmU8_driver;

static struct of_device_id gemmU8_device_ids[] = {
	{
		.name = "SLD_GEMMU8_STRATUS",
	},
	{
		.name = "eb_051",
	},
	{
		.compatible = "sld,gemmU8_stratus",
	},
	{ },
};

static int gemmU8_devs;

static inline struct gemmU8_stratus_device *to_gemmU8(struct esp_device *esp)
{
	return container_of(esp, struct gemmU8_stratus_device, esp);
}

static void gemmU8_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmU8_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->do_relu, esp->iomem + GEMMU8_DO_RELU_REG);
	iowrite32be(a->transpose, esp->iomem + GEMMU8_TRANSPOSE_REG);
	iowrite32be(a->ninputs, esp->iomem + GEMMU8_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMU8_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMU8_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMU8_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMU8_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMU8_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMU8_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool gemmU8_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmU8_stratus_device *gemmU8 = to_gemmU8(esp); */
	/* struct gemmU8_stratus_access *a = arg; */

	return true;
}

static int gemmU8_probe(struct platform_device *pdev)
{
	struct gemmU8_stratus_device *gemmU8;
	struct esp_device *esp;
	int rc;

	gemmU8 = kzalloc(sizeof(*gemmU8), GFP_KERNEL);
	if (gemmU8 == NULL)
		return -ENOMEM;
	esp = &gemmU8->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmU8_devs;
	esp->driver = &gemmU8_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmU8_devs++;
	return 0;
 err:
	kfree(gemmU8);
	return rc;
}

static int __exit gemmU8_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmU8_stratus_device *gemmU8 = to_gemmU8(esp);

	esp_device_unregister(esp);
	kfree(gemmU8);
	return 0;
}

static struct esp_driver gemmU8_driver = {
	.plat = {
		.probe		= gemmU8_probe,
		.remove		= gemmU8_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmU8_device_ids,
		},
	},
	.xfer_input_ok	= gemmU8_xfer_input_ok,
	.prep_xfer	= gemmU8_prep_xfer,
	.ioctl_cm	= GEMMU8_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmU8_stratus_access),
};

static int __init gemmU8_init(void)
{
	return esp_driver_register(&gemmU8_driver);
}

static void __exit gemmU8_exit(void)
{
	esp_driver_unregister(&gemmU8_driver);
}

module_init(gemmU8_init)
module_exit(gemmU8_exit)

MODULE_DEVICE_TABLE(of, gemmU8_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmU8_stratus driver");
