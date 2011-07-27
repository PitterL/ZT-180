/***************************************************************************** 
 * encode.c 
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: main file of imapx200 media encode driver
 *
 * Author:
 *     Sololz <sololz@infotm.com>
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.1  01/12/2010 Sololz 
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/poll.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/clk.h>

#include <plat/clock.h>

#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>

#include "common.h"

/* functions declaration */
static int reset_hw_reg_encode(void);
static int encode_driver_register(void);
static int encode_driver_unregister(void);
static int encode_enable_hw_power(void);
static int encode_disable_hw_power(void);

static encode_param_t encode_param;		/* global variables structure */
static struct class *encode_class = NULL;	/* char device for imapx200 encode */
static struct clk *imap_venc_clock = NULL;	/* hardware clock */
static unsigned int encode_open_count = 0;	/* record encode open times */
static struct mutex encode_lock;
/* for poll system call */
static wait_queue_head_t wait_encode;
static volatile unsigned int encode_poll_mark = 0;

/* irq handle function */
static irqreturn_t imapx200_encode_irq_handle(int irq, void *dev_id)
{
	encode_param_t *dev = (encode_param_t *)dev_id;
	unsigned int irq_status = 0;

	/* get interrupt register status */
	irq_status = readl(dev->reg_base_virt_addr + 0x04);

	if (irq_status & 0x01) {
		writel(irq_status & (~0x01), dev->reg_base_virt_addr + 0x04);

		encode_poll_mark = 1;
		wake_up(&wait_encode);
		return IRQ_HANDLED;
	} else {
		encode_debug("An unknown interrupt detected\n");
		return IRQ_HANDLED;
	}
}

/* open */
static int imapx200_encode_open(struct inode *inode, struct file *file)
{
	mutex_lock(&encode_lock);
	if (encode_open_count == 0) {
		encode_enable_hw_power();
	}
	encode_open_count++;
	mutex_unlock(&encode_lock);

	file->private_data = (void *)(&encode_param);

	encode_debug("IMAPX200 Encode open OK\n");

	return ENCODE_RET_OK;
}

/* release */
static int imapx200_encode_release(struct inode *inode, struct file *file)
{
	mutex_lock(&encode_lock);
	encode_open_count--;
	if (encode_open_count == 0) {
		encode_disable_hw_power();
	}
	mutex_unlock(&encode_lock);

	encode_debug("IMAPX200 Encode release OK\n");

	return ENCODE_RET_OK;
}

/* ioctl */
static int imapx200_encode_ioctl(struct inode *inode, struct file *file, \
		unsigned int cmd, unsigned long arg)
{
	int ret = -1;

	/* cmd check */
	if (_IOC_TYPE(cmd) != HX280ENC_IOC_MAGIC) {
		return -ENOTTY;
	}
	if (_IOC_NR(cmd) > HX280ENC_IOC_MAXNR) {
		return -ENOTTY;
	}

	/* check command by command feature READ/WRITE */
	if (_IOC_DIR(cmd) & _IOC_READ) {
		ret = !access_ok(VERIFY_WRITE, (void *)arg, _IOC_SIZE(cmd));
	} else if (_IOC_DIR(cmd) & _IOC_WRITE) {
		ret = !access_ok(VERIFY_READ, (void *)arg, _IOC_SIZE(cmd));
	}
	if (ret) {
		return -EFAULT;
	}

	switch (cmd) {
		case HX280ENC_IOCGHWOFFSET:
			__put_user(encode_param.reg_base_phys_addr, (unsigned int *)arg);
			break;

		case HX280ENC_IOCGHWIOSIZE:
			__put_user(IMAPX200_ENCODE_ACT_REG_SIZE, (unsigned int *)arg);
			break;

		case HX280ENC_IOC_CLI:
		case HX280ENC_IOC_STI:
		case HX280ENC_IOCHARDRESET:
			encode_debug("current ioctl command unsupported yet\n");
			break;

		default:
			encode_error("encode driver unknow ioctl command\n");
			break;
	}

	return ENCODE_RET_OK;
}

/* poll */
static unsigned int imapx200_encode_poll(struct file *file, poll_table *wait)
{
	unsigned int mask = 0;

	poll_wait(file, &wait_encode, wait);

	if (encode_poll_mark != 0) {
		mask = POLLIN | POLLRDNORM;
		encode_poll_mark = 0;
	}

	return mask;
}

/* file operations */
static struct file_operations imapx200_encode_fops = {
	.owner = THIS_MODULE,
	.open = imapx200_encode_open,
	.release = imapx200_encode_release,
	.ioctl = imapx200_encode_ioctl,
	.poll = imapx200_encode_poll,
};

