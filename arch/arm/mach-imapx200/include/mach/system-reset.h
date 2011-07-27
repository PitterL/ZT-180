/***************************************************************************** 
 * ** linux/arch/arm/mach-imap/include/mach/system-reset.h 
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: IMAPX200 system define for arch_reset() function.
 * **
 * ** Author:
 * **     Alex Zhang   <tao_zhang@infotm.com>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.1  18/09/2009  Alex Zhang   
 * *****************************************************************************/ 

#include <linux/io.h>

#include <plat/regs-watchdog.h>
#include <plat/regs-clock.h>

#include <linux/clk.h>
#include <linux/err.h>

extern void (*imap_reset_hook)(void);

static void
arch_reset(char mode, const char *cmd)
{
	struct clk *wdtclk;

	if(mode = 's'){
		cpu_reset(0);
	}

	if(imap_reset_hook)
		imap_reset_hook();

	printk("arch_reset: attemping watching reset\n");

//	__raw_writel(0, S3C2410_WTCON);	  /* disable watchdog, to be safe  */

	wdtclk = clk_get(NULL, "watchdog");
	if (!IS_ERR(wdtclk)) {
		clk_enable(wdtclk);
	} else
		printk(KERN_WARNING "%s: warning: cannot get watchdog clock\n", __func__);

	/* put initial values into count and data */
//	__raw_writel(0x80, S3C2410_WTCNT);
//	__raw_writel(0x80, S3C2410_WTDAT);

	/* set the watchdog to go and reset... */
//	__raw_writel(S3C2410_WTCON_ENABLE|S3C2410_WTCON_DIV16|S3C2410_WTCON_RSTEN |
//		     S3C2410_WTCON_PRESCALE(0x20), S3C2410_WTCON);

	/* wait for reset to assert... */
	mdelay(500);

	printk(KERN_ERR "Watchdog reset failed to assert reset\n");

	/* delay to allow the serial port to show the message */
	mdelay(50);

	/* we'll take a jump through zero as a poor second */
	cpu_reset(0);

}
