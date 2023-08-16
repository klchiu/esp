// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include <linux/log2.h>

#include "nightHistEq_stratus.h"

#define DRV_NAME "nightHistEq_stratus"

#define NIGHTHISTEQ_NIMAGES_REG 0x40
#define NIGHTHISTEQ_ROWS_REG    0x44
#define NIGHTHISTEQ_COLS_REG    0x48
#define NIGHTHISTEQ_DO_DWT_REG  0x4C

struct nightHistEq_stratus_device {
    struct esp_device esp;
};

static struct esp_driver nightHistEq_driver;

static struct of_device_id nightHistEq_device_ids[] = {
    {
        .name = "SLD_NIGHTHISTEQ_STRATUS",
    },
    {
        .name = "eb_011",
    },
    {
        .compatible = "sld,nightHistEq_stratus",
    },
    {},
};

static int nightHistEq_devs;

static inline struct nightHistEq_stratus_device *to_nightHistEq(struct esp_device *esp)
{
    return container_of(esp, struct nightHistEq_stratus_device, esp);
}

static void nightHistEq_prep_xfer(struct esp_device *esp, void *arg)
{
    struct nightHistEq_stratus_access *a = arg;

    iowrite32be(a->nimages, esp->iomem + NIGHTHISTEQ_NIMAGES_REG);
    iowrite32be(a->rows, esp->iomem + NIGHTHISTEQ_ROWS_REG);
    iowrite32be(a->cols, esp->iomem + NIGHTHISTEQ_COLS_REG);
    iowrite32be(a->do_dwt, esp->iomem + NIGHTHISTEQ_DO_DWT_REG);
    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool nightHistEq_xfer_input_ok(struct esp_device *esp, void *arg)
{
    struct nightHistEq_stratus_access *a = arg;

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

static int nightHistEq_probe(struct platform_device *pdev)
{
    struct nightHistEq_stratus_device *nightHistEq;
    struct esp_device *                esp;
    int                                rc;

    nightHistEq = kzalloc(sizeof(*nightHistEq), GFP_KERNEL);
    if (nightHistEq == NULL)
        return -ENOMEM;
    esp         = &nightHistEq->esp;
    esp->module = THIS_MODULE;
    esp->number = nightHistEq_devs;
    esp->driver = &nightHistEq_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    nightHistEq_devs++;
    return 0;
err:
    kfree(nightHistEq);
    return rc;
}

static int __exit nightHistEq_remove(struct platform_device *pdev)
{
    struct esp_device *                esp         = platform_get_drvdata(pdev);
    struct nightHistEq_stratus_device *nightHistEq = to_nightHistEq(esp);

    esp_device_unregister(esp);
    kfree(nightHistEq);
    return 0;
}

static struct esp_driver nightHistEq_driver = {
    .plat =
        {
            .probe  = nightHistEq_probe,
            .remove = nightHistEq_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = nightHistEq_device_ids,
                },
        },
    .xfer_input_ok = nightHistEq_xfer_input_ok,
    .prep_xfer     = nightHistEq_prep_xfer,
    .ioctl_cm      = NIGHTHISTEQ_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct nightHistEq_stratus_access),
};

static int __init nightHistEq_init(void)
{
    printk(KERN_ALERT "Hello world nightHistEq!!\n");
    return esp_driver_register(&nightHistEq_driver);
}

static void __exit nightHistEq_exit(void) { esp_driver_unregister(&nightHistEq_driver); }

module_init(nightHistEq_init);
module_exit(nightHistEq_exit);

MODULE_DEVICE_TABLE(of, nightHistEq_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("nightHistEq_stratus driver");
