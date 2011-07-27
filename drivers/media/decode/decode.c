/***************************************************************************** 
 * decode.c 
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: main file of imapx200 media decode driver
 *
 * Author:
 *     Sololz <sololz@infotm.com>
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.1  2009/12/09 Sololz 
 * 	Create: driver structure, support return register address to 
 * 	user space and response interrupt.
 * 1.2  2010/05/06 Sololz
 * 	Add: add poll mode to inform user space of interrupt.
 * 1.3  2010/06/13 Sololz
 * 	Modify: memory pool enable mode changed to fix conflict with 
 * 	memalloc driver.
 * 1.4	2010/10/21 Sololz
 * 	Add: communication with memalloc driver to decide mpool power.
 ******************************************************************************/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/poll.h>  
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/types.h>    
#include <linux/interrupt.h>
#include <linux/init.h>      
#include <linux/string.h>
#include <linux/mm.h>             
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/clk.h>

/*
 * XXX: decode hard ware directly connect to system bus, there is 
 * no use to clock set
 */ 
#include <plat/clock.h>

#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>

#include "common.h"

/*
 * functions delare
 */
static int reset_hw_reg_decode(void);		/* this function just set all resigster to be 0 */
static int decode_driver_register(void);	/* register driver as an char device */
static int decode_driver_unregister(void);
static void decode_enable_hw_power(void);	/* do it when first open */
static void decode_disable_hw_power(void);	/* do it when last close */

/*
 * this structure include global varaibles
 */
decode_param_t decode_param;			/* global variables group */
static struct clk *imap_vdec_clock;		/* decode hardware clock */
static unsigned int decode_open_count;		/* record open syscall times */
static struct mutex decode_lock;		/* mainly lock for decode instance count */
#ifdef CONFIG_IMAP_DECODE_POLL_MODE
static wait_queue_head_t wait_decode;		/* a wait queue for poll system call */
static volatile unsigned int dec_poll_mark = 0;
static volatile unsigned int pp_poll_mark = 0;
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

#if !defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
static void reset_mp_mark(void);		/* reset memory pool init mark */
static volatile unsigned int mp_mark = 0;	/* mark for memory pool init */
#else	/* CONFIG_IMAP_MMPOOL_MANAGEMENT */
extern int power_on_mpool_in_memalloc(void);
extern int power_off_mpool_in_memalloc(void);
#endif	/* N CONFIG_IMAP_MMPOOL_MANAGEMENT */

/* 
 * This irq handle function should never be reexecute at the same
 * time. Pp and decode share the same interrupt signal thread, in 
 * pp external mode, a finished interruption refer to be a pp irq, 
 * but in pp pipeline mode(decode pipeline with decode), recieved 
 * irq refers to be a decode interruption.
 * In this driver, we use System V signal asynchronization thread
 * communication. So you should set System V in you kernel make 
 * menuconfig.
 * ATTENTION: if your application runs in multi-thread mode, be 
 * aware that if you set your application getting this signal
 * as process mode, the signal will be send to your application's
 * main thread, and signal will never be processed twice. So in 
 * this case you won't get anything in your derived thread.
 */
