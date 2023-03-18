// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/log2.h>
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include "tf_sub3_stratus.h"

#define DRV_NAME "tf_sub3_stratus"

#define TF_LENGTH_REG          0x40
#define TF_SRC_DST_OFFSET_0_REG 0x44
#define TF_SRC_DST_OFFSET_1_REG 0x48
#define TF_SRC_DST_OFFSET_2_REG 0x4C

struct tf_sub3_stratus_device {
    struct esp_device esp;
};

static struct esp_driver tf_sub3_driver;

static struct of_device_id tf_sub3_device_ids[] = {
    {
        .name = "SLD_TF_SUB3_STRATUS",
    },
    {
        .name = "eb_07C",
    },
    {
        .compatible = "sld,tf_sub3_stratus",
    },
    {},
};

static int tf_sub3_devs;

static inline struct tf_sub3_stratus_device *to_tf_sub3(struct esp_device *esp)
{
    return container_of(esp, struct tf_sub3_stratus_device, esp);
}

static void tf_sub3_prep_xfer(struct esp_device *esp, void *arg)
{
    struct tf_sub3_stratus_access *a = arg;

    iowrite32be(a->tf_length, esp->iomem + TF_LENGTH_REG);
    iowrite32be(a->tf_src_dst_offset_0, esp->iomem + TF_SRC_DST_OFFSET_0_REG);
    iowrite32be(a->tf_src_dst_offset_1, esp->iomem + TF_SRC_DST_OFFSET_1_REG);
    iowrite32be(a->tf_src_dst_offset_2, esp->iomem + TF_SRC_DST_OFFSET_2_REG);
    
    iowrite32be(a->src_offset, esp->iomem + SRC_OFFSET_REG);
    iowrite32be(a->dst_offset, esp->iomem + DST_OFFSET_REG);
}

static bool tf_sub3_xfer_input_ok(struct esp_device *esp, void *arg)
{
    // struct tf_sub3_stratus_access *a = arg;

    return true;
}

static int tf_sub3_probe(struct platform_device *pdev)
{
    struct tf_sub3_stratus_device *tf_sub3;
    struct esp_device *              esp;
    int                              rc;

    tf_sub3 = kzalloc(sizeof(*tf_sub3), GFP_KERNEL);
    if (tf_sub3 == NULL)
        return -ENOMEM;
    esp         = &tf_sub3->esp;
    esp->module = THIS_MODULE;
    esp->number = tf_sub3_devs;
    esp->driver = &tf_sub3_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    tf_sub3_devs++;
    return 0;
err:
    kfree(tf_sub3);
    return rc;
}

static int __exit tf_sub3_remove(struct platform_device *pdev)
{
    struct esp_device *              esp       = platform_get_drvdata(pdev);
    struct tf_sub3_stratus_device *tf_sub3 = to_tf_sub3(esp);

    esp_device_unregister(esp);
    kfree(tf_sub3);
    return 0;
}

static struct esp_driver tf_sub3_driver = {
    .plat =
        {
            .probe  = tf_sub3_probe,
            .remove = tf_sub3_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = tf_sub3_device_ids,
                },
        },
    .xfer_input_ok = tf_sub3_xfer_input_ok,
    .prep_xfer     = tf_sub3_prep_xfer,
    .ioctl_cm      = TF_SUB3_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct tf_sub3_stratus_access),
};

static int __init tf_sub3_init(void) { return esp_driver_register(&tf_sub3_driver); }

static void __exit tf_sub3_exit(void) { esp_driver_unregister(&tf_sub3_driver); }

module_init(tf_sub3_init) module_exit(tf_sub3_exit)

    MODULE_DEVICE_TABLE(of, tf_sub3_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("tf_sub3_stratus driver");
