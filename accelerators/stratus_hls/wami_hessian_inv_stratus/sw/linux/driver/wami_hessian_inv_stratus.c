// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/log2.h>
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include "wami_hessian_inv_stratus.h"

#define DRV_NAME "wami_hessian_inv_stratus"

#define WAMI_SRC_DST_OFFSET_0_REG 0x40
#define WAMI_NUM_IMG_REG          0x44
#define WAMI_NUM_COL_REG          0x48
#define WAMI_NUM_ROW_REG          0x4C
#define WAMI_PAD_REG              0x50
#define WAMI_KERN_ID_REG          0x54
#define WAMI_BATCH_REG            0x58
#define WAMI_SRC_DST_OFFSET_1_REG 0x5C
#define WAMI_SRC_DST_OFFSET_2_REG 0x60
#define WAMI_SRC_DST_OFFSET_3_REG 0x64
#define WAMI_SRC_DST_OFFSET_4_REG 0x68

struct wami_hessian_inv_stratus_device {
    struct esp_device esp;
};

static struct esp_driver wami_hessian_inv_driver;

static struct of_device_id wami_hessian_inv_device_ids[] = {
    {
        .name = "SLD_WAMI_HESSIAN_INV_STRATUS",
    },
    {
        .name = "eb_07C",
    },
    {
        .compatible = "sld,wami_hessian_inv_stratus",
    },
    {},
};

static int wami_hessian_inv_devs;

static inline struct wami_hessian_inv_stratus_device *to_wami_hessian_inv(struct esp_device *esp)
{
    return container_of(esp, struct wami_hessian_inv_stratus_device, esp);
}

static void wami_hessian_inv_prep_xfer(struct esp_device *esp, void *arg)
{
    struct wami_hessian_inv_stratus_access *a = arg;

    iowrite32be(a->wami_src_dst_offset_0, esp->iomem + WAMI_SRC_DST_OFFSET_0_REG);
    iowrite32be(a->wami_num_img, esp->iomem + WAMI_NUM_IMG_REG);
    iowrite32be(a->wami_num_col, esp->iomem + WAMI_NUM_COL_REG);
    iowrite32be(a->wami_num_row, esp->iomem + WAMI_NUM_ROW_REG);
    iowrite32be(a->wami_pad, esp->iomem + WAMI_PAD_REG);
    iowrite32be(a->wami_kern_id, esp->iomem + WAMI_KERN_ID_REG);
    iowrite32be(a->wami_batch, esp->iomem + WAMI_BATCH_REG);
    iowrite32be(a->wami_src_dst_offset_1, esp->iomem + WAMI_SRC_DST_OFFSET_1_REG);
    iowrite32be(a->wami_src_dst_offset_2, esp->iomem + WAMI_SRC_DST_OFFSET_2_REG);
    iowrite32be(a->wami_src_dst_offset_3, esp->iomem + WAMI_SRC_DST_OFFSET_3_REG);
    iowrite32be(a->wami_src_dst_offset_4, esp->iomem + WAMI_SRC_DST_OFFSET_4_REG);

    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool wami_hessian_inv_xfer_input_ok(struct esp_device *esp, void *arg)
{
    // struct wami_hessian_inv_stratus_access *a = arg;

    return true;
}

static int wami_hessian_inv_probe(struct platform_device *pdev)
{
    struct wami_hessian_inv_stratus_device *wami_hessian_inv;
    struct esp_device *              esp;
    int                              rc;

    wami_hessian_inv = kzalloc(sizeof(*wami_hessian_inv), GFP_KERNEL);
    if (wami_hessian_inv == NULL)
        return -ENOMEM;
    esp         = &wami_hessian_inv->esp;
    esp->module = THIS_MODULE;
    esp->number = wami_hessian_inv_devs;
    esp->driver = &wami_hessian_inv_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    wami_hessian_inv_devs++;
    return 0;
err:
    kfree(wami_hessian_inv);
    return rc;
}

static int __exit wami_hessian_inv_remove(struct platform_device *pdev)
{
    struct esp_device *              esp       = platform_get_drvdata(pdev);
    struct wami_hessian_inv_stratus_device *wami_hessian_inv = to_wami_hessian_inv(esp);

    esp_device_unregister(esp);
    kfree(wami_hessian_inv);
    return 0;
}

static struct esp_driver wami_hessian_inv_driver = {
    .plat =
        {
            .probe  = wami_hessian_inv_probe,
            .remove = wami_hessian_inv_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = wami_hessian_inv_device_ids,
                },
        },
    .xfer_input_ok = wami_hessian_inv_xfer_input_ok,
    .prep_xfer     = wami_hessian_inv_prep_xfer,
    .ioctl_cm      = WAMI_HESSIAN_INV_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct wami_hessian_inv_stratus_access),
};

static int __init wami_hessian_inv_init(void) { return esp_driver_register(&wami_hessian_inv_driver); }

static void __exit wami_hessian_inv_exit(void) { esp_driver_unregister(&wami_hessian_inv_driver); }

module_init(wami_hessian_inv_init) module_exit(wami_hessian_inv_exit)

    MODULE_DEVICE_TABLE(of, wami_hessian_inv_device_ids);

MODULE_AUTHOR("Emilio G. Cota <cota@braap.org>");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("wami_hessian_inv_stratus driver");
