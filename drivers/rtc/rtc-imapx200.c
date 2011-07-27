/***************************************************************************** 
** drivers/rtc/rtc-imapx200.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: RTC driver for iMAPx200
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 09/18/2009 XXX	Warits
*****************************************************************************/


#include <linux/module.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/rtc.h>
#include <linux/bcd.h>
#include <linux/clk.h>
#include <linux/log2.h>
#include <linux/delay.h>

#include <mach/hardware.h>
#include <asm/uaccess.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/mach/time.h>

/* I have yet to find an iMAP implementation with more than one
 * of these rtc blocks in */

#define IMAP_RTC_ACCESS_BCD		0x01
#define IMAP_RTC_ACCESS_ALM		0x02
#define IMAP_RTC_ACCESS_READ	0x10
#define IMAP_RTC_ACCESS_WRITE	0x20

static struct resource *imap_rtc_mem;

static void __iomem *imap_rtc_base;
static int imap_rtc_alarmno		= NO_IRQ;
static int imap_rtc_tickno		= NO_IRQ;
static struct rtc_device *imap_rtc;

static DEFINE_SPINLOCK(imap_rtc_pie_lock);

/* RTC common Functions for iMAP API */

/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_pie(void __iomem *base, uint on)
 *
 * -Description:
 *	  This function will enable periodical interrupt bit.
 *
 * -Input Param
 *    *base     The base address of RTC regs
 *    *to		(Turn On), wether turn on periodical interrupt
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    None
 *
 * -Others
 *    None
 ***********************************************************************
 */
void __imap_rtc_pie(void __iomem *base, uint on)
{
	uint tmp;

	tmp = readb(base + IMAPX200_TICNT) & ~IMAPX200_TICNT_TICEN;

	if (on)
	  tmp |= IMAPX200_TICNT_TICEN;
	writeb(tmp, base + IMAPX200_TICNT);
}

/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_aie(uint on)
 *
 * -Description:
 *    Turn on/off alarm enable bit.
 *
 * -Input Param
 *    on		Wether enable alarm
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    None
 *
 * -Others
 *    None
 ***********************************************************************
 */
static void __imap_rtc_aie(uint on)
{
	uint tmp;

	pr_debug("%s: aie=%d\n", __func__, on);

	tmp = readb(imap_rtc_base + IMAPX200_RTCALM) & ~IMAPX200_RTCALM_ALMEN;

	if (on)
	{
		tmp |= IMAPX200_RTCALM_ALMEN;
		tmp |= 0x3f;	/* Enable all alarm bits */
	}
	
	writeb(tmp, imap_rtc_base + IMAPX200_RTCALM);
}

/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_freq(void __iomem *base, uint freq)
 *
 * -Description:
 *	  Set the value of reg IMAPX200_TICNT according to freq.
 *    only frequence between 1 and 64Hz can be applied.
 *    the following equation is used:
 *         freq = 128 / ((IMAPX200_TICNT & 0x7f) + 1)
 *
 * -Input Param
 *    *base     The base address of RTC regs
 *    freq		The required frequence
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    None
 *
 * -Others
 *    None
 ***********************************************************************
 */
void __imap_rtc_freq(void __iomem *base, uint freq)
{
	unsigned int tmp;

	tmp = readb(base + IMAPX200_TICNT) & IMAPX200_TICNT_TICEN;	
	tmp |= (128 / freq) - 1;
	writeb(tmp, base + IMAPX200_TICNT);
}

/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_enable(struct platform_device *pdev, void __iomem *base, int en)
 *
 * -Description:
 *    RTC enable/disable function, according to the value of en
 *
 * -Input Param
 *    *pdev     Pointer to platform device
 *    *base     The base address of RTC regs
 *    en		Wether enable RTC
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    None
 *
 * -Others
 *    None
 ***********************************************************************
 */
void __imap_rtc_enable(struct platform_device *pdev, void __iomem *base, int en)
{
	unsigned int tmp;

	if(!en) {
		tmp = readb(base + IMAPX200_RTCCON);
		writeb(tmp & ~IMAPX200_RTCCON_RTCEN, base + IMAPX200_RTCCON);
		writeb(tmp & ~IMAPX200_TICNT_TICEN,  base + IMAPX200_TICNT);
	} else {
		tmp = 0;
		tmp |= IMAPX200_RTCCON_RTCEN;
		tmp &= ~IMAPX200_RTCCON_CLKSEL;
		tmp &= ~IMAPX200_RTCCON_Reserved1;
		tmp &= ~IMAPX200_RTCCON_Reserved2;

		writeb(tmp, base + IMAPX200_RTCCON);

		/*
		 * Wait for at least 61us before doing any change to RTC registers
		 * after enabling RTC
		 */
		dev_dbg(&pdev->dev, "Enabling RTC device, RTCCON is %08x\n", readb(base + IMAPX200_RTCCON));
		mdelay(1);
	}
}

