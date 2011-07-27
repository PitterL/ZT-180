/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/time.c
**
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Author:
**     Raymond Wang   <raymond.wang@infotmic.com.cn>
**
** Revision History:
**     1.2  25/11/2009    Raymond Wang
********************************************************************************/

#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/err.h>
#include <linux/clk.h>
#include <linux/io.h>

#include <asm/system.h>
#include <asm/mach-types.h>

#include <asm/irq.h>
#include <asm/mach/time.h>
#include <plat/imapx.h>
#include <plat/clock.h>
#include <plat/cpu.h>

static unsigned long timer_startval;
static unsigned long timer_usec_ticks;
struct clk *clk;

#define TIMER_USEC_SHIFT 16

/* we use the shifted arithmetic to work out the ratio of timer ticks
 * to usecs, as often the peripheral clock is not a nice even multiple
 * of 1MHz.
 *
 * shift of 14 and 15 are too low for the 12MHz, 16 seems to be ok
 * for the current HZ value of 200 without producing overflows.
 *
 * Original patch by Dimitry Andric, updated by Ben Dooks
*/


/* timer_mask_usec_ticks
 *
 * given a clock and divisor, make the value to pass into timer_ticks_to_usec
 * to scale the ticks into usecs
*/

static inline unsigned long
timer_mask_usec_ticks(unsigned long scaler, unsigned long pclk)
{
	unsigned long den = pclk / 1000;

	return ((1000 << TIMER_USEC_SHIFT) * scaler + (den >> 1)) / den;
}

/* timer_ticks_to_usec
 *
 * convert timer ticks to usec.
*/

static inline unsigned long timer_ticks_to_usec(unsigned long ticks)
{
//	unsigned long res;

//	res = ticks * timer_usec_ticks;
//	res += 1 << (TIMER_USEC_SHIFT - 4);	/* round up slightly */

//	return res >> TIMER_USEC_SHIFT;
	return (ticks/60);
}

/***
 * Returns microsecond  since last clock interrupt.  Note that interrupts
 * will have been disabled by do_gettimeoffset()
 * IRQs are disabled before entering here from do_gettimeofday()
 */

static unsigned long imapx200_gettimeoffset (void)
{
	unsigned long tdone;
	unsigned long tval;
        
	tval =  __raw_readl(rTimer0CurrentValue);
	tdone = timer_startval - tval;

	return timer_ticks_to_usec(tdone);
}

/*
 * IRQ handler for the timer
 */
static irqreturn_t
imapx200_timer_interrupt(int irq, void *dev_id)
{
	unsigned long bitval;

	__raw_readl(rTimer0EOI);
	
	bitval = (1UL << irq);
	__raw_writel(bitval, rINTPND);
	__raw_writel(bitval, rSRCPND);

	timer_tick();

	return IRQ_HANDLED;
}

static struct irqaction imapx200_timer_irq = {
	.name		= "IMAP Timer Tick",
	.flags		= IRQF_DISABLED | IRQF_TIMER | IRQF_IRQPOLL,
	.handler	= imapx200_timer_interrupt,
};

static void imapx200_set_timer_clock (void)
{
	unsigned long divcfg1;

	divcfg1 =  __raw_readl(rDIV_CFG1);
	divcfg1 &= ~(IMAP_DIV_CFG1_TIM0 | IMAP_DIV_CFG1_TIMRAT0);
	divcfg1 |= (0x1EUL);
	__raw_writel(divcfg1, rDIV_CFG1);

	/* this is used as default if no other timer can be found */
	clk = clk_get(NULL, "timer0");
	if (IS_ERR(clk))
		panic("failed to get clock for system timer\n");
}

/*
 * Set up timer interrupt, and return the current time in seconds.
 *
 * Currently we only use timer0.
 */
static void imapx200_timer_setup (void)
{
	unsigned long tcr0;
	unsigned long tcnt;
	unsigned long timerclock;
	unsigned long pclk;
	tcnt = 0xffffffff;  /* default value for tcnt */

	/* read the current timer configuration bits */

	tcr0 = __raw_readl(rTimer0ControlReg);
	tcr0 &= ~IMAP_TCR_TIMER_EN;
        __raw_writel(tcr0, rTimer0ControlReg);
#ifdef CONFIG_IMAP_FPGA
	timerclock = CONFIG_FPGA_EXT_CLOCK;
	timer_usec_ticks = timer_mask_usec_ticks(1, CONFIG_FPGA_EXT_CLOCK);
#else
	pclk = clk_get_rate(clk);
	
	timerclock = (pclk / ((PRESCALER + 1)*DIVIDER));
	timer_usec_ticks = timer_mask_usec_ticks(((PRESCALER + 1)*DIVIDER), pclk);
#endif
//	printk("############## timerclock = %ld, HZ = %d\n", timerclock, HZ);
	tcnt = timerclock / (HZ);

	/* timers reload after counting zero, so reduce the count by 1 */
	tcnt--;
	timer_startval = tcnt;

	tcr0 = __raw_readl(rTimer0ControlReg);
	tcr0 |= IMAP_TCR_TIMER_MD;         //set user define count mode
	tcr0 &= ~IMAP_TCR_TIMER_INTMASK;   //no mask
	__raw_writel(tcr0, rTimer0ControlReg);
	__raw_writel(tcnt, rTimer0LoadCount);

//	printk("timer tcr0=%08lx, tcnt %08lx, usec %08lx\n", tcr0, tcnt, timer_usec_ticks);

	/* start the timer running */
	tcr0 |= IMAP_TCR_TIMER_EN;
	__raw_writel(tcr0, rTimer0ControlReg);
}

/*
static irqreturn_t unstable_power_supply(int irq, void *dev_id)
{
	unsigned long powerValue;

	powerValue = __raw_readl(rPOW_STB);
	if (powerValue & 0x4)
		printk(KERN_ERR "WARNING: Unstable power supply for CPU, system will shut down!\n");
	return IRQ_HANDLED;
}
*/

static void __init imapx200_timer_init (void)
{
	imapx200_set_timer_clock();
	setup_irq(IRQ_TIMER0, &imapx200_timer_irq);
	imapx200_timer_setup();
	clk_enable(clk);
//	request_irq(IRQ_PowerMode, unstable_power_supply, IRQF_DISABLED, "unstable_power_supply", NULL);
}

struct sys_timer imapx200_init_timer = {
	.init		= imapx200_timer_init,
	.offset		= imapx200_gettimeoffset,
	.resume		= imapx200_timer_setup
};