/* probe */
static int imapx200_encode_probe(struct platform_device *pdev)
{
	int ret = -1;
	unsigned int size = 0;
	struct resource *res = NULL;

	encode_debug("In imapx200_encode_probe\n");

	/* get encode hardware clock and enable it */
	imap_venc_clock = clk_get(&pdev->dev, "venc");
	if (imap_venc_clock == NULL) {
		encode_error("Fail to get encode hardware source\n");
		return -ENOENT;
	}
	clk_enable(imap_venc_clock);

	/* close 7280 ic if uboot has done something needs ic powered on */
	encode_disable_hw_power();

	/* initialize encode open count */
	encode_open_count = 0;

	/* initialize global varaibles */
	memset(&encode_param, 0x00, sizeof(encode_param_t));

	/* initialize wait queue for poll system call */
	init_waitqueue_head(&wait_encode);

	/* get register base resource */
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		encode_error("Fail to get encode platform device resource\n");
		return -ENOENT;
	}

	/* request memory region */
	size = res->end - res->start + 1;
	encode_param.reg_reserved_size = size;
	encode_param.resource_mem = request_mem_region(res->start, size, pdev->name);
	if (encode_param.resource_mem == NULL) {
		encode_error("Fail to get IMAPX200 encode register memory region\n");
		return -ENOENT;
	}
	encode_param.reg_base_phys_addr = res->start;

	/* remap register physics address nocache */
	encode_param.reg_base_virt_addr = ioremap_nocache(res->start, size);
	if (encode_param.reg_base_virt_addr == NULL) {
		encode_error("Fail to ioremap IMAPX200 encode register base address\n");
		return -EINVAL;
	}

	/* get and config irq */
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if (res == NULL) {
		encode_error("Fail to get IMAPX200 encode irq resource\n");
		return -ENOENT;
	}

	if (res->start != IRQ_VENC) {
		encode_error("Get wrong irq number for IMAPX200 encode\n");
		return -ENOENT;
	}

	/* request for irq */
	ret = request_irq(res->start, imapx200_encode_irq_handle, IRQF_DISABLED, \
			pdev->name, (void *)(&encode_param));
	if (ret) {
		encode_error("Fail to request irq for IMAPX200 encode device\n");
		return ret;
	}

	/* register driver device */
	if (encode_driver_register() != ENCODE_RET_OK) {
		return ENCODE_RET_ERROR;
	}

	/* reset hardware registers */
	reset_hw_reg_encode();

	mutex_init(&encode_lock);

	encode_debug("IMAPX200 encode driver probe OK, major: %d minor: %d\n", \
			encode_param.major, encode_param.minor);

	return ENCODE_RET_OK;
}

/* remove */
static int imapx200_encode_remove(struct platform_device *pdev)
{
	/* disable hardware */
	writel(0, encode_param.reg_base_virt_addr + 0x38);
	/* clean encode irq */
	writel(0, encode_param.reg_base_virt_addr + 0x04);

	/* unmap registers address */
	iounmap((void *)(encode_param.reg_base_virt_addr));

	/* release resource */
	release_mem_region(encode_param.reg_base_phys_addr, \
			encode_param.reg_reserved_size);

	/* release source */
	if (encode_param.resource_mem != NULL) {
		release_resource(encode_param.resource_mem);
		kfree(encode_param.resource_mem);
		encode_param.resource_mem = NULL;
	}

	/* release irq */
	free_irq(IRQ_VENC, pdev);

	/* unregister encode driver */
	encode_driver_unregister();

	/* disable encode clock */
	clk_disable(imap_venc_clock);

	mutex_destroy(&encode_lock);

	encode_debug("IMAPX200 encode driver remove OK\n");

	return ENCODE_RET_OK;
}

#if defined(CONFIG_PM)
/* suspend and resume */
static int imapx200_encode_suspend(struct platform_device *pdev, pm_message_t state)
{
	encode_disable_hw_power();
	return ENCODE_RET_OK;
}

static int imapx200_encode_resume(struct platform_device *pdev)
{
	mutex_lock(&encode_lock);
	if (encode_open_count > 0) {
		encode_enable_hw_power();
	}
	mutex_unlock(&encode_lock);
	return ENCODE_RET_OK;
}
#endif

static struct platform_driver imapx200_encode_driver = {
	.probe = imapx200_encode_probe,
	.remove = imapx200_encode_remove,
#if defined(CONFIG_PM)
	.suspend = imapx200_encode_suspend,
	.resume = imapx200_encode_resume,
#endif
	.driver = {
		.owner = THIS_MODULE,
		.name = "imapx200_venc",
	},
};

/*
 * init and exit
 */
static int __init imapx200_encode_init(void)
{
	if (platform_driver_register(&imapx200_encode_driver)) {
		encode_error("Fail to register platform driver for IMAPX200 Encoder Driver\n");
		return -EPERM;
	}

	encode_debug("IMAPX200 Hantro 7280 Encode Driver init OK\n");

	return ENCODE_RET_OK;
}

static void __exit imapx200_encode_exit(void)
{
	platform_driver_unregister(&imapx200_encode_driver);

	encode_debug("IMAPX200 Encode Driver exit OK\n");
}

