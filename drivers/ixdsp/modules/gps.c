/***************************************************************************** 
** drivers/ixgps/modules/gps.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: GPS functions based on DSP.
**
** Author:
**     xecle  <xecle.zhang@infotmic.com.cn>
**     warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  08/12/2010
** 1.2  08/16/2010		add ioctl functions
*****************************************************************************/
#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/cpufreq.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/poll.h>
#include <linux/rtc.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>

#include <plat/idsp.h>
#include "ix_gps.h"

struct ix_gps_dev ix_gps;

static void inline __gps_dump(const uint8_t *dat, int len)
{
	char str[64];
	int i;

	str[0] = 0;
	for(i = 0; i < len / 4; i++)
	{
		if(!(i & 0x3))
		  gps_dbg("%s\n", str);
		sprintf(str + 9 * (i & 0x3), "%08x ", *(uint32_t *)(dat + i * 4));
	}
}

static void inline __gps_char(const uint8_t *dat, int len)
{
	char str[64] = {0};
	int i;

	str[0] = 0;
	for(i = 0; i < len; i++)
	{
		if(!(i & 0x3f))
		  gps_dbg("%s\n", str);
		sprintf(str + (i & 0x3f), "%c", dat[i]);
	}
}

static int __kread_file(const char *s, uint8_t *buf, uint32_t len)
{
	struct file *fp;
    mm_segment_t old_fs;
    loff_t pos = 0;
	size_t ret, readed = 0, read_len;

	gps_dbg("%s..step1\n", __func__);

    fp = filp_open(s, O_RDONLY, 0);
	if(IS_ERR(fp))
	{
		printk(KERN_ERR "%s: failed to open file %s\n",
		   __func__, s);
		return -EFAULT;
	}

	gps_dbg("%s..step2\n", __func__);

    old_fs = get_fs();
    set_fs(KERNEL_DS);
	gps_dbg("%s..step3\n", __func__);

	for(;;)
	{
		int i;
		read_len = ((len - readed > 0x400)?0x400:(len - readed));
		ret = vfs_read(fp, ix_gps.nmea_data, read_len, &pos);

		if(ret < read_len)
		{
			printk(KERN_WARNING "%s: file end before demonded bytes. %d(%d)\n",
			   __func__,ret,read_len);
			//break;
			read_len = ret;
		}

		for(i = 0; i < read_len; i += 4)
		  *(uint32_t *)(buf + readed + i) =
			  *(uint32_t *)(ix_gps.nmea_data + i);

		readed += read_len;
		if(readed == len || read_len < 0x400)
		  break;
	}

    set_fs(old_fs);

    filp_close(fp, 0);

	gps_dbg("%s..step4\n", __func__);
	return ret;
}


void ix_gps_enable(int en)
{
	if(en)
	{
		/* power on DSP module */
		writel(readl(rNPOW_CFG) | iGPS_POWER_BIT, rNPOW_CFG);

		while(!(readl(rPOW_ST) & iGPS_POWER_BIT));

		/* open CLK mask */
		writel(readl(rHCLK_MASK) & ~IMAP_CLKCON_HCLK_GPS, rHCLK_MASK);

		/* reset DSP */
		writel(readl(rMD_RST) | iGPS_POWER_BIT, rMD_RST);

		/* reset APB */
		writel(readl(rAHBP_RST) | IMAP_CLKCON_HCLK_GPS, rAHBP_RST);
		udelay(20);

		/* release DSP APB interface */
		writel(readl(rAHBP_RST) & ~IMAP_CLKCON_HCLK_GPS, rAHBP_RST);

		/* release isolation */
		writel(readl(rMD_ISO) & ~iGPS_POWER_BIT, rMD_ISO);

		/* release reset */
		writel(readl(rMD_RST) & ~iGPS_POWER_BIT, rMD_RST);

		/* release boe */
		writel(readl(rAHBP_EN) | IMAP_CLKCON_HCLK_GPS, rAHBP_EN);
	} else {
		/* power down DSP module */
		/* isolation */
		writel(readl(rMD_ISO) | iGPS_POWER_BIT, rMD_ISO);
		 
		/* reset DSP */
		writel(readl(rMD_RST) | iGPS_POWER_BIT, rMD_RST);

		/* mask DSP clock */
		writel(readl(rHCLK_MASK) | IMAP_CLKCON_HCLK_GPS, rHCLK_MASK);

		/* power off */
		writel(readl(rNPOW_CFG) & ~iGPS_POWER_BIT, rNPOW_CFG);
		
		/* wait until no power is left */
		while(readl(rPOW_ST) & iGPS_POWER_BIT);
	}
}

