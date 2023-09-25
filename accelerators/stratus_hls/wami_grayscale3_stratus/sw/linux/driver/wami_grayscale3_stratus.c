// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/log2.h>
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include "wami_grayscale3_stratus.h"

#define DRV_NAME "wami_grayscale3_stratus"

#define WAMI_NUM_IMG_REG          0x40
#define WAMI_NUM_COL_REG          0x44
#define WAMI_NUM_ROW_REG          0x48
#define WAMI_PAD_REG              0x4C
#define WAMI_KERN_ID_REG          0x50
#define WAMI_BATCH_REG            0x54
#define WAMI_SRC_DST_OFFSET_0_REG 0x58
#define WAMI_SRC_DST_OFFSET_1_REG 0x5C
#define WAMI_SRC_DST_OFFSET_2_REG 0x60
#define WAMI_SRC_DST_OFFSET_3_REG 0x64
#define WAMI_SRC_DST_OFFSET_4_REG 0x68
#define WAMI_IS_P2P_REG           0x6C
#define WAMI_P2P_CONFIG_0_REG     0x70
#define WAMI_P2P_CONFIG_1_REG     0x74

struct wami_grayscale3_stratus_device {
    struct esp_device esp;
};

static struct esp_driver wami_grayscale3_driver;

static struct of_device_id wami_grayscale3_device_ids[] = {
    {
        .name = "SLD_WAMI_GRAYSCALE3_STRATUS",
    },
    {
        .name = "eb_07C",
    },
    {
        .compatible = "sld,wami_grayscale3_stratus",
    },
    {},
};

static int wami_grayscale3_devs;

static inline struct wami_grayscale3_stratus_device *to_wami_grayscale3(struct esp_device *esp)
{
    return container_of(esp, struct wami_grayscale3_stratus_device, esp);
}

static void wami_grayscale3_prep_xfer(struct esp_device *esp, void *arg)
{
    struct wami_grayscale3_stratus_access *a = arg;

    iowrite32be(a->wami_num_img, esp->iomem + WAMI_NUM_IMG_REG);
    iowrite32be(a->wami_num_col, esp->iomem + WAMI_NUM_COL_REG);
    iowrite32be(a->wami_num_row, esp->iomem + WAMI_NUM_ROW_REG);
    iowrite32be(a->wami_pad, esp->iomem + WAMI_PAD_REG);
    iowrite32be(a->wami_kern_id, esp->iomem + WAMI_KERN_ID_REG);
    iowrite32be(a->wami_batch, esp->iomem + WAMI_BATCH_REG);
    iowrite32be(a->wami_src_dst_offset_0, esp->iomem + WAMI_SRC_DST_OFFSET_0_REG);
    iowrite32be(a->wami_src_dst_offset_1, esp->iomem + WAMI_SRC_DST_OFFSET_1_REG);
    iowrite32be(a->wami_src_dst_offset_2, esp->iomem + WAMI_SRC_DST_OFFSET_2_REG);
    iowrite32be(a->wami_src_dst_offset_3, esp->iomem + WAMI_SRC_DST_OFFSET_3_REG);
    iowrite32be(a->wami_src_dst_offset_4, esp->iomem + WAMI_SRC_DST_OFFSET_4_REG);
    iowrite32be(a->wami_is_p2p, esp->iomem + WAMI_IS_P2P_REG);
    iowrite32be(a->wami_p2p_config_0, esp->iomem + WAMI_P2P_CONFIG_0_REG);
    iowrite32be(a->wami_p2p_config_1, esp->iomem + WAMI_P2P_CONFIG_1_REG);

    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool wami_grayscale3_xfer_input_ok(struct esp_device *esp, void *arg)
{
    // struct wami_grayscale3_stratus_access *a = arg;

    return true;
}

static int wami_grayscale3_probe(struct platform_device *pdev)
{
    struct wami_grayscale3_stratus_device *wami_grayscale3;
    struct esp_device *                    esp;
    int                                    rc;

    wami_grayscale3 = kzalloc(sizeof(*wami_grayscale3), GFP_KERNEL);
    if (wami_grayscale3 == NULL)
        return -ENOMEM;
    esp         = &wami_grayscale3->esp;
    esp->module = THIS_MODULE;
    esp->number = wami_grayscale3_devs;
    esp->driver = &wami_grayscale3_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    wami_grayscale3_devs++;
    return 0;
err:
    kfree(wami_grayscale3);
    return rc;
}

static int __exit wami_grayscale3_remove(struct platform_device *pdev)
{
    struct esp_device *                    esp             = platform_get_drvdata(pdev);
    struct wami_grayscale3_stratus_device *wami_grayscale3 = to_wami_grayscale3(esp);

    esp_device_unregister(esp);
    kfree(wami_grayscale3);
    return 0;
}

static struct esp_driver wami_grayscale3_driver = {
    .plat =
        {
            .probe  = wami_grayscale3_probe,
            .remove = wami_grayscale3_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = wami_grayscale3_device_ids,
                },
        },
    .xfer_input_ok = wami_grayscale3_xfer_input_ok,
    .prep_xfer     = wami_grayscale3_prep_xfer,
    .ioctl_cm      = WAMI_GRAYSCALE3_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct wami_grayscale3_stratus_access),
};

static int __init wami_grayscale3_init(void) { return esp_driver_register(&wami_grayscale3_driver); }

static void __exit wami_grayscale3_exit(void) { esp_driver_unregister(&wami_grayscale3_driver); }

module_init(wami_grayscale3_init) module_exit(wami_grayscale3_exit)

    MODULE_DEVICE_TABLE(of, wami_grayscale3_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wami_grayscale3_stratus driver");
