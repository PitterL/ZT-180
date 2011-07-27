/* **************************************************
 * **  arch/arm/mach-imapx200/pwm.c
 * **
 * **
 * ** Copyright (c) 2009~2014 ShangHai Infotm .Ltd all rights reserved. 
 * **
 * ** Use of infoTM's code  is governed by terms and conditions 
 * ** stated in the accompanying licensing statment.
 * **   
 * ** Description: PWM TIMER driver for imapx200 SOC
 * **
 * ** Author:
 * **
 * **   Haixu Fu       <haixu_fu@infotm.com>
 * ** Revision History:
 * ** ----------------
 * **  1.1  03/06/2010   Haixu Fu 
 * **************************************************/


#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/clk.h>
#include <linux/mutex.h>

#include <mach/irqs.h>
#include <linux/io.h>
#include <asm/irq.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/sysdev.h>
//#include <plat/regs-pwm.h>
//#include "pwm.h"


#define	IMAP_PWM_CHANNEL  5


typedef struct imap_pwm_chan_s  imap_pwm_chan_t;
static uint32_t save_tcfg0, save_tcfg1, save_tcntb2, save_tcmpb2, save_tcon;

/* struct imap_pwm_chan_s */

struct imap_pwm_chan_s {
	void __iomem		*base;
	
	unsigned char     	channel;
	int			irq;

	struct sys_device	sysdev;

};

//extern imap_pwm_chan_t imap_chans[];

struct resource imapx_pwm_resource[1] = {
	[0] = {
		.start = PWM_BASE_REG_PA,
		.end   = PWM_BASE_REG_PA + 0xFFF,
		.flags = IORESOURCE_MEM,
	}
};

imap_pwm_chan_t imap_chans[IMAP_PWM_CHANNEL];

void  __iomem 		*ioaddr;

#ifdef CONFIG_PM
int imap_pwm_suspend(struct sys_device *pdev, pm_message_t pm)
{
	/* Save the registers */
	save_tcfg0 = readl(ioaddr + IMAP_TCFG0);
	save_tcfg1 = readl(ioaddr + IMAP_TCFG1);
	save_tcon  = readl(ioaddr + IMAP_TCON);
	save_tcntb2 = readl(ioaddr + IMAP_TCNTB2);
	save_tcmpb2 = readl(ioaddr + IMAP_TCMPB2);
	save_tcntb2 = readl(ioaddr + IMAP_TCNTB0);
	save_tcmpb2 = readl(ioaddr + IMAP_TCMPB0);
	return 0;
}

int imap_pwm_resume(struct sys_device *pdev)
{
	/* Restore the registers */
	writel(save_tcfg0, ioaddr + IMAP_TCFG0);
	writel(save_tcfg1, ioaddr + IMAP_TCFG1);
	writel(save_tcon, ioaddr + IMAP_TCON);
	writel(save_tcntb2, ioaddr + IMAP_TCNTB2);
	writel(save_tcmpb2, ioaddr + IMAP_TCMPB2);
	writel(save_tcntb2, ioaddr + IMAP_TCNTB0);
	writel(save_tcmpb2, ioaddr + IMAP_TCMPB0);
	return 0;
}
#else
#define imap_pwm_suspend   NULL
#define	imap_pwm_resume    NULL
#endif

struct sysdev_class pwm_sysclass = {
	.name 		= "imap-pwm",
	.suspend	= imap_pwm_suspend,
	.resume		= imap_pwm_resume,
};

void pwm_writel(void __iomem *base, int offset,int value)
{
	__raw_writel(value,base+offset);
}

u32 pwm_readl(void __iomem  *base,int offset)
{
	return __raw_readl(base+offset);
}
/* imap PWM initialisation */

int imap_pwm_start(int chan)
{
	unsigned long tcon;
	
	tcon = pwm_readl(ioaddr,IMAP_TCON);
	
	switch(chan){
		case 0:
			tcon |= IMAP_TCON_T0START;
			tcon &= ~IMAP_TCON_T0MU_ON;
			break;
		case 1:
			tcon |= IMAP_TCON_T1START;
			tcon &= ~IMAP_TCON_T1MU_ON;
			break;
		case 2:
			tcon |= IMAP_TCON_T2START;
			tcon &= ~IMAP_TCON_T2MU_ON;
			break;
		case 3:	
			tcon |= IMAP_TCON_T3START;
			tcon &= ~IMAP_TCON_T3MU_ON;
			break;
		case 4 :
			tcon |= IMAP_TCON_T4START;
			tcon &= ~IMAP_TCON_T4MU_ON;
	}
	
	pwm_writel(ioaddr,IMAP_TCON,tcon);
	return 0;
}	