void ix_gps_hw_init(int en)
{
	uint32_t tmp;

	if(en)
	{
		/* power up GPS */
		ix_gps_enable(1);

		/* power up DSP */
		idsp_enable(1);

		/* init GPIOs
		 * configure GPSCLK GPSDAT0 nad GPSDAT1
		 */
#if 0
		tmp = readl(rGPECON);
		tmp &= 0xf03fffff;
		tmp |= 0x0a800000;
		writel(tmp, rGPECON);

		/* configure OPEN GPS RF module power
		 * wwe10 inch, SD2CMD is power enable
		 * config GPE[7] as output
		 */
		tmp = readl(rGPOCON);
		tmp &= 0xffff3fff;
		tmp |= 0x00004000;
		writel(tmp, rGPOCON);

		/* set GPE[7] to 1, open RF power */
		writel(readl(rGPEDAT) | (1 << 7), rGPEDAT);
#endif
	} else {

		/* shutdown GPS module */
		ix_gps_enable(0);
		idsp_enable(0);
#if 0		
		writel(readl(rGPEDAT) & ~(1 << 7), rGPEDAT);
#endif
	}
}

static int ix_gps_cmd(enum ix_gps_cmds cmd, uint32_t args)
{
	uint32_t data;

	switch(cmd)
	{
		case IX_GPS_CMD_SET_AID_ADDR:
			data = (0x1 << 24) | (uint32_t)args;
			break;
		case IX_GPS_CMD_SET_DATA:
			data = (0x2 << 24) | (uint32_t)args;
			break;
		case IX_GPS_CMD_SET_UTC:
			data = (0x7 << 24);
			break;
		case IX_GPS_CMD_GET_UTC:
			data = (0x3 << 24);
			break;
		case IX_GPS_CMD_NAVIGATION_STOP:
			data = (0x4 << 24);
			break;
	}

	/* send cmd */
	idsp_mail_send(ix_gps.sender, &data, 1);
	return 0;
}

int ix_gps_inject_time(void)
{
	unsigned long time;
	struct rtc_time tm;
	uint32_t gps_time[6];

	/* set time */
	time = get_seconds();
	rtc_time_to_tm(time, &tm);

	gps_time[0] = tm.tm_year - 100;
	gps_time[1] = tm.tm_mon + 1;
	gps_time[2] = tm.tm_mday;
	gps_time[3] = tm.tm_hour;
	gps_time[4] = tm.tm_min;
	gps_time[5] = tm.tm_sec;

	gps_dbg("Aiding time %d, %d, %d, %d, %d, %d\n",
	   gps_time[0], gps_time[1], gps_time[2], gps_time[3],
		gps_time[4], gps_time[5]);
	memcpy(ix_gps.auxdata + 4, (uint8_t *)gps_time, 24);
	
	/* aiding valid */
	*(uint32_t *)ix_gps.auxdata |= 0x1;

	return 0;
}