static irqreturn_t imapx200_decode_irq_handle(int irq, void *dev_id)
{
	unsigned int handled;
	unsigned int irq_status_dec;
	unsigned int irq_status_pp;
	decode_param_t *dev;
	
	handled = 0;
	dev = (decode_param_t *)dev_id;

	/* interrupt status register read */
	irq_status_dec = readl(dev->reg_base_virt_addr + IMAPX200_DECODE_IRQ_STAT_DEC * 4);
	irq_status_pp = readl(dev->reg_base_virt_addr + IMAPX200_DECODE_IRQ_STAT_PP * 4);

	/* this structure is to enable irq in irq */
	if((irq_status_dec & IMAPX200_DECODE_IRQ_BIT_DEC) || (irq_status_pp & IMAPX200_DECODE_IRQ_BIT_PP))
	{
		if(irq_status_dec & IMAPX200_DECODE_IRQ_BIT_DEC)
		{
#ifdef CONFIG_IMAP_DEC_HW_PERFORMANCE
			do_gettimeofday(&(dev->end_time));
#endif
			/* clear irq */
			writel(irq_status_dec & (~IMAPX200_DECODE_IRQ_BIT_DEC), \
					dev->reg_base_virt_addr + IMAPX200_DECODE_IRQ_STAT_DEC * 4);

#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
			/* fasync kill for decode instances to send signal */
			if(dev->async_queue_dec != NULL)
				kill_fasync(&(dev->async_queue_dec), SIGIO, POLL_IN);
			else
				decode_alert("IMAPX200 Decode dec received w/o anybody waiting for it\n");
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */

#ifdef CONFIG_IMAP_DECODE_POLL_MODE
			dec_poll_mark = 1;
			wake_up(&wait_decode);
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

			decode_debug("IMAPX200 Decode get dec irq\n");
		}

		/* In pp pipeline mode, only one decode interrupt will be set */
		if(irq_status_pp & IMAPX200_DECODE_IRQ_BIT_PP)
		{
#ifdef CONFIG_IMAP_DEC_HW_PERFORMANCE
			do_gettimeofday(&(dev->end_time));
#endif
			/* clear irq */
			writel(irq_status_dec & (~IMAPX200_DECODE_IRQ_BIT_PP), \
					dev->reg_base_virt_addr + IMAPX200_DECODE_IRQ_STAT_PP * 4);

#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
			/* fasync kill for pp instance */
			if(dev->async_queue_pp != NULL)
				kill_fasync(&dev->async_queue_pp, SIGIO, POLL_IN);
			else
				decode_alert("IMAPX200 Decode pp received w/o anybody waiting for it\n");
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */

#ifdef CONFIG_IMAP_DECODE_POLL_MODE
			pp_poll_mark = 1;
			wake_up(&wait_decode);
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

			 decode_debug("IMAPX200 Decode get pp irq\n");
		}
		
		handled = 1;
	}
	else
		decode_debug("IMAPX200 Decode Driver get an unknown irq\n");

	return IRQ_RETVAL(handled);
}

#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
/*
 * File operations, this system call should be called before
 * any interrupt occurs. You can call fcntl system call to set 
 * driver node file property in order to get asynchronization
 * signal.
 */
static int imapx200_decode_fasync(int fd, struct file *file, int mode)
{
	decode_param_t *dev;
	struct fasync_struct **async_queue;

	dev = &decode_param;
	async_queue = NULL;

	/* select whitch interrupt this instance will sign up for */
	if((unsigned int *)(file->private_data) == &(decode_param.dec_instance))
	{
		decode_debug("IMAPX200 Decode dec fasync called\n");
		async_queue = &(decode_param.async_queue_dec);
	}
	else if((unsigned int *)(file->private_data) == &(decode_param.pp_instance))
	{
		decode_debug("IMAPX200 Decode pp fasync called\n");
		async_queue = &(decode_param.async_queue_pp);
	}
	else
		decode_error("IMAPX200 Decode wrong fasync called\n");

	return fasync_helper(fd, file, mode, async_queue);
}
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */

#ifdef CONFIG_IMAP_DECODE_POLL_MODE
/*
 * poll system call, a little bit slower than signal mode
 */
static unsigned int imapx200_decode_poll(struct file *file, poll_table *wait)
{
	unsigned int mask;

	mask = 0;

	poll_wait(file, &wait_decode, wait);

	if(file->private_data == &(decode_param.dec_instance))
	{
		if(dec_poll_mark != 0)
		{
			mask = POLLIN | POLLRDNORM;
			dec_poll_mark = 0;
		}
	}
	else if(file->private_data == &(decode_param.pp_instance))
	{
		if(pp_poll_mark != 0)
		{
			mask = POLLIN | POLLRDNORM;
			pp_poll_mark = 0;
		}
	}
	else
	{
		dec_poll_mark = 0;
		pp_poll_mark = 0;
		mask = POLLERR;
	}

	return mask;
}
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

/*
 * open system call just mark file private data as a decode 
 * instance by default, and you can change it by a ioctl call
 */
