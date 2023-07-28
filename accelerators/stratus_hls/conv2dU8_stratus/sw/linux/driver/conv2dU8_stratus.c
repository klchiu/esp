#include <linux/of_device.h>
#include <linux/mm.h>

#include <asm/io.h>

#include <esp_accelerator.h>
#include <esp.h>

#include "conv2dU8_stratus.h"

#define DRV_NAME "conv2dU8_stratus"

/* <<--regs-->> */
#define CONV2DU8_N_CHANNELS_REG         0x40
#define CONV2DU8_FEATURE_MAP_HEIGHT_REG 0x44
#define CONV2DU8_FEATURE_MAP_WIDTH_REG  0x48
#define CONV2DU8_N_FILTERS_REG          0x4c
#define CONV2DU8_FILTER_DIM_REG         0x50
#define CONV2DU8_IS_PADDED_REG          0x54
#define CONV2DU8_STRIDE_REG             0x58
#define CONV2DU8_DO_RELU_REG            0x5c
#define CONV2DU8_POOL_TYPE_REG          0x60
#define CONV2DU8_BATCH_SIZE_REG         0x64

struct conv2dU8_stratus_device {
    struct esp_device esp;
};

static struct esp_driver conv2dU8_driver;

static struct of_device_id conv2dU8_device_ids[] = {
    {
        .name = "SLD_CONV2DU8_STRATUS",
    },
    {
        .name = "eb_054",
    },
    {
        .compatible = "sld,conv2dU8_stratus",
    },
    {},
};

static int conv2dU8_devs;

static inline struct conv2dU8_stratus_device *to_conv2dU8(struct esp_device *esp)
{
    printk("[humu]: to_conv2dU8()\n");

    return container_of(esp, struct conv2dU8_stratus_device, esp);
}

static void conv2dU8_prep_xfer(struct esp_device *esp, void *arg)
{
    struct conv2dU8_stratus_access *a = arg;

    printk("[humu]: conv2dU8_prep_xfer()\n");

    /* <<--regs-config-->> */
    iowrite32be(a->n_channels, esp->iomem + CONV2DU8_N_CHANNELS_REG);
    iowrite32be(a->feature_map_height, esp->iomem + CONV2DU8_FEATURE_MAP_HEIGHT_REG);
    iowrite32be(a->feature_map_width, esp->iomem + CONV2DU8_FEATURE_MAP_WIDTH_REG);
    iowrite32be(a->n_filters, esp->iomem + CONV2DU8_N_FILTERS_REG);
    iowrite32be(a->filter_dim, esp->iomem + CONV2DU8_FILTER_DIM_REG);
    iowrite32be(a->is_padded, esp->iomem + CONV2DU8_IS_PADDED_REG);
    iowrite32be(a->stride, esp->iomem + CONV2DU8_STRIDE_REG);
    iowrite32be(a->do_relu, esp->iomem + CONV2DU8_DO_RELU_REG);
    iowrite32be(a->pool_type, esp->iomem + CONV2DU8_POOL_TYPE_REG);
    iowrite32be(a->batch_size, esp->iomem + CONV2DU8_BATCH_SIZE_REG);
    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool conv2dU8_xfer_input_ok(struct esp_device *esp, void *arg)
{
    printk("[humu]: conv2dU8_xfer_input_ok()\n");

    /* struct conv2dU8_stratus_device *conv2dU8 = to_conv2dU8(esp); */
    /* struct conv2dU8_stratus_access *a = arg; */

    return true;
}

static int conv2dU8_probe(struct platform_device *pdev)
{
    struct conv2dU8_stratus_device *conv2dU8;
    struct esp_device *           esp;
    int                           rc;

    printk("[humu]: conv2dU8_probe()\n");


    conv2dU8 = kzalloc(sizeof(*conv2dU8), GFP_KERNEL);
    if (conv2dU8 == NULL)
        return -ENOMEM;
    esp         = &conv2dU8->esp;
    esp->module = THIS_MODULE;
    esp->number = conv2dU8_devs;
    esp->driver = &conv2dU8_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    conv2dU8_devs++;
    return 0;
err:
    kfree(conv2dU8);
    return rc;
}

static int __exit conv2dU8_remove(struct platform_device *pdev)
{
    struct esp_device *           esp    = platform_get_drvdata(pdev);
    struct conv2dU8_stratus_device *conv2dU8 = to_conv2dU8(esp);

    printk("[humu]: conv2dU8_remove()\n");


    esp_device_unregister(esp);
    kfree(conv2dU8);
    return 0;
}

static struct esp_driver conv2dU8_driver = {
    .plat =
        {
            .probe  = conv2dU8_probe,
            .remove = conv2dU8_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = conv2dU8_device_ids,
                },
        },
    .xfer_input_ok = conv2dU8_xfer_input_ok,
    .prep_xfer     = conv2dU8_prep_xfer,
    .ioctl_cm      = CONV2DU8_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct conv2dU8_stratus_access),
};

static int __init conv2dU8_init(void)
{
    printk("[humu]: conv2dU8_init()\n");
    return esp_driver_register(&conv2dU8_driver);
}

static void __exit conv2dU8_exit(void)
{
    printk("[humu]: conv2dU8_exit()\n");
    esp_driver_unregister(&conv2dU8_driver);
}

module_init(conv2dU8_init);
module_exit(conv2dU8_exit);

MODULE_DEVICE_TABLE(of, conv2dU8_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("conv2dU8_stratus driver");