/*!
 ***********************************************************************
 * -Function:
 *    __imap_rtc_access(struct rtc_time *tm, uint acc)
 *
 * -Description:
 *	  Access RTC registers to set or read time/alarm.
 *
 * -Input Param
 *	  tm, the structure to store time values.
 *	  acc,	access descriptor, 0x00 write time
 *							   0x01 write alarm
 *							   0x10 read time
 *							   0x11 read alarm
 *
 * -Output Param
 *	  tm, the structure to store time values.
 *                
 * -Return
 *    None
 *
 * -Others
 *    None
 ***********************************************************************
 */
void __imap_rtc_access(struct rtc_time *tm, uint acc)
{
	void __iomem *base = imap_rtc_base;

	if (likely(acc & IMAP_RTC_ACCESS_READ))
	{
		if (likely(acc & IMAP_RTC_ACCESS_BCD))
		{
			tm->tm_min  = bcd2bin(readb(base + IMAPX200_BCDMIN));
			tm->tm_hour = bcd2bin(readb(base + IMAPX200_BCDHOUR));
			tm->tm_mday = bcd2bin(readb(base + IMAPX200_BCDDATE));
			tm->tm_wday = bcd2bin(readb(base + IMAPX200_BCDDAY));
			tm->tm_mon  = bcd2bin(readb(base + IMAPX200_BCDMON));
			tm->tm_year = bcd2bin(readb(base + IMAPX200_BCDYEAR));
			tm->tm_sec  = bcd2bin(readb(base + IMAPX200_BCDSEC));

		} else {
			tm->tm_min  = bcd2bin(readb(base + IMAPX200_ALMMIN));
			tm->tm_hour = bcd2bin(readb(base + IMAPX200_ALMHOUR));
			tm->tm_mday = bcd2bin(readb(base + IMAPX200_ALMDATE));
			tm->tm_mon  = bcd2bin(readb(base + IMAPX200_ALMMON));
			tm->tm_year = bcd2bin(readb(base + IMAPX200_ALMYEAR));
			tm->tm_sec  = bcd2bin(readb(base + IMAPX200_ALMSEC));
		}
	} else { /* Write process */
		if (likely(acc & IMAP_RTC_ACCESS_BCD))
		{
			writeb(bin2bcd(tm->tm_sec), base + IMAPX200_BCDSEC);
			writeb(bin2bcd(tm->tm_min), base + IMAPX200_BCDMIN);
			writeb(bin2bcd(tm->tm_hour), base + IMAPX200_BCDHOUR);
			writeb(bin2bcd(tm->tm_mday), base + IMAPX200_BCDDATE);
			writeb(bin2bcd(tm->tm_wday), base + IMAPX200_BCDDAY);
			writeb(bin2bcd(tm->tm_mon), base + IMAPX200_BCDMON);
			writeb(bin2bcd(tm->tm_year), base + IMAPX200_BCDYEAR);
		} else {
			writeb(bin2bcd(tm->tm_sec), base + IMAPX200_ALMSEC);
			writeb(bin2bcd(tm->tm_min), base + IMAPX200_ALMMIN);
			writeb(bin2bcd(tm->tm_hour), base + IMAPX200_ALMHOUR);
			writeb(bin2bcd(tm->tm_mday), base + IMAPX200_ALMDATE);
			writeb(bin2bcd(tm->tm_mon), base + IMAPX200_ALMMON);
			writeb(bin2bcd(tm->tm_year), base + IMAPX200_ALMYEAR);
		}
	}
	return ;
}


/* IRQ Handlers */
/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_alarmirq(int irq, void *id)
 *
 * -Description:
 *    Interrupt handler when RTCALARM INT is generated.
 *
 * -Input Param
 *    irq		The irq number
 *    id		The private device pointer
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    IRQ_HANDLED
 *
 * -Others
 *    None
 ***********************************************************************
 */
