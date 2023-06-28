// Copyright (c) 2011-2023 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include <linux/log2.h>

#include "nightNF_stratus.h"

#define DRV_NAME "nightNF_stratus"

#define NIGHTNF_NIMAGES_REG 0x40
#define NIGHTNF_ROWS_REG    0x44
#define NIGHTNF_COLS_REG    0x48
#define NIGHTNF_DO_DWT_REG  0x4C

struct nightNF_stratus_device {
    struct esp_device esp;
};

static struct esp_driver nightNF_driver;

static struct of_device_id nightNF_device_ids[] = {
    {
        .name = "SLD_NIGHTNF_STRATUS",
    },
    {
        .name = "eb_011",
    },
    {
        .compatible = "sld,nightNF_stratus",
    },
    {},
};

static int nightNF_devs;

static inline struct nightNF_stratus_device *to_nightNF(struct esp_device *esp)
{
    return container_of(esp, struct nightNF_stratus_device, esp);
}

static void nightNF_prep_xfer(struct esp_device *esp, void *arg)
{
    struct nightNF_stratus_access *a = arg;

    iowrite32be(a->nimages, esp->iomem + NIGHTNF_NIMAGES_REG);
    iowrite32be(a->rows, esp->iomem + NIGHTNF_ROWS_REG);
    iowrite32be(a->cols, esp->iomem + NIGHTNF_COLS_REG);
    iowrite32be(a->do_dwt, esp->iomem + NIGHTNF_DO_DWT_REG);
    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool nightNF_xfer_input_ok(struct esp_device *esp, void *arg)
{
    struct nightNF_stratus_access *a = arg;

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

static int nightNF_probe(struct platform_device *pdev)
{
    struct nightNF_stratus_device *nightNF;
    struct esp_device *                esp;
    int                                rc;

    nightNF = kzalloc(sizeof(*nightNF), GFP_KERNEL);
    if (nightNF == NULL)
        return -ENOMEM;
    esp         = &nightNF->esp;
    esp->module = THIS_MODULE;
    esp->number = nightNF_devs;
    esp->driver = &nightNF_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    nightNF_devs++;
    return 0;
err:
    kfree(nightNF);
    return rc;
}

static int __exit nightNF_remove(struct platform_device *pdev)
{
    struct esp_device *                esp         = platform_get_drvdata(pdev);
    struct nightNF_stratus_device *nightNF = to_nightNF(esp);

    esp_device_unregister(esp);
    kfree(nightNF);
    return 0;
}

static struct esp_driver nightNF_driver = {
    .plat =
        {
            .probe  = nightNF_probe,
            .remove = nightNF_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = nightNF_device_ids,
                },
        },
    .xfer_input_ok = nightNF_xfer_input_ok,
    .prep_xfer     = nightNF_prep_xfer,
    .ioctl_cm      = NIGHTNF_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct nightNF_stratus_access),
};

static int __init nightNF_init(void)
{
    printk(KERN_ALERT "Hello world nightNF!!\n");
    return esp_driver_register(&nightNF_driver);
}

static void __exit nightNF_exit(void) { esp_driver_unregister(&nightNF_driver); }

module_init(nightNF_init);
module_exit(nightNF_exit);

MODULE_DEVICE_TABLE(of, nightNF_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("nightNF_stratus driver");