static int imapx200_decode_open(struct inode *inode, struct file *file)
{
	mutex_lock(&decode_lock);
	if(decode_open_count == 0)
	{
#if defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
		power_on_mpool_in_memalloc();
#endif	/* CONFIG_IMAP_MMPOOL_MANAGEMENT */
		decode_enable_hw_power();
	}
	decode_open_count++;
	mutex_unlock(&decode_lock);

	/* dec instance by default, you can change it by ioctl pp instance */
	file->private_data = &(decode_param.dec_instance);

	decode_debug("IMAPX200 Decode open OK\n");

	return IMAPX200_DECODE_RET_OK;
}

/*
 * fasync system call be called here
 */
static int imapx200_decode_release(struct inode *inode, struct file *file)
{
#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
	/* reset decode driver node file property */
	if(file->f_flags & FASYNC)
		imapx200_decode_fasync(-1, file, 0);
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */

	mutex_lock(&decode_lock);
	decode_open_count--;
	if(decode_open_count == 0)
	{
		decode_disable_hw_power();
#if defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
		power_off_mpool_in_memalloc();
#endif	/* CONFIG_IMAP_MMPOOL_MANAGEMENT */
	}
	mutex_unlock(&decode_lock);

	decode_debug("IMAPX200 Decode release OK\n");

	return IMAPX200_DECODE_RET_OK;
}

static int imapx200_decode_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg)
{
	int ret;
	int out;	/* for copy from/to user */

#ifdef CONFIG_IMAP_DEC_HW_PERFORMANCE
	struct timeval end_time_arg;
#endif

	ret = -1;
	out = -1;

	/* cmd check */
	if(_IOC_TYPE(cmd) != IMAPX200_DECODE_MAGIC)
		return -ENOTTY;
	if(_IOC_NR(cmd) > IMAPX200_DECODE_MAX_CMD)
		return -ENOTTY;

	/* check command by command feature READ/WRITE */
	if(_IOC_DIR(cmd) & _IOC_READ)
		ret = !access_ok(VERIFY_WRITE, (void *) arg, _IOC_SIZE(cmd));
	else if(_IOC_DIR(cmd) & _IOC_WRITE)
		ret = !access_ok(VERIFY_READ, (void *) arg, _IOC_SIZE(cmd));
	if(ret)
		return -EFAULT;

	switch(cmd)
	{
		case IMAPX200_IRQ_DISABLE:
			disable_irq(IRQ_VDEC);
			break;

		case IMAPX200_IRQ_ENABLE:
			enable_irq(IRQ_VDEC);
			break;

		/* we should return a physics address to application level */
		case IMAPX200_REG_BASE:
			__put_user(decode_param.reg_base_phys_addr, (unsigned int *)arg);
			break;

		/* this is 101 * 4 by default */
		case IMAPX200_REG_SIZE:
			__put_user(IMAPX200_DECODE_ACT_REG_SIZE, (unsigned int *)arg);
			break;

		case IMAPX200_PP_INSTANCE:
			file->private_data = &(decode_param.pp_instance);
			break;
#ifdef CONFIG_IMAP_DEC_HW_PERFORMANCE	/* this ioctl command call is for hardware decode performance detect */
		case IMAPX200_HW_PERFORMANCE:
			end_time_arg.tv_sec	= decode_param.end_time.tv_sec;
			end_time_arg.tv_usec	= decode_param.end_time.tv_usec;

			out = copy_to_user((struct timeval *)arg, &end_time_arg, sizeof(struct timeval));
			break;
#endif
		default:
			decode_error("IMAPX200 Decode unknown ioctl command\n");
			return -EFAULT;
	}

	return IMAPX200_DECODE_RET_OK;
}

static struct file_operations imapx200_decode_fops = 
{
	owner:		THIS_MODULE,
	open:		imapx200_decode_open,
	release:	imapx200_decode_release,
	ioctl:		imapx200_decode_ioctl,
#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
	fasync:		imapx200_decode_fasync,
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */
#ifdef CONFIG_IMAP_DECODE_POLL_MODE
	poll:		imapx200_decode_poll,
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */
};

/*
 * platform operation relate functions
 */