module_init(imapx200_encode_init);
module_exit(imapx200_encode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");
MODULE_DESCRIPTION("IMAPX200 Encode Driver");

/* reset hardware */
int reset_hw_reg_encode(void)
{
	int i = 0;

	/* I don't know why write 0x38 address first */
	writel(0, encode_param.reg_base_virt_addr + 0x38);

	for (i = 4; i < IMAPX200_ENCODE_ACT_REG_SIZE; i += 4) {
		writel(0, encode_param.reg_base_virt_addr + i);
	}

	return ENCODE_RET_OK;
}

/*
 * register and unregister function to treat this device as char device
 */
int encode_driver_register(void)
{
	struct device *encdev = NULL;

	/* encode char device node create */
	encode_param.major = register_chrdev(ENCODE_DEFAULT_MAJOR, \
			"imapx200-venc", &imapx200_encode_fops);
	if (encode_param.major < 0) {
		encode_error("Register char device for encode error\n");
		goto register_error_1;
	}
	encode_param.major = ENCODE_DEFAULT_MAJOR;

	/* create a new char device class */
	encode_class = class_create(THIS_MODULE, "imapx200-venc");
	if (encode_class == NULL) {
		encode_error("Encode driver char device class create error\n");
		goto register_error_2;
	}

	/* get major and minor number by MKDEV() */
	encode_param.dev = MKDEV(encode_param.major, ENCODE_DEFAULT_MINOR);
	if (MAJOR(encode_param.dev) != encode_param.major) {
		encode_error("Encode driver MKDEV error\n");
		goto register_error_3;
	}
	encode_param.minor = MINOR(encode_param.dev);

	/* mknod in filesystem using device_create() */
	encdev = NULL;
	encdev = device_create(encode_class, NULL, encode_param.dev, NULL, "imapx200-venc");	/* this function mknod in filesystem */
	if (encdev == NULL) {
		encode_error("Encode driver create node error\n");
		goto register_error_3;
	}

	return ENCODE_RET_OK;

register_error_3:
	class_destroy(encode_class);
register_error_2:
	unregister_chrdev(encode_param.major, "imapx200-venc");
register_error_1:
	return ENCODE_RET_ERROR;
}

int encode_driver_unregister(void)
{
	/* all void return functions */
	device_destroy(encode_class, encode_param.dev);
	class_destroy(encode_class);
	unregister_chrdev(encode_param.major, "imapx200-venc");

	return ENCODE_RET_OK;
}

int encode_enable_hw_power(void)
{
	int i = 0;
	unsigned int reg_val = 0;

	/* reset encode hardware */

	reg_val = readl(rAHBP_RST);
	reg_val |= (1 << 1);
	writel(reg_val, rAHBP_RST);
	for (i = 0; i < 0x1000; i++);
	writel(readl(rAHBP_RST) & ~(1 << 1), rAHBP_RST);

	/* enable encode */
	reg_val = readl(rAHBP_EN);
	reg_val |= (1 << 1);
	writel(reg_val, rAHBP_EN);

	/* set encode power */
	reg_val = readl(rNPOW_CFG);
	reg_val |= (1 << 3);
	writel(reg_val, rNPOW_CFG);
	while (!(readl(rPOW_ST) & (1 << 3)));

	/* reset encode power */
	reg_val = readl(rMD_RST);
	reg_val |= (1 << 3);
	writel(reg_val, rMD_RST);
	for (i = 0; i < 0x1000; i++) ;
	writel(readl(rMD_RST) & ~(1 << 3), rMD_RST);

	/* isolate encode */
	reg_val = readl(rMD_ISO);
	reg_val &= ~(1 << 3);
	writel(reg_val, rMD_ISO);

#if defined(CONFIG_IMAP_ENCODE_IC_POWER_TRACE)
	printk(KERN_ALERT "[KERN ENC POWER TRACE] 7280 is powered on\n");
#endif

	return ENCODE_RET_OK;
}

int encode_disable_hw_power(void)
{
	unsigned int reg_val = 0;

	/* unisolate encode */
	reg_val = readl(rMD_ISO);
	reg_val |= (1 << 3);
	writel(reg_val, rMD_ISO);

	/* shut encode power */
	reg_val = readl(rNPOW_CFG);
	reg_val &= ~(1 << 3);
	writel(reg_val, rNPOW_CFG);
	while ((readl(rPOW_ST) & (1 << 3)));

	/* disable encode */
	reg_val = readl(rAHBP_EN);
	reg_val &= ~(1 << 1);
	writel(reg_val, rAHBP_EN);

#if defined(CONFIG_IMAP_ENCODE_IC_POWER_TRACE)
	printk(KERN_ALERT "[KERN ENC POWER TRACE] 7280 is powered off\n");
#endif

	return ENCODE_RET_OK;
}