static irqreturn_t imap_rtc_alarmirq(int irq, void *id)
{
	struct rtc_device *rdev = id;
	rtc_update_irq(rdev, 1, RTC_AF | RTC_IRQF);

	return IRQ_HANDLED;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_tickirq(int irq, void *id)
 *
 * -Description:
 *    Interrupt handler when RTCTICK INT is generated.
 *
 * -Input Param
 *    irq		The irq number
 *    id		The private device pointer
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    IRQ_HANDLED
 *
 * -Others
 *    None
 ***********************************************************************
 */
static irqreturn_t imap_rtc_tickirq(int irq, void * id)
{
	struct rtc_device *rdev = id;
	rtc_update_irq(rdev, 1, RTC_PF | RTC_IRQF);

	return IRQ_HANDLED;
}

/* Update control registers */
/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_pie(int to)
 *
 * -Description:
 *    Turn on/off periodical interrupt enable bit.
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *    enabled	Wether enable periodical interrupt
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *    0 on success
 *
 * -Others
 *    None
 ***********************************************************************
 */
static int imap_rtc_pie(struct device *dev, int enabled)
{
	pr_debug("%s: pie=%d\n", __func__, enabled);

	spin_lock_irq(&imap_rtc_pie_lock);

	__imap_rtc_pie(imap_rtc_base, enabled);

	spin_unlock_irq(&imap_rtc_pie_lock);
	
	return 0;
}

static int imap_rtc_aie(struct device *dev, unsigned int enabled)
{
	__imap_rtc_aie(enabled);
	return 0;
}

static void imap_rtc_enable(struct platform_device *pdev, int en)
{
	void __iomem *base = imap_rtc_base;

	if (imap_rtc_base == NULL)
	  return;
	__imap_rtc_enable(pdev, base, en);
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_setfreq(struct device *dev, int freq)
 *
 * -Description:
 *	  Set the value of reg IMAPX200_TICNT according to freq.
 *    only frequence between 1 and 64Hz can be applied.
 *    the following equation is used:
 *         freq = 128 / ((IMAPX200_TICNT & 0x7f) + 1)
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *    freq		The required frequence
 *
 * -Output Param
 *	  imap_freq	output the freq setted.
 *                
 * -Return
 *    0 on success
 *
 * -Others
 *	  This is the secure inferface to users, compared to the above one.
 ***********************************************************************
 */
static int imap_rtc_setfreq(struct device *dev, int freq)
{
	/* iMAPx200 can only generate Hz from 1 to 64
	 * any thing out of this band is not acceptted
	 */
	if (freq < 1 || freq > 64) return -EINVAL;

	spin_lock_irq(&imap_rtc_pie_lock);
	__imap_rtc_freq(imap_rtc_base, freq);
	spin_unlock_irq(&imap_rtc_pie_lock);
	return 0;
}

/* Time read/write */
/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
 *
 * -Description:
 *	  Read the current time from RTC registers.
 *	  The date and time value is store in BCD format in BCDSEC~BCDYEAR
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *
 * -Output Param
 *	  rtc_tm	This is a pointer to a time values container structure,
 *				which carries the read values.
 *                
 * -Return
 *	  0 on success
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_gettime(struct device *dev, struct rtc_time *rtc_tm)
{
	__imap_rtc_access(rtc_tm, IMAP_RTC_ACCESS_READ | IMAP_RTC_ACCESS_BCD);

	/* the only way to work out wether the system war mid-update
	 * when we read it is to check the second counter, and if it
	 * is zero, then we re-try the entire read
	 */
	if(!rtc_tm->tm_sec)
	  __imap_rtc_access(rtc_tm, IMAP_RTC_ACCESS_READ | IMAP_RTC_ACCESS_BCD);

	dev_dbg(dev, "read time %02x.%02x.%02x %02x/%02x/%02x\n",
	   rtc_tm->tm_year, rtc_tm->tm_mon, rtc_tm->tm_mday,
	   rtc_tm->tm_hour, rtc_tm->tm_min, rtc_tm->tm_sec);

	rtc_tm->tm_year += 100;
	rtc_tm->tm_mon  -= 1;

	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_settime(struct device *dev, struct rtc_time *tm)
 *
 * -Description:
 *	  Store the values in tm structure into RTC registers.
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *	  rtc_tm	This is a pointer to a time values container structure,
 *				values in this structure will be wrote into RTC regs.
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0		    on success
 *	  -EINVAL	on failure
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_settime(struct device *dev, struct rtc_time *tm)
{
	struct rtc_time ttm;
	struct platform_device *pdev = to_platform_device(dev);

	memcpy(&ttm, tm, sizeof(struct rtc_time));
	ttm.tm_year -= 100;
	ttm.tm_mon  += 1;

	/* kick y2k out */
	if (ttm.tm_year <= 0 || ttm.tm_year > 100) {
		printk(KERN_DEBUG "RTC supports from 2001 to 2100.\n");
		return -EINVAL;
	}

	dev_dbg(dev, "set time %02d.%02d.%02d %02d/%02d/%02d\n",
	   ttm.tm_year, ttm.tm_mon, ttm.tm_mday,
	   ttm.tm_hour, ttm.tm_min, ttm.tm_sec);

	/* RDWR RTC */
	imap_rtc_enable(pdev, 1);
	__imap_rtc_access(&ttm, IMAP_RTC_ACCESS_WRITE | IMAP_RTC_ACCESS_BCD);

	/* Wait until time is set */
	while(readb(imap_rtc_base + IMAPX200_RTCSET) & IMAPX200_RTCSET_BCD);

	/* Read Only */
	imap_rtc_enable(pdev, 0);

	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm)
 *
 * -Description:
 *	  Read the current alarm from RTC registers.
 *	  The date and time value is store in BCD format in ALMSEC~ALMYEAR
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *
 * -Output Param
 *	  alrm		Alarm structure, containning:
 *					enabled;	0 = alarm disabled, 1 = alarm enabled 
 *					pending;	0 = alarm not pending, 1 = alarm pending 
 *					time;		time the alarm is set to 
 *				The read values is stored in alrm.time, if the relative
 *				alarm bit is not enabled, 0xff will be read.
 *                
 * -Return
 *	  0 on success
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_getalarm(struct device *dev, struct rtc_wkalrm *alrm){
	struct rtc_time *alm_tm = &alrm->time;
	void __iomem *base = imap_rtc_base;
	uint alm_en;

	__imap_rtc_access(alm_tm, IMAP_RTC_ACCESS_READ | IMAP_RTC_ACCESS_ALM);
	alm_tm->tm_mon	-= 1;
	alm_tm->tm_year	+= 100;

	alm_en = readb(base + IMAPX200_RTCALM);
	alrm->enabled = (alm_en & IMAPX200_RTCALM_ALMEN) ? 1: 0;

	pr_debug("read alarm %02x %02x.%02x.%02x %02x/%02x/%02x\n",
	   alm_en,
	   alm_tm->tm_year, alm_tm->tm_mon, alm_tm->tm_mday,
	   alm_tm->tm_hour, alm_tm->tm_min, alm_tm->tm_sec);

	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
 *
 * -Description:
 *	  Store the values in tm structure into RTC registers.
 *
 * -Input Param
 *    dev		Standard function interface required, but not used here
 *	  alrm		values in alrm.time will be stored into RTC ALM regs,
 *				and alarm will be enabled.
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0		    on success
 *	  -EINVAL	on failure
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_setalarm(struct device *dev, struct rtc_wkalrm *alrm)
{
	struct rtc_time *tm = &alrm->time, ttm;
	struct platform_device *pdev = to_platform_device(dev);

	memcpy(&ttm, tm, sizeof(struct rtc_time));
	ttm.tm_year	-= 100;
	ttm.tm_mon	+= 1;

	/* kick y2k out*/
	if (ttm.tm_year <= 0 || ttm.tm_year > 100) {
		printk(KERN_DEBUG "RTC supports from 2001 to 2100.\n");
		return -EINVAL;
	}

	pr_debug("imap_rtc_setalarm: %d, %02x/%02x/%02x %02x.%02x.%02x\n",
	   alrm->enabled,
	   tm->tm_mday & 0xff, tm->tm_mon & 0xff, tm->tm_year & 0xff,
	   tm->tm_hour & 0xff, tm->tm_min & 0xff, tm->tm_sec & 0xff);

	/* RDWR RTC */
	imap_rtc_enable(pdev, 1);
	/* the date and time put forward by RTC subsystem is always valid
	 * so just accommodate them into there registers
	 */
	__imap_rtc_access(&ttm, IMAP_RTC_ACCESS_WRITE | IMAP_RTC_ACCESS_ALM);

	/* Wait until alarm is set */
	while(readb(imap_rtc_base + IMAPX200_RTCSET) & IMAPX200_RTCSET_ALM);

	__imap_rtc_aie(alrm->enabled);

	/* Read Only */
	imap_rtc_enable(pdev, 0);
	return 0;
}

static int imap_rtc_proc(struct device *dev, struct seq_file *seq)
{
	unsigned int ticnt = readb(imap_rtc_base + IMAPX200_TICNT);

	seq_printf(seq, "periodic_IRQ\t: %s\n",
	   (ticnt & IMAPX200_TICNT_TICEN) ? "yes" : "no");
	return 0;
}

static int imap_rtc_open(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rtc_device *rtc_dev = platform_get_drvdata(pdev);
	int ret;

	ret = request_irq(imap_rtc_alarmno, imap_rtc_alarmirq,
	   IRQF_DISABLED, "imap-rtc alarm", rtc_dev);

	if (ret) {
		dev_err(dev, "IRQ%d error %d\n", imap_rtc_alarmno, ret);
		return ret;
	}

	ret = request_irq(imap_rtc_tickno, imap_rtc_tickirq,
	   IRQF_DISABLED, "imap-rtc tick", rtc_dev);

	if (ret) {
		dev_err(dev, "IRQ%d error %d\n", imap_rtc_tickno, ret);
		goto tick_err;
	}

	return ret;

tick_err:
	free_irq(imap_rtc_alarmno, rtc_dev);
	return ret;
}

static void imap_rtc_release(struct device *dev)
{
	struct platform_device *pdev = to_platform_device(dev);
	struct rtc_device *rtc_dev = platform_get_drvdata(pdev);

	/* do not clear AIE here, it may be needed for wake */

	imap_rtc_pie(dev, 0);
	free_irq(imap_rtc_alarmno, rtc_dev);
	free_irq(imap_rtc_tickno, rtc_dev);
}

static const struct rtc_class_ops imap_rtcops = {
	.open				=	imap_rtc_open,
	.release			=	imap_rtc_release,
	.ioctl				=	NULL,
	.read_time			=	imap_rtc_gettime,
	.set_time			=	imap_rtc_settime,
	.read_alarm			=	imap_rtc_getalarm,
	.set_alarm			=	imap_rtc_setalarm,
	.proc				=	imap_rtc_proc,
	.irq_set_state		=	imap_rtc_pie,
	.irq_set_freq		=	imap_rtc_setfreq,
	.alarm_irq_enable	=	imap_rtc_aie,
};

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_probe(struct platform_device *pdev)
 *
 * -Description:
 *	  Platform device probe.
 *
 * -Input Param
 *	  pdev		platform device pointer provided by system
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0			On success
 *	  -ENOENT	If resource is not availiable
 *	  -EINVAL	If resource is invalid
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_probe(struct platform_device *pdev)
{
	struct resource *res;
	int ret;
	unsigned char bcd_tmp, bcd_loop;
	dev_dbg(&pdev->dev, "%s: probe=%p\n", __func__, pdev);

	/* find the IRQs */

	imap_rtc_tickno = platform_get_irq(pdev, 1);
	if (imap_rtc_tickno < 0) {
		dev_err(&pdev->dev, "no irq for rtc tick\n");
		return -ENOENT;
	}

	imap_rtc_alarmno = platform_get_irq(pdev, 0);
	if (imap_rtc_alarmno < 0) {
		dev_err(&pdev->dev, "no irq for alarm\n");
		return -ENOENT;
	}

	dev_dbg(&pdev->dev, "Tick IRQ %d, alarm IRQ %d\n",
	   imap_rtc_tickno, imap_rtc_alarmno);

	/* get the memory region */

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (res == NULL) {
		dev_err(&pdev->dev, "failed to get memory region resource\n");
		return -ENOENT;
	}

	imap_rtc_mem = request_mem_region(res->start,
	   res->end - res->start + 1, pdev->name);
	
	if (imap_rtc_mem == NULL) {
		dev_err(&pdev->dev, "failed to reserve memory region\n");
		ret = -ENOENT;
		goto err_nores;
	}

	imap_rtc_base = ioremap(res->start, res->end - res->start + 1);
	if (imap_rtc_base == NULL) {
		dev_err(&pdev->dev, "failed ioremap()\n");
		ret = -EINVAL;
		goto err_nomap;
	}

	/* check to see if everything is setup correctly */

	imap_rtc_enable(pdev, 1);

	pr_debug("imap_rtc: RTCCON =%02x\n", 
	   readb(imap_rtc_base + IMAPX200_RTCCON));

	imap_rtc_setfreq(&pdev->dev, 1);

	/* register RTC and exit */

	imap_rtc = rtc_device_register("imap", &pdev->dev, &imap_rtcops, THIS_MODULE);

	if (IS_ERR(imap_rtc)) {
		dev_err(&pdev->dev, "cannot attach rtc\n");
		ret = PTR_ERR(imap_rtc);
		goto err_nortc;
	}

	imap_rtc->max_user_freq = IMAPX200_MAX_CNT;
	imap_rtc->irq_freq = 1;


	/* check rtc time */
	for (bcd_loop = IMAPX200_BCDSEC; bcd_loop <= IMAPX200_BCDYEAR; bcd_loop += 0x04)
	{
		bcd_tmp = readb(imap_rtc_base + bcd_loop);
		if (((bcd_tmp & 0xf) > 0x9) || ((bcd_tmp & 0xf0) > 0x90))
		  writeb(0, imap_rtc_base + bcd_loop);
	}

	/* Read only after probe */
	imap_rtc_enable(pdev, 0);

	platform_set_drvdata(pdev, imap_rtc);
	return 0;

err_nortc:
	imap_rtc_enable(pdev, 0);
	iounmap(imap_rtc_base);

err_nomap:
	release_resource(imap_rtc_mem);

err_nores:
	return ret;
}

static int imap_rtc_remove(struct platform_device *pdev)
{
	struct rtc_device *rtc = platform_get_drvdata(pdev);

	platform_set_drvdata(pdev, NULL);
	rtc_device_unregister(rtc);

	imap_rtc_pie(&pdev->dev, 0);
	imap_rtc_aie(&pdev->dev, 0);
	imap_rtc_enable(pdev, 0);

	iounmap(imap_rtc_base);
	release_resource(imap_rtc_mem);
	kfree(imap_rtc_mem);

	return 0;
}

#ifdef CONFIG_PM
/* RTC Power management control */

static struct timespec imap_rtc_delta;
static int ticnt_save;

/*!
 ***********************************************************************
 * -Function:
 *    imap_rtc_suspend
 *	  imap_rtc_resume
 *
 * -Description:
 *	  RTC PM functions, RTC should be disabled before suspend, and re-enabled
 *	  after resuem. Alarm state should be leave alone.
 *
 * -Input Param
 *	  pdev		platform device pointer provided by system
 *    state		Standard function interface required, but not used here
 *
 * -Output Param
 *	  None
 *                
 * -Return
 *	  0			On success
 *
 * -Others
 *	  None
 ***********************************************************************
 */
static int imap_rtc_suspend(struct platform_device *pdev, pm_message_t state)
{
	struct rtc_time tm;
	struct timespec time;
	
	time.tv_nsec = 0;
	/* save TICNT for anyone using periodic interrupts */
	ticnt_save = readb(imap_rtc_base + IMAPX200_TICNT);

	imap_rtc_gettime(&pdev->dev, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);
	save_time_delta(&imap_rtc_delta, &time);

	return 0;
}

static int imap_rtc_resume(struct platform_device *pdev)
{
	struct rtc_time tm = {.tm_sec = 0,};
	struct timespec time;

	time.tv_nsec = 0;

	while(!tm.tm_sec)
		imap_rtc_gettime(&pdev->dev, &tm);
	rtc_tm_to_time(&tm, &time.tv_sec);

	if(!rtc_valid_tm(&tm))
	  do_settimeofday(&time);

	restore_time_delta(&imap_rtc_delta, &time);
	writeb(ticnt_save, imap_rtc_base + IMAPX200_TICNT);
	return 0;
}
#endif

static struct platform_driver imap_rtc_driver = {
	.probe		= imap_rtc_probe,
	.remove		= imap_rtc_remove,
#ifdef CONFIG_PM
	.suspend	= imap_rtc_suspend,
	.resume		= imap_rtc_resume,
#endif
	.driver		=	{
		.name	= "imapx200_rtc",
		.owner	= THIS_MODULE,
	},
};


static int __init imap_rtc_init(void)
{
	printk(KERN_INFO "iMAPx200 RTC, (c) 2009, 2014 InfoTM Microelctronics Co., Ltd\n");
	return platform_driver_register(&imap_rtc_driver);
}

static void __exit imap_rtc_exit(void)
{
	platform_driver_unregister(&imap_rtc_driver);
}

module_init(imap_rtc_init);
module_exit(imap_rtc_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("warits <warits.wang@infotm.com>");
MODULE_DESCRIPTION("InfoTM iMAP RTC driver");