static int imapx200_decode_probe(struct platform_device *pdev)
{
	int		ret;
	unsigned int	size;
	struct resource	*res;

	/* get decode hardware clock and enable it */
	imap_vdec_clock = NULL;
	imap_vdec_clock = clk_get(&pdev->dev, "vdec");
	if(imap_vdec_clock == NULL)
	{
		decode_error("Fail to get decode hardware source\n");
		return IMAPX200_DECODE_RET_ERROR;
	}
	clk_enable(imap_vdec_clock);

	/* 
	 * 9170 decode ic is supposed that powered on when using, but uboot powers on 
	 * it for some usage, so i have to power down it here to ensure that when system
	 * boot, decode ic is not costing elec
	 */
	decode_disable_hw_power();

	/* initualize open count */
	decode_open_count = 0;

	/* initualize instances just for mark */
	decode_param.dec_instance	= 0;
	decode_param.pp_instance	= 0;

#ifdef CONFIG_IMAP_DECODE_POLL_MODE
	/* initualize wait queue */
	init_waitqueue_head(&wait_decode);
#endif	/* CONFIG_IMAP_DECODE_POLL_MODE */

#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
	/* initualize async queue */
	decode_param.async_queue_dec	= NULL;
	decode_param.async_queue_pp	= NULL;
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */

	/* get register base address */
	res = NULL;
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(res == NULL)
	{
		decode_error("Fail to get IMAPX200 Decode resource\n");
		return -ENOENT;
	}

	/* request memory region for registers */
	size = res->end - res->start + 1;
	decode_param.reg_reserved_size = size;
	decode_param.resource_mem = NULL;
	decode_param.resource_mem = request_mem_region(res->start, size, pdev->name);
	if(decode_param.resource_mem == NULL)
	{
		decode_error("Fail to get IMAPX200 Decode register memory region\n");
		return -ENOENT;
	}
	decode_param.reg_base_phys_addr = res->start;

	decode_param.reg_base_virt_addr = NULL;
	/*
	 * ATTENTON: memory map for registers should always be nocache method
	 * cuz we don't want register data to be cached 
	 */
	decode_param.reg_base_virt_addr = ioremap_nocache(res->start, size);
	if(decode_param.reg_base_virt_addr == NULL)
	{
		decode_error("Fail to ioremap IMAPX200 Decode register base address\n");
		return -EINVAL;
	}

	/* get and config irq */
	res = NULL;
	res = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
	if(res == NULL)
	{
		decode_error("Fail to get IMAPX200 Decode irq resource\n");
		return -ENOENT;
	}
	
	if(res->start != IRQ_VDEC)
	{
		decode_error("Get wrong irq number for IMAPX200 Decode\n");
		return -ENOENT;
	}

	/*
	 * decoder and pp shared one irq line, so we must register irq in share mode
	 */
	ret = -1;
	ret = request_irq(res->start, imapx200_decode_irq_handle, IRQF_DISABLED, \
			pdev->name, (void *)(&decode_param));
	if(ret)
	{
		decode_error("Fail to request irq for IMAPX200 Decode device\n");
		return ret;
	}

	/* register char device driver */
	ret = -1;
	ret = decode_driver_register();
	if(ret)
	{
		decode_error("Fail to register char device for IMAPX200 Decode\n");
		return ret;
	}

	/* reset hardware */
	reset_hw_reg_decode();

	mutex_init(&decode_lock);

	decode_debug("IMAPX200 Decode Driver probe OK\n");

	return IMAPX200_DECODE_RET_OK;
}

static int imapx200_decode_remove(struct platform_device *pdev)
{
	/* clear dec IRQ */
	writel(0, decode_param.reg_base_virt_addr + IMAPX200_DECODE_IRQ_STAT_DEC * 4);
	/* clear pp IRQ */
	writel(0, decode_param.reg_base_virt_addr + IMAPX200_DECODE_IRQ_STAT_PP * 4);

	/* unmap register base */
	iounmap((void *)(decode_param.reg_base_virt_addr));

	/* release registers memory region */
	release_mem_region(decode_param.reg_base_phys_addr, \
			decode_param.reg_reserved_size);

	/* release source */
	if(decode_param.resource_mem != NULL)
	{
		release_resource(decode_param.resource_mem);
		kfree(decode_param.resource_mem);
		decode_param.resource_mem = NULL;
	}

	/* release irq */
	free_irq(IRQ_VDEC, pdev);

	decode_driver_unregister();

	/* disable decode clock */
	clk_disable(imap_vdec_clock);

	mutex_destroy(&decode_lock);

	return IMAPX200_DECODE_RET_OK;
}

