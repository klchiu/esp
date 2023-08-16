// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include <linux/log2.h>

#include "nightHist_stratus.h"

#define DRV_NAME "nightHist_stratus"

#define NIGHTHIST_NIMAGES_REG 0x40
#define NIGHTHIST_ROWS_REG    0x44
#define NIGHTHIST_COLS_REG    0x48
#define NIGHTHIST_DO_DWT_REG  0x4C

struct nightHist_stratus_device {
    struct esp_device esp;
};

static struct esp_driver nightHist_driver;

static struct of_device_id nightHist_device_ids[] = {
    {
        .name = "SLD_NIGHTHIST_STRATUS",
    },
    {
        .name = "eb_011",
    },
    {
        .compatible = "sld,nightHist_stratus",
    },
    {},
};

static int nightHist_devs;

static inline struct nightHist_stratus_device *to_nightHist(struct esp_device *esp)
{
    return container_of(esp, struct nightHist_stratus_device, esp);
}

static void nightHist_prep_xfer(struct esp_device *esp, void *arg)
{
    struct nightHist_stratus_access *a = arg;

    iowrite32be(a->nimages, esp->iomem + NIGHTHIST_NIMAGES_REG);
    iowrite32be(a->rows, esp->iomem + NIGHTHIST_ROWS_REG);
    iowrite32be(a->cols, esp->iomem + NIGHTHIST_COLS_REG);
    iowrite32be(a->do_dwt, esp->iomem + NIGHTHIST_DO_DWT_REG);
    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool nightHist_xfer_input_ok(struct esp_device *esp, void *arg)
{
    struct nightHist_stratus_access *a = arg;

    if (a->nimages > MAX_NIMAGES)
        return false;

    if (a->rows > MAX_ROWS)
        return false;

    if (a->cols > MAX_COLS)
        return false;

    if (a->do_dwt != 0 && a->do_dwt != 1)
        return false;

    return true;
}

static int nightHist_probe(struct platform_device *pdev)
{
    struct nightHist_stratus_device *nightHist;
    struct esp_device *                esp;
    int                                rc;

    nightHist = kzalloc(sizeof(*nightHist), GFP_KERNEL);
    if (nightHist == NULL)
        return -ENOMEM;
    esp         = &nightHist->esp;
    esp->module = THIS_MODULE;
    esp->number = nightHist_devs;
    esp->driver = &nightHist_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    nightHist_devs++;
    return 0;
err:
    kfree(nightHist);
    return rc;
}

static int __exit nightHist_remove(struct platform_device *pdev)
{
    struct esp_device *                esp         = platform_get_drvdata(pdev);
    struct nightHist_stratus_device *nightHist = to_nightHist(esp);

    esp_device_unregister(esp);
    kfree(nightHist);
    return 0;
}

static struct esp_driver nightHist_driver = {
    .plat =
        {
            .probe  = nightHist_probe,
            .remove = nightHist_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = nightHist_device_ids,
                },
        },
    .xfer_input_ok = nightHist_xfer_input_ok,
    .prep_xfer     = nightHist_prep_xfer,
    .ioctl_cm      = NIGHTHIST_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct nightHist_stratus_access),
};

static int __init nightHist_init(void)
{
    printk(KERN_ALERT "Hello world nightHist!!\n");
    return esp_driver_register(&nightHist_driver);
}

static void __exit nightHist_exit(void) { esp_driver_unregister(&nightHist_driver); }

module_init(nightHist_init);
module_exit(nightHist_exit);

MODULE_DEVICE_TABLE(of, nightHist_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("nightHist_stratus driver");
