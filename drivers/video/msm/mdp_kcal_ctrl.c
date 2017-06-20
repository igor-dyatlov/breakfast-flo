/*
 * Copyright (c) 2011-2013, The Linux Foundation. All rights reserved.
 * Copyright (c) 2013, LGE Inc. All rights reserved
 * Copyright (c) 2014 savoca <adeddo27@gmail.com>
 * Copyright (c) 2014 Paul Reioux <reioux@gmail.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/init.h>
#include <linux/module.h>

#include "msm_fb.h"
#include "mdp4.h"

#define DEF_PCC 0x100
#define PCC_ADJ 0x80
#define MDP4_PCC_RETRY 3

struct kcal_pcc_data {
	int red;
	int green;
	int blue;
	int minimum;
	int enable;
	int invert;
};

static void mdp_kcal_update_pcc(struct kcal_pcc_data *pcc_data)
{
	int retry;
	struct mdp_pcc_cfg_data pcc_config;

	memset(&pcc_config, 0, sizeof(struct mdp_pcc_cfg_data));

	pcc_data->red = pcc_data->red < pcc_data->minimum ?
		pcc_data->minimum : pcc_data->red;
	pcc_data->green = pcc_data->green < pcc_data->minimum ?
		pcc_data->minimum : pcc_data->green;
	pcc_data->blue = pcc_data->blue < pcc_data->minimum ?
		pcc_data->minimum : pcc_data->blue;

	pcc_config.block = MDP_BLOCK_DMA_P;
	pcc_config.ops = pcc_data->enable ?
		MDP_PP_OPS_WRITE | MDP_PP_OPS_ENABLE :
		MDP_PP_OPS_WRITE | MDP_PP_OPS_DISABLE;
	pcc_config.r.r = pcc_data->red * PCC_ADJ;
	pcc_config.g.g = pcc_data->green * PCC_ADJ;
	pcc_config.b.b = pcc_data->blue * PCC_ADJ;

	if (pcc_data->invert) {
		pcc_config.r.c = pcc_config.g.c = pcc_config.b.c = 0x7ff8;
		pcc_config.r.r |= (0xffff << 16);
		pcc_config.g.g |= (0xffff << 16);
		pcc_config.b.b |= (0xffff << 16);
	}

	for (retry = 0; retry < MDP4_PCC_RETRY; retry++) {
		if (mdp4_pcc_cfg(&pcc_config))
			break;
		else
			msleep(10);
	}
}

static ssize_t kcal_store(struct device *dev,
			  struct device_attribute *attr,
			  const char *buf, size_t count)
{
	int kcal_r, kcal_g, kcal_b, r;
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	r = sscanf(buf, "%d %d %d", &kcal_r, &kcal_g, &kcal_b);
	if ((r != 3) ||
	    (kcal_r < 1 || kcal_r > 256) ||
	    (kcal_g < 1 || kcal_g > 256) ||
	    (kcal_b < 1 || kcal_b > 256))
		return -EINVAL;

	pcc_data->red = kcal_r;
	pcc_data->green = kcal_g;
	pcc_data->blue = kcal_b;

	mdp_kcal_update_pcc(pcc_data);

	return count;
}

static ssize_t kcal_show(struct device *dev,
			 struct device_attribute *attr,
			 char *buf)
{
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d %d %d\n",
			 pcc_data->red, pcc_data->green, pcc_data->blue);
}

static ssize_t kcal_min_store(struct device *dev,
			      struct device_attribute *attr,
			      const char *buf, size_t count)
{
	int kcal_min, r;
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	r = kstrtoint(buf, 10, &kcal_min);
	if ((r) ||
	    (kcal_min < 1 || kcal_min > 256))
		return -EINVAL;

	pcc_data->minimum = kcal_min;

	mdp_kcal_update_pcc(pcc_data);

	return count;
}

static ssize_t kcal_min_show(struct device *dev,
			     struct device_attribute *attr,
			     char *buf)
{
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pcc_data->minimum);
}

static ssize_t kcal_enable_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	int kcal_enable, r;
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	r = kstrtoint(buf, 10, &kcal_enable);
	if ((r) ||
	    (kcal_enable != 0 && kcal_enable != 1) ||
	    (pcc_data->enable == kcal_enable))
		return -EINVAL;

	pcc_data->enable = kcal_enable;

	mdp_kcal_update_pcc(pcc_data);

	return count;
}

static ssize_t kcal_enable_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pcc_data->enable);
}

static ssize_t kcal_invert_store(struct device *dev,
				 struct device_attribute *attr,
				 const char *buf, size_t count)
{
	int kcal_invert, r;
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	r = kstrtoint(buf, 10, &kcal_invert);
	if ((r) ||
	    (kcal_invert != 0 && kcal_invert != 1) ||
	    (pcc_data->invert == kcal_invert))
		return -EINVAL;

	pcc_data->invert = kcal_invert;

	mdp_kcal_update_pcc(pcc_data);

	return count;
}

static ssize_t kcal_invert_show(struct device *dev,
				struct device_attribute *attr,
				char *buf)
{
	struct kcal_pcc_data *pcc_data = dev_get_drvdata(dev);

	return scnprintf(buf, PAGE_SIZE, "%d\n", pcc_data->invert);
}

static DEVICE_ATTR(kcal, S_IWUSR | S_IRUGO,
	kcal_show, kcal_store);
static DEVICE_ATTR(kcal_min, S_IWUSR | S_IRUGO,
	kcal_min_show, kcal_min_store);
static DEVICE_ATTR(kcal_enable, S_IWUSR | S_IRUGO,
	kcal_enable_show, kcal_enable_store);
static DEVICE_ATTR(kcal_invert, S_IWUSR | S_IRUGO,
	kcal_invert_show, kcal_invert_store);

static int kcal_ctrl_probe(struct platform_device *pdev)
{
	int ret;
	struct kcal_pcc_data *pcc_data;

	pcc_data = devm_kzalloc(&pdev->dev, sizeof(*pcc_data), GFP_KERNEL);
	if (!pcc_data) {
		pr_err("%s: failed to allocate memory for pcc_data\n",
			__func__);
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, pcc_data);

	pcc_data->enable = 0x1;
	pcc_data->red = DEF_PCC;
	pcc_data->green = DEF_PCC;
	pcc_data->blue = DEF_PCC;
	pcc_data->minimum = 0x23;
	pcc_data->invert = 0x0;

	mdp_kcal_update_pcc(pcc_data);

	ret = device_create_file(&pdev->dev, &dev_attr_kcal);
	ret |= device_create_file(&pdev->dev, &dev_attr_kcal_min);
	ret |= device_create_file(&pdev->dev, &dev_attr_kcal_enable);
	ret |= device_create_file(&pdev->dev, &dev_attr_kcal_invert);
	if (ret) {
		pr_err("%s: unable to create sysfs entries\n", __func__);
		return ret;
	}

	return 0;
}

static int kcal_ctrl_remove(struct platform_device *pdev)
{
	device_remove_file(&pdev->dev, &dev_attr_kcal);
	device_remove_file(&pdev->dev, &dev_attr_kcal_min);
	device_remove_file(&pdev->dev, &dev_attr_kcal_enable);
	device_remove_file(&pdev->dev, &dev_attr_kcal_invert);

	return 0;
}

static struct platform_driver kcal_ctrl_driver = {
	.probe = kcal_ctrl_probe,
	.remove = kcal_ctrl_remove,
	.driver = {
		.name = "kcal_ctrl",
	},
};

static struct platform_device kcal_ctrl_device = {
	.name = "kcal_ctrl",
};

static int __init kcal_ctrl_init(void)
{
	if (platform_driver_register(&kcal_ctrl_driver))
		return -ENODEV;

	if (platform_device_register(&kcal_ctrl_device))
		return -ENODEV;

	pr_info("%s: registered\n", __func__);

	return 0;
}

static void __exit kcal_ctrl_exit(void)
{
	platform_device_unregister(&kcal_ctrl_device);
	platform_driver_unregister(&kcal_ctrl_driver);
}

module_init(kcal_ctrl_init);
module_exit(kcal_ctrl_exit);