void ix_gps_ite(void)
{
//	gps_dbg("%s..i\n", __func__);
	uint32_t msg;

	if(idsp_mail_unread_msg(ix_gps.receiver))
	{
//		gps_dbg("%s..2\n", __func__);
		idsp_mail_receive(ix_gps.receiver, &msg, 1);

		gps_dbg("ite: cmd=0x%02x, len=0x%x\n", msg >> 24,
		   msg & 0xffff);

		if((msg >> 24) == 0x5)
		{
			int i;

			/* check gps state 
			 * renew nmea data if the device is empty
			 */
			if(ix_gps_get_stat(&ix_gps) == IX_GPS_NMEA_READING)
			  goto __exit_ite;
			/* set state to IX_GPS_NMEA_UPDATING to prevent
			 * user program from reading
			 */
			ix_gps_set_stat(&ix_gps, IX_GPS_NMEA_UPDATING);

//			gps_dbg("%s..3\n", __func__);
			ix_gps.nmea_len = msg & 0xffff;

			for(i = 0; i < ix_gps.nmea_len; i++)
			  ix_gps.nmea_data[i] = *(uint16_t *)(ix_gps.nmea_pos + i * 2) & 0xff;

			ix_gps.nmea_data[i] = 0;
#if 0
			   gps_dbg("gps nmea raw data:\n");
			__gps_dump(ix_gps.nmea_pos, ix_gps.nmea_len << 1);
			   gps_dbg("gps nmea data:\n");
			   printk(KERN_ERR "%s\n", ix_gps.nmea_data);
#endif

			ix_gps.nmea_readed = 0;
			ix_gps_set_stat(&ix_gps, IX_GPS_NMEA_READY);
			wake_up(&ix_gps.wq);
		}
	}

//	gps_dbg("%s..o\n", __func__);

__exit_ite:
	return ;
}

static int ix_gps_start(int xxioo)
{
	int err, i;
	void * tmp;

	if(xxioo == 2)
	{
		printk(KERN_INFO "Resume DSP binary from DRAM.\n");
		tmp = ix_gps.prog_suspend;
		for (i = 0; i < IX_DSP_COP_LEN; i += 4)
		{
			writel(*(uint32_t *)tmp, ix_gps.copvp + i);
			tmp += 4;
		}
	}
	else
	{
		if(!ix_gps.dsp_user)
		{
			err = __kread_file(IX_DSP_BIN_FILE, ix_gps.copvp, IX_DSP_COP_LEN);
			if(err < 0)
			  printk(KERN_WARNING "Open file %s failed.\n", IX_DSP_BIN_FILE);
			else
			  printk(KERN_INFO "Using defaut DSP binary file.\n");
		} else
		  printk(KERN_INFO "Using user defined DSP binary file.\n");

		//	__gps_dump(ix_gps.copvp, 128);

		if(ix_gps.aid_user)
		  printk(KERN_INFO "Using user injected aiding data.\n");
		else
		{
			err = __kread_file(IX_DSP_DATA_FILE, ix_gps.auxdata, IX_GPS_AUX_SIZE);
			if(err < 0)
			  printk(KERN_WARNING "Open file %s failed.\n", IX_DSP_DATA_FILE);
			else
			  printk(KERN_INFO "Using defaut aiding file.\n");
		}
	}

	/* XXX  removed XXX
	 * we don't manage aiding data in driver any more
	 */
#if 0
	__gps_dump(ix_gps.auxdata, 128);
	err = __kread_file(IX_DSP_DATA_FILE, ix_gps.auxdata, 0x900 << 1);
	if(err < 0)
	  printk(KERN_WARNING "Open file %s failed.\n", IX_DSP_DATA_FILE);
	__gps_dump(ix_gps.auxdata, 128);
#endif

	/* set aiding time */
	ix_gps_inject_time();

//	printk(KERN_ERR "Aiding data before start DSP!\n");
//	__gps_dump(ix_gps.auxdata, 128);
	/* now start GPS */
	idsp_boot_jump();

	gps_dbg("Nav OK\n");
	
	/* use edm as our defaut DSP memory */
    ix_gps_cmd(IX_GPS_CMD_SET_AID_ADDR,
	   (((ix_gps.auxdata_dma & 0x003fffff) | 0x00800000) >> 1));

	gps_dbg("gps start OK\n");
	
	/* set gps dev to fp */
	ix_gps.start = 1;

	return 0;
}

static int ix_gps_stop(void)
{
	return 0;
}

