// Copyright (c) 2011-2021 Columbia University, System Level Design Group
// SPDX-License-Identifier: Apache-2.0
#include <linux/log2.h>
#include <linux/mm.h>
#include <linux/of_device.h>

#include <asm/io.h>

#include <esp.h>
#include <esp_accelerator.h>

#include "ny_acc_stratus.h"

#define DRV_NAME "ny_acc_stratus"

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

struct ny_acc_stratus_device {
    struct esp_device esp;
};

static struct esp_driver ny_acc_driver;

static struct of_device_id ny_acc_device_ids[] = {
    {
        .name = "SLD_NY_ACC_STRATUS",
    },
    {
        .name = "eb_07C",
    },
    {
        .compatible = "sld,ny_acc_stratus",
    },
    {},
};

static int ny_acc_devs;

static inline struct ny_acc_stratus_device *to_ny_acc(struct esp_device *esp)
{
    return container_of(esp, struct ny_acc_stratus_device, esp);
}

static void ny_acc_prep_xfer(struct esp_device *esp, void *arg)
{
    struct ny_acc_stratus_access *a = arg;

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

static bool ny_acc_xfer_input_ok(struct esp_device *esp, void *arg)
{
    // struct ny_acc_stratus_access *a = arg;

    return true;
}

static int ny_acc_probe(struct platform_device *pdev)
{
    struct ny_acc_stratus_device *ny_acc;
    struct esp_device *              esp;
    int                              rc;

    ny_acc = kzalloc(sizeof(*ny_acc), GFP_KERNEL);
    if (ny_acc == NULL)
        return -ENOMEM;
    esp         = &ny_acc->esp;
    esp->module = THIS_MODULE;
    esp->number = ny_acc_devs;
    esp->driver = &ny_acc_driver;
    rc          = esp_device_register(esp, pdev);
    if (rc)
        goto err;

    ny_acc_devs++;
    return 0;
err:
    kfree(ny_acc);
    return rc;
}

static int __exit ny_acc_remove(struct platform_device *pdev)
{
    struct esp_device *              esp       = platform_get_drvdata(pdev);
    struct ny_acc_stratus_device *ny_acc = to_ny_acc(esp);

    esp_device_unregister(esp);
    kfree(ny_acc);
    return 0;
}

static struct esp_driver ny_acc_driver = {
    .plat =
        {
            .probe  = ny_acc_probe,
            .remove = ny_acc_remove,
            .driver =
                {
                    .name           = DRV_NAME,
                    .owner          = THIS_MODULE,
                    .of_match_table = ny_acc_device_ids,
                },
        },
    .xfer_input_ok = ny_acc_xfer_input_ok,
    .prep_xfer     = ny_acc_prep_xfer,
    .ioctl_cm      = NY_ACC_STRATUS_IOC_ACCESS,
    .arg_size      = sizeof(struct ny_acc_stratus_access),
};

static int __init ny_acc_init(void) { return esp_driver_register(&ny_acc_driver); }

static void __exit ny_acc_exit(void) { esp_driver_unregister(&ny_acc_driver); }

module_init(ny_acc_init) module_exit(ny_acc_exit)

    MODULE_DEVICE_TABLE(of, ny_acc_device_ids);

MODULE_AUTHOR("SLD Group");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("ny_acc_stratus driver");