int imap_timer_setup(int channel,unsigned long g_tcnt,unsigned long g_tcmp)
{
	unsigned long tcon;
	unsigned long tcnt;
	unsigned long tcmp;
	tcon = pwm_readl(ioaddr,IMAP_TCON);

	switch(channel)
	{
		case 1:
		case 3:
		case 4:
		 	printk(KERN_INFO "Only Timer 2 supported right now\n.");
			return -1;
		case 0:
			tcon &= ~(7);
			tcon |= IMAP_TCON_T0RL_ON;
			break;
		case 2:
			tcon &= ~(7<<12);
			tcon |= IMAP_TCON_T2RL_ON;
			break;
		default:
			printk(KERN_ERR "segment invalid!\n");
			break;	
	}
	
	pwm_writel(ioaddr,IMAP_TCON,tcon);
	
#if 0
	tcnt = 100;
	pwm_writel(ioaddr,IMAP_TCNTB(channel),tcnt);
	tcmp = 75;
	pwm_writel(ioaddr,IMAP_TCMPB(channel),tcmp);
#endif
	tcnt = g_tcnt;
	pwm_writel(ioaddr,IMAP_TCNTB(channel),tcnt);
	tcmp = g_tcmp;
	pwm_writel(ioaddr,IMAP_TCMPB(channel),tcmp);
	
	switch(channel)
	{
		case 0:
			tcon |= IMAP_TCON_T0MU_ON;
			break;
		case 1:
			tcon |= IMAP_TCON_T1MU_ON;
			break;
		case 2:	
			tcon |= IMAP_TCON_T2MU_ON;
			break;
		case 3:
			tcon |= IMAP_TCON_T3MU_ON;
			break;
		case 4:
			tcon |= IMAP_TCON_T4MU_ON;
			break;
		default:
			printk(KERN_ERR "segment invalid\n");
			break;
	} 
	
	pwm_writel(ioaddr,IMAP_TCON,tcon);
	
	imap_pwm_start(channel);

	/* XXX DEBUG XXX */
#if 0
	printk(KERN_INFO "The regs: cfg0=%08x, cfg1=%08x,\ntcon=%08x, tcnt2=%08x, tcmp2=%08x\n\n",
	   readl(ioaddr + IMAP_TCFG0),
	   readl(ioaddr + IMAP_TCFG1),
	   readl(ioaddr + IMAP_TCON),
	   readl(ioaddr + IMAP_TCNTB2),
	   readl(ioaddr + IMAP_TCMPB2));
#endif

	
	return 0;
}

EXPORT_SYMBOL(imap_timer_setup);


static int __init imap_init_pwm(void)
{
	//struct pwm_host *host;
	imap_pwm_chan_t *cp ;
 	int channel;
	int ret;
	uint32_t tmp;
	struct resource  *res0,*res1;
	
	printk(KERN_ERR "iMAP PWM Driver Init.\n");
	
//	host = malloc(sizeof(struct pwm_host));	
	res0 = &imapx_pwm_resource[0];

	res1 = request_mem_region(res0->start,resource_size(res0),"imap_pwm");
	if(!res1)
	{
		printk(KERN_ERR " [imap_pwm]-FAILED TO RESERVE MEM REGION \n");
		return -ENXIO;
	}

	ioaddr = ioremap_nocache(res0->start,resource_size(res0));
	
 	if(!ioaddr)
	{
		printk(KERN_ERR "[imap_pwm]-FAILED TO MEM REGIDSTERS\n");
		return -ENXIO;	
	}	

	ret = sysdev_class_register(&pwm_sysclass);
	if(ret != 0)
	{
		printk(KERN_ERR "[imap_pwm]-pwm system device registration failed\n");
		return  -ENODEV;
	}
	
	for(channel = 0 ;channel < IMAP_PWM_CHANNEL;channel++)
	{
		cp = &imap_chans[channel];
		
		memset(cp, 0, sizeof(imap_pwm_chan_t));
		cp->channel = channel;
		
		cp->irq = channel + IRQ_PWM0;

		/* register sysdev */
		ret = sysdev_register(&cp->sysdev); 	
	}

	tmp = readl(ioaddr + IMAP_TCFG1);
	tmp &= ~(0xf << 8);
	tmp |= (0x8 << 8);
	tmp &= ~0xf;
	tmp |= 0x8;
	writel(tmp, ioaddr + IMAP_TCFG1);
	tmp = readl(ioaddr + IMAP_TCFG0);
	tmp &= ~(0xff);
	writel(tmp, ioaddr + IMAP_TCFG0);
	return 0;
}

__initcall(imap_init_pwm);