static int ix_gps_access(struct ix_gps_access_desc *desc)
{
	switch(desc->type)
	{
		case IX_GPS_INJECT_DSP:
		{
			uint32_t i, tmp;

			if((desc->len & 0x3) || (desc->start & 0x3))
			{
				printk(KERN_ERR "DSP binary must be 4 bytes aligned.\n");
				return -EFAULT;
			}

			if(desc->len + desc->start > IX_DSP_COP_LEN)
			{
				printk(KERN_ERR "DSP binary exceed size limit.\n");
				return -EFAULT;
			}

			for(i = 0; i < desc->len; i += 4)
			{
				if(copy_from_user(&tmp, desc->buf + i, 4))
				{
					printk("Get user data error.\n");
					ix_gps.dsp_user = 0;
					return -EFAULT;
				}
				writel(tmp, ix_gps.copvp + desc->start + i);
			}

//			__gps_dump(ix_gps.copvp + desc->start, 128);

			ix_gps.dsp_user = 1;
			break;
		}
		case IX_GPS_INJECT_AIDING:
		{
			if(desc->len + desc->start > IX_GPS_AUX_SIZE)
			{
				printk(KERN_ERR "Inject aiding exceed aux limit.\n");
				return -EFAULT;
			}

			if(copy_from_user(ix_gps.auxdata + desc->start,
			   desc->buf, desc->len))
			  printk(KERN_ERR "Inject aiding failed.\n");

//			__gps_dump(ix_gps.auxdata + desc->start, 128);
			ix_gps.aid_user = 1;
			break;
		}
		case IX_GPS_DUMP_AIDING:
		{
			if(desc->len + desc->start > IX_GPS_AUX_SIZE)
			{
				printk(KERN_ERR "Dump aiding exceed aux limit.\n");
				return -EFAULT;
			}

			if(copy_to_user(desc->buf, ix_gps.auxdata + desc->start, desc->len))
			  return -EFAULT;
			break;
		}
		default:
			printk(KERN_ERR "Invalid descripor passed to iGPS driver.\n");
			return -EINVAL;
	}

	return 0;
}

static int ix_gps_ioctl(struct inode *inode,
   struct file *file, unsigned int cmd, unsigned long arg)                       
{
	struct ix_gps_access_desc desc;

	if(_IOC_TYPE(cmd) != IX_GPS_IOC_MAGIC
	   || _IOC_NR(cmd) > IX_GPS_IOC_MAX)
	  return -ENOTTY;

	switch(cmd)
	{
		case IX_GPS_START:
			return ix_gps_start(0);
		case IX_GPS_STOP:
			return ix_gps_stop();
		case IX_GPS_ACCESS_DATA:
			if(copy_from_user(&desc, (struct ix_gps_access_desc *)arg,
			   sizeof(struct ix_gps_access_desc)))
			{
				printk(KERN_ERR "Get access descriptor failed.\n");
				return -EFAULT;
			}
			return ix_gps_access(&desc);
	}

	return 0;
}

static int ix_gps_read(struct file *filp,
   char __user *buf, size_t length, loff_t *offset)
{
	int ret, len;

	if(ix_gps_get_stat(&ix_gps) == IX_GPS_NMEA_READY)
	  ix_gps_set_stat(&ix_gps, IX_GPS_NMEA_READING);

	if((ix_gps_get_stat(&ix_gps) == IX_GPS_NMEA_READING)
	   && (ix_gps.nmea_len == ix_gps.nmea_readed))
	{
		/* we must tell the upper layer that nmea data has reach the end */
		ix_gps_set_stat(&ix_gps, IX_GPS_EMPTY);
		return -EAGAIN;
	}

	len = ix_gps.nmea_len - ix_gps.nmea_readed;
	len = (len > length)?length:len;
	len = (len > 0)?len:0;

	if(len)
	{
		ret = copy_to_user(buf,
		   ix_gps.nmea_data + ix_gps.nmea_readed, len);

		if(ret)
		{
			printk(KERN_ERR "%s: copy to user failed !\n", __func__);
			/* assume the device to be empty before returning */
			ix_gps_set_stat(&ix_gps, IX_GPS_EMPTY);
			return -EFAULT;
		}
	}

	ix_gps.nmea_readed += len;

	return len;
}