static int imapx200_decode_suspend(struct platform_device *pdev, pm_message_t state)
{
	decode_disable_hw_power();
#if !defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
	reset_mp_mark();
#else	/* CONFIG_IMAP_MMPOOL_MANAGEMENT */
	power_off_mpool_in_memalloc();
#endif	/* N CONFIG_IMAP_MMPOOL_MANAGEMENT */

	return IMAPX200_DECODE_RET_OK;
}

static int imapx200_decode_resume(struct platform_device *pdev)
{
	mutex_lock(&decode_lock);
#if defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
	power_on_mpool_in_memalloc();
#endif	/* CONFIG_IMAP_MMPOOL_MANAGEMENT */
	if(decode_open_count > 0)
		decode_enable_hw_power();
	mutex_unlock(&decode_lock);

	return IMAPX200_DECODE_RET_OK;
}

static struct platform_driver imapx200_decode_driver = 
{
	.probe		= imapx200_decode_probe,
	.remove		= imapx200_decode_remove,
	.suspend	= imapx200_decode_suspend,
	.resume		= imapx200_decode_resume,
	.driver		=
	{
		.owner		= THIS_MODULE,
		.name		= "imapx200_vdec",
	},
};

/*
 * init and exit
 */
static int __init imapx200_decode_init(void)
{
	/* call probe */
	if(platform_driver_register(&imapx200_decode_driver))
	{
		decode_error("Fail to register platform driver for IMAPX200 Decode Driver\n");
		decode_debug("IMAPX200 Decode Driver init failed\n");
		return -EPERM;
	}

	decode_debug("IMAPX200 Hantro 9170 Decode Driver init OK\n");

	return IMAPX200_DECODE_RET_OK;
}

static void __exit imapx200_decode_exit(void)
{
	/* call remove */
	platform_driver_unregister(&imapx200_decode_driver);

	decode_debug("IMAPX200 Decode Driver exit OK\n");
}

