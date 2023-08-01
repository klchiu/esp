#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "gemmBig_stratus.h"

#define DRV_NAME	"gemmBig_stratus"

/* <<--regs-->> */
#define GEMMBIG_TRANSPOSE_REG 0x60
#define GEMMBIG_DO_RELU_REG 0x5c
#define GEMMBIG_ST_OFFSET_REG 0x58
#define GEMMBIG_LD_OFFSET2_REG 0x54
#define GEMMBIG_LD_OFFSET1_REG 0x50
#define GEMMBIG_D3_REG 0x4c
#define GEMMBIG_D2_REG 0x48
#define GEMMBIG_D1_REG 0x44
#define GEMMBIG_NINPUTS_REG 0x40

struct gemmBig_stratus_device {
	struct esp_device esp;
};

static struct esp_driver gemmBig_driver;

static struct of_device_id gemmBig_device_ids[] = {
	{
		.name = "SLD_GEMMBIG_STRATUS",
	},
	{
		.name = "eb_051",
	},
	{
		.compatible = "sld,gemmBig_stratus",
	},
	{ },
};

static int gemmBig_devs;

static inline struct gemmBig_stratus_device *to_gemmBig(struct esp_device *esp)
{
	return container_of(esp, struct gemmBig_stratus_device, esp);
}

static void gemmBig_prep_xfer(struct esp_device *esp, void *arg)
{
	struct gemmBig_stratus_access *a = arg;

	/* <<--regs-config-->> */
	iowrite32be(a->do_relu, esp->iomem + GEMMBIG_DO_RELU_REG);
	iowrite32be(a->transpose, esp->iomem + GEMMBIG_TRANSPOSE_REG);
	iowrite32be(a->ninputs, esp->iomem + GEMMBIG_NINPUTS_REG);
	iowrite32be(a->d3, esp->iomem + GEMMBIG_D3_REG);
	iowrite32be(a->d2, esp->iomem + GEMMBIG_D2_REG);
	iowrite32be(a->d1, esp->iomem + GEMMBIG_D1_REG);
	iowrite32be(a->st_offset, esp->iomem + GEMMBIG_ST_OFFSET_REG);
	iowrite32be(a->ld_offset1, esp->iomem + GEMMBIG_LD_OFFSET1_REG);
	iowrite32be(a->ld_offset2, esp->iomem + GEMMBIG_LD_OFFSET2_REG);
	iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
	iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool gemmBig_xfer_input_ok(struct esp_device *esp, void *arg)
{
	/* struct gemmBig_stratus_device *gemmBig = to_gemmBig(esp); */
	/* struct gemmBig_stratus_access *a = arg; */

	return true;
}

static int gemmBig_probe(struct platform_device *pdev)
{
	struct gemmBig_stratus_device *gemmBig;
	struct esp_device *esp;
	int rc;

	gemmBig = kzalloc(sizeof(*gemmBig), GFP_KERNEL);
	if (gemmBig == NULL)
		return -ENOMEM;
	esp = &gemmBig->esp;
	esp->module = THIS_MODULE;
	esp->number = gemmBig_devs;
	esp->driver = &gemmBig_driver;
	rc = esp_device_register(esp, pdev);
	if (rc)
		goto err;

	gemmBig_devs++;
	return 0;
 err:
	kfree(gemmBig);
	return rc;
}

static int __exit gemmBig_remove(struct platform_device *pdev)
{
	struct esp_device *esp = platform_get_drvdata(pdev);
	struct gemmBig_stratus_device *gemmBig = to_gemmBig(esp);

	esp_device_unregister(esp);
	kfree(gemmBig);
	return 0;
}

static struct esp_driver gemmBig_driver = {
	.plat = {
		.probe		= gemmBig_probe,
		.remove		= gemmBig_remove,
		.driver		= {
			.name = DRV_NAME,
			.owner = THIS_MODULE,
			.of_match_table = gemmBig_device_ids,
		},
	},
	.xfer_input_ok	= gemmBig_xfer_input_ok,
	.prep_xfer	= gemmBig_prep_xfer,
	.ioctl_cm	= GEMMBIG_STRATUS_IOC_ACCESS,
	.arg_size	= sizeof(struct gemmBig_stratus_access),
};

static int __init gemmBig_init(void)
{
	return esp_driver_register(&gemmBig_driver);
}

static void __exit gemmBig_exit(void)
{
	esp_driver_unregister(&gemmBig_driver);
}

module_init(gemmBig_init)
module_exit(gemmBig_exit)

MODULE_DEVICE_TABLE(of, gemmBig_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("gemmBig_stratus driver");