static int ix_gps_write(struct file *filp,
   const char __user *buf, size_t length, loff_t * offset)
{
	return 0;
}

static unsigned int ix_gps_poll(struct file *filp, poll_table *wait)
{
	static unsigned long long pt;
	unsigned int mask = 0;

	if((pt++ & 0xfffff) == 0)
	  gps_dbg("polled %llu times\n", pt);

	poll_wait(filp, &ix_gps.wq, wait);

	if(ix_gps_get_stat(&ix_gps) == IX_GPS_NMEA_READY)
	{
		mask |= POLLIN;
//		gps_dbg("GPS state is %d\n", ix_gps.state);
	}

	return mask;
}

static int ix_gps_open ( struct inode *inode, struct file *filp)
{
	struct idsp_desc dsp = {
		.endian0 = 0,
		.endian1 = 0x10100,
		.epm_base = ix_gps.auxdata_dma & 0xff800000,
		.edm_base = (ix_gps.auxdata_dma & 0xffc00000), /* FIXME */
		.csr_base = PERIPHERAL_BASE_ADDR_PA,
		.b_addr		= 0x100100,
	};
	struct idsp_mail_desc mb[] = {
		{
			.act = IDSP_MAIL_SENDER,
		}, {
			.act = IDSP_MAIL_RECEIVER,
		}
	};

	if(ix_gps.start)
	{
		printk(KERN_ERR "%s: GPS node is already opened.\n",
		   __func__);
		return -EINVAL;
	}

	/* init GPS */
	ix_gps_hw_init(1);

	/* init iDSP */
	idsp_sw_init(&dsp);

	/* init mailbox */
	idsp_mail_init(mb);

	gps_dbg("iDSP init OK\n");

	/* register intr */
	idsp_set_ite(ix_gps_ite);
			
	gps_dbg("register ITE OK\n");

    if(filp)
        filp->private_data = &ix_gps;

	ix_gps.dsp_user = 0;
	ix_gps.aid_user = 0;
	return 0;
}

static int ix_gps_close (struct inode *inode, struct file *filp)
{
	ix_gps.start = 0;

	/* close GPS */
	ix_gps_hw_init(0);

	/* unset ite */
	idsp_set_ite(NULL);

	return 0;
}

static const struct file_operations ix_gps_fops = {
	.read = ix_gps_read,
	.write = ix_gps_write,
	.open = ix_gps_open,
	.release = ix_gps_close,
	.poll = ix_gps_poll,
	.ioctl = ix_gps_ioctl,
};