module_init(imapx200_decode_init);
module_exit(imapx200_decode_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Sololz of InfoTM");
MODULE_DESCRIPTION("IMAPX200 Decode Driver");

/*
 * just write 0 to all registers to reset harware
 * TODO: we have check whether it's needed
 */
int reset_hw_reg_decode(void)
{
	int i;

	writel(0, decode_param.reg_base_virt_addr + 0x04);

	for(i = 4; i < IMAPX200_DECODE_ACT_REG_SIZE; i += 4)
		writel(0, decode_param.reg_base_virt_addr + i);

	return IMAPX200_DECODE_RET_OK;
}

/*
 * this function do driver register to regist a node under /dev
 */
static struct class *decode_class;

int decode_driver_register(void)
{
	int ret;

	ret = -1;
	ret = register_chrdev(DECODE_DEFAULT_MAJOR, "imapx200-vdec", &imapx200_decode_fops);
	if(ret < 0)
	{
		decode_error("register char deivce error\n");
		return IMAPX200_DECODE_RET_ERROR;
	}

	decode_class = class_create(THIS_MODULE, "imapx200-vdec");
	device_create(decode_class, NULL, MKDEV(DECODE_DEFAULT_MAJOR, DECODE_DEFAULT_MINOR), NULL, "imapx200-vdec");

	return IMAPX200_DECODE_RET_OK;
}

int decode_driver_unregister(void)
{
	device_destroy(decode_class, MKDEV(DECODE_DEFAULT_MAJOR, DECODE_DEFAULT_MINOR));
	class_destroy(decode_class);
	unregister_chrdev(DECODE_DEFAULT_MAJOR, "imapx200-vdec");

	return IMAPX200_DECODE_RET_OK;
}

/*
 * clock related functions
 */
void decode_enable_hw_power(void)
{
	int i;
	unsigned int reg_val;

#if !defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
	if(!mp_mark)
	{
		/* set mempool power */
		reg_val = readl(rNPOW_CFG);
		reg_val |= (1 << 2);
		writel(reg_val, rNPOW_CFG);
		while(!(readl(rPOW_ST) & (1 << 2)));

		/* reset mempool power */
		reg_val = readl(rMD_RST);
		reg_val |= (1 << 2);
		writel(reg_val, rMD_RST);
		for(i = 0; i < 0x1000; i++);
		writel(readl(rMD_RST) & ~(1 << 2), rMD_RST);

		/* isolate mempool */
		reg_val = readl(rMD_ISO);
		reg_val &= ~(1 << 2);
		writel(reg_val, rMD_ISO);

		/* reset mempool */
		reg_val = readl(rAHBP_RST);
		reg_val |= (1 << 22);
		writel(reg_val, rAHBP_RST);
		for(i = 0; i < 0x1000; i++);		/* just for delay */
		writel(readl(rAHBP_RST) & ~(1 << 22), rAHBP_RST);

		/* enable mempool */
		reg_val = readl(rAHBP_EN);
		reg_val |= (1 << 22);
		writel(reg_val, rAHBP_EN);

		/* set memory pool mode */
		/* 
		 * This set memory pool to be decode mode, if you set this 
		 * register incorrectly, mosaic and green block will appear
		 * in some of your decoded pictures
		 */
		reg_val = readl(rMP_ACCESS_CFG);
		reg_val |= 0x01;
		writel(reg_val, rMP_ACCESS_CFG);

		mp_mark = 1;
	}
#endif	/* N CONFIG_IMAP_MMPOOL_MANAGEMENT */

	/* reset decode hardware */
	reg_val = readl(rAHBP_RST);
	reg_val |= (1 << 2);
	writel(reg_val, rAHBP_RST);
	for(i = 0;i < 0x1000; i++);
	writel(readl(rAHBP_RST) & ~(1 << 2), rAHBP_RST);

	/* enable decode */
	reg_val = readl(rAHBP_EN);
	reg_val |= (1 << 2);
	writel(reg_val, rAHBP_EN);

	/* set decode power */
	reg_val = readl(rNPOW_CFG);
	reg_val |= (1 << 1);
	writel(reg_val, rNPOW_CFG);
	while(!(readl(rPOW_ST) & (1 << 1)));

	/* reset decode power */
	reg_val = readl(rMD_RST);
	reg_val |= (1 << 1);
	writel(reg_val, rMD_RST);
	for(i = 0; i < 0x1000; i++);
	writel(readl(rMD_RST) & ~(1 << 1), rMD_RST);

	/* isolate decode */
	reg_val = readl(rMD_ISO);
	reg_val &= ~(1 << 1);
	writel(reg_val, rMD_ISO);

#if defined(CONFIG_IMAP_DECODE_IC_POWER_TRACE)
	printk(KERN_ALERT "[KERN DEC POWER TRACE] 9170 is powered on\n");
#endif	/* CONFIG_IMAP_DECODE_IC_POWER_TRACE */
}

void decode_disable_hw_power(void)
{
	unsigned int reg_val;

	/* unisolate decode */
	reg_val = readl(rMD_ISO);
	reg_val |= (1 << 1);
	writel(reg_val, rMD_ISO);

	/* shut decode power */
	reg_val = readl(rNPOW_CFG);
	reg_val &= ~(1 << 1);
	writel(reg_val, rNPOW_CFG);
	while((readl(rPOW_ST) & (1 << 1)));

	/* disable decode */
	reg_val = readl(rAHBP_EN);
	reg_val &= ~(1 << 2);
	writel(reg_val, rAHBP_EN);

#if defined(CONFIG_IMAP_DECODE_IC_POWER_TRACE)
	printk(KERN_ALERT "[KERN DEC POWER TRACE] 9170 is powered off\n");
#endif	/* CONFIG_IMAP_DECODE_IC_POWER_TRACE */
}

#if !defined(CONFIG_IMAP_MMPOOL_MANAGEMENT)	/* 2010/10/21 */
void reset_mp_mark(void)
{
	mp_mark = 0;
}
#else	/* CONFIG_IMAP_MMPOOL_MANAGEMENT */
uint32_t get_decode_running_status(void)
{
	uint32_t running = 0;

	mutex_lock(&decode_lock);
	running = (decode_open_count == 0? 0 : 1);
	mutex_unlock(&decode_lock);

	return running;
}
EXPORT_SYMBOL(get_decode_running_status);
#endif	/* N CONFIG_IMAP_MMPOOL_MANAGEMENT */
