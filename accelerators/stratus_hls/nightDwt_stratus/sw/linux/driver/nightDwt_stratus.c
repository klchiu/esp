// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include <linux/log2.h>

#include "nightDwt_stratus.h"

#define DRV_NAME "nightDwt_stratus"

#define NIGHTDWT_NIMAGES_REG 0x40
#define NIGHTDWT_ROWS_REG    0x44
#define NIGHTDWT_COLS_REG    0x48
#define NIGHTDWT_DO_DWT_REG  0x4C

struct nightDwt_stratus_device {
    struct esp_device esp;
};

static struct esp_driver nightDwt_driver;

static struct of_device_id nightDwt_device_ids[] = {
    {
        .name = "SLD_NIGHTDWT_STRATUS",
    },
    {
        .name = "eb_011",
    },
    {
        .compatible = "sld,nightDwt_stratus",
    },
    {},
};

static int nightDwt_devs;

static inline struct nightDwt_stratus_device *to_nightDwt(struct esp_device *esp)
{
    return container_of(esp, struct nightDwt_stratus_device, esp);
}

static void nightDwt_prep_xfer(struct esp_device *esp, void *arg)
{
    struct nightDwt_stratus_access *a = arg;

    iowrite32be(a->nimages, esp->iomem + NIGHTDWT_NIMAGES_REG);
    iowrite32be(a->rows, esp->iomem + NIGHTDWT_ROWS_REG);
    iowrite32be(a->cols, esp->iomem + NIGHTDWT_COLS_REG);
    iowrite32be(a->do_dwt, esp->iomem + NIGHTDWT_DO_DWT_REG);
    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool nightDwt_xfer_input_ok(struct esp_device *esp, void *arg)
{
    struct nightDwt_stratus_access *a = arg;

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

static int nightDwt_probe(struct platform_device *pdev)
{
    struct nightDwt_stratus_device *nightDwt;
    struct esp_device *                esp;
    int                                rc;

    nightDwt = kzalloc(sizeof(*nightDwt), GFP_KERNEL);
    if (nightDwt == NULL)
        return -ENOMEM;
    esp         = &nightDwt->esp;
    esp->module = THIS_MODULE;
    esp->number = nightDwt_devs;
    esp->driver = &nightDwt_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    nightDwt_devs++;
    return 0;
err:
    kfree(nightDwt);
    return rc;
}

static int __exit nightDwt_remove(struct platform_device *pdev)
{
    struct esp_device *                esp         = platform_get_drvdata(pdev);
    struct nightDwt_stratus_device *nightDwt = to_nightDwt(esp);

    esp_device_unregister(esp);
    kfree(nightDwt);
    return 0;
}

static struct esp_driver nightDwt_driver = {
    .plat =
        {
            .probe  = nightDwt_probe,
            .remove = nightDwt_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = nightDwt_device_ids,
                },
        },
    .xfer_input_ok = nightDwt_xfer_input_ok,
    .prep_xfer     = nightDwt_prep_xfer,
    .ioctl_cm      = NIGHTDWT_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct nightDwt_stratus_access),
};

static int __init nightDwt_init(void)
{
    printk(KERN_ALERT "Hello world nightDwt!!\n");
    return esp_driver_register(&nightDwt_driver);
}

static void __exit nightDwt_exit(void) { esp_driver_unregister(&nightDwt_driver); }

module_init(nightDwt_init);
module_exit(nightDwt_exit);

MODULE_DEVICE_TABLE(of, nightDwt_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("nightDwt_stratus driver");