static int ix_gps_probe(struct platform_device *pdev)
{
	int err = 0, ret = 0;

	gps_dbg("%s\n", __func__);

	/* init gps_dev struct */
	memset(&ix_gps, 0, sizeof(struct ix_gps_dev));
	ix_gps.sender  = 0;
	ix_gps.receiver = 1;

	/* allocate auxdata buffer
	 * alloc 8k buffer here, the first 4K is used to 
	 * save nmea data, the last 4K is used to interact with
	 * iDSP.
	 */
#if 1
	ix_gps.auxdata = dmam_alloc_coherent(&pdev->dev, 0x42000,
	   &ix_gps.auxdata_dma, GFP_KERNEL | GFP_DMA );
#else
	ix_gps.auxdata_dma = 0x70a60000;
	ix_gps.auxdata = ioremap(ix_gps.auxdata_dma, IX_GPS_AUX_SIZE);
#endif

	memset(ix_gps.auxdata, 0, IX_GPS_AUX_SIZE);
	gps_dbg("coherent buffer is %08x.\n", ix_gps.auxdata_dma);

	if(!ix_gps.auxdata)
	{
		dev_err(&pdev->dev, "Can not allocate dmabuffer for auxdata.\n");
		return -ENOMEM;
	}
	ix_gps.nmea_pos = ix_gps.auxdata + (0x900 << 1);
	ix_gps.prog_suspend = ix_gps.auxdata + 0x22000;

	gps_dbg("DMA buffer alloc OK\n");

	/* allocate nmea buffer */
	ix_gps.nmea_data = kzalloc(0x400, GFP_KERNEL);
	if(!ix_gps.nmea_data)
	{
		dev_err(&pdev->dev, "Can not allocate buffer for nmea_data.\n");
		err = -ENOMEM;
		goto __err_exit_1;
	}

	gps_dbg("nmea_data buffer alloc OK\n");

	/* create gps dev node */
	ret = register_chrdev(IX_GPS_MAJOR, IX_GPS_NAME, &ix_gps_fops);

	if(ret < 0)
	{
		dev_err(&pdev->dev, "Register char device for gps failed.\n");
		err = -EFAULT;
		goto __err_exit_1;
	}

	ix_gps.class = class_create(THIS_MODULE, IX_GPS_NAME);
	if(!ix_gps.class)
	{
		dev_err(&pdev->dev, "Can not register class for GPS .\n");
		err = -EFAULT;
		goto __err_exit_1;
	}

	/* create device node */
	device_create(ix_gps.class, NULL,
	   MKDEV(IX_GPS_MAJOR, IX_GPS_MINOR), NULL, IX_GPS_NAME);

	/* init wait queue head */
	init_waitqueue_head(&ix_gps.wq);

	/* init spinlock */
	spin_lock_init(&ix_gps.nmea_lock);

	ix_gps.copvp = idsp_get_tcm_base();
	gps_dbg("Got copvp %p\n", ix_gps.copvp);
	/* XXX
	 * NOTE: we will init hardware and do irq register in start/stop function
	 */

	gps_dbg("probe ok.\n");
	return 0;

__err_exit_1:
	dmam_free_coherent(&pdev->dev, 0x22000, ix_gps.auxdata, ix_gps.auxdata_dma);
	return err;
}

static int ix_gps_remove(struct platform_device *pdev)
{
	dmam_free_coherent(&pdev->dev, 0x22000, ix_gps.auxdata, ix_gps.auxdata_dma);
    iounmap(ix_gps.copvp);

	return 0;
}

#ifdef CONFIG_PM
static int ix_gps_suspend(struct platform_device *dev, pm_message_t pm)
{
	int i;
	void * tmp;

	tmp = ix_gps.prog_suspend;
	for (i = 0; i < IX_DSP_COP_LEN; i += 4)
	{
		*(uint32_t *)tmp = readl(ix_gps.copvp + i);
		tmp += 4;
	}
    ix_gps_stop();
    ix_gps_close(NULL, NULL);

    return 0;
}

static int ix_gps_resume(struct platform_device *dev)
{
    ix_gps_open(NULL, NULL);
    ix_gps_start(2);
    
    return 0;
}
#else
#define ix_gps_suspend NULL
#define ix_gps_resume NULL
#endif

static struct platform_driver ix210_gps_driver = {
	.probe		= ix_gps_probe,
	.remove		= ix_gps_remove,
#ifdef CONFIG_PM
	.suspend	= ix_gps_suspend,
	.resume		= ix_gps_resume,
#endif
	.driver		= {
		.name		= "imap_gps",
		.owner		= THIS_MODULE,
	},
};

static int __init ix_gps_init(void)
{
	printk(KERN_INFO "IX210 iGPS driver (c) 2009, 2014 InfoTM\n");
	return platform_driver_register(&ix210_gps_driver);
}

static void __exit ix_gps_exit(void)
{
	platform_driver_unregister(&ix210_gps_driver);
}

module_init(ix_gps_init);
module_exit(ix_gps_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("xecle <xecle.zhang@infotmic.com.cn, warits <warits.wang@infotmic.com.cn>");
MODULE_DESCRIPTION("IX210 GPS Driver");

