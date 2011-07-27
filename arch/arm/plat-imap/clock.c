/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/clock.c
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
**     1.2  11/25/2009    Raymond Wang
**     1.3  01/21/2010    Raymond Wang
********************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/sysdev.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/io.h>

#include <mach/hardware.h>
#include <asm/irq.h>

#include <plat/imapx.h>
#include <plat/clock.h>
#include <plat/cpu.h>

#ifdef CONFIG_IMAP_FPGA
#include <plat/fpga_test.h>
#endif

/* clock information */

static LIST_HEAD(clocks);

DEFINE_MUTEX(clocks_mutex);

/* enable and disable calls for use with the clk struct */

static int clk_null_enable(struct clk *clk, int enable)
{
	return 0;
}

/* Clock API calls */

struct clk *clk_get(struct device *dev, const char *id)
{
	struct clk *p;
	struct clk *clk = ERR_PTR(-ENOENT);
	int idno;

	if (dev == NULL || dev->bus != &platform_bus_type)
		idno = -1;
	else
		idno = to_platform_device(dev)->id;

	mutex_lock(&clocks_mutex);

	list_for_each_entry(p, &clocks, list) {
		if (p->id == idno &&
		    strcmp(id, p->name) == 0 &&
		    try_module_get(p->owner)) {
			clk = p;
			break;
		}
	}

	/* check for the case where a device was supplied, but the
	 * clock that was being searched for is not device specific */

	if (IS_ERR(clk)) {
		list_for_each_entry(p, &clocks, list) {
			if (p->id == -1 && strcmp(id, p->name) == 0 &&
			    try_module_get(p->owner)) {
				clk = p;
				break;
			}
		}
	}

	mutex_unlock(&clocks_mutex);
	return clk;
}

void clk_put(struct clk *clk)
{
	module_put(clk->owner);
}

int clk_enable(struct clk *clk)
{
	if (IS_ERR(clk) || clk == NULL)
		return -EINVAL;

	clk_enable(clk->parent);

	mutex_lock(&clocks_mutex);

	if ((clk->usage++) == 0)
		(clk->enable)(clk, 1);

	mutex_unlock(&clocks_mutex);
	return 0;
}

void clk_disable(struct clk *clk)
{
	if (IS_ERR(clk) || clk == NULL)
		return;

	mutex_lock(&clocks_mutex);

	if ((--clk->usage) == 0)
		(clk->enable)(clk, 0);

	mutex_unlock(&clocks_mutex);
	clk_disable(clk->parent);
}


unsigned long clk_get_rate(struct clk *clk)
{
	if (IS_ERR(clk))
		return 0;

	if (clk->rate != 0)
		return clk->rate;

	if (clk->get_rate != NULL)
		return (clk->get_rate)(clk);

	if (clk->parent != NULL)
		return clk_get_rate(clk->parent);

	return clk->rate;
}

long clk_round_rate(struct clk *clk, unsigned long rate)
{
	if (!IS_ERR(clk) && clk->round_rate)
		return (clk->round_rate)(clk, rate);

	return rate;
}

int clk_set_rate(struct clk *clk, unsigned long rate)
{
	int ret;

	if (IS_ERR(clk))
		return -EINVAL;

	/* We do not default just do a clk->rate = rate as
	 * the clock may have been made this way by choice.
	 */

	WARN_ON(clk->set_rate == NULL);

	if (clk->set_rate == NULL)
		return -EINVAL;

	mutex_lock(&clocks_mutex);
	ret = (clk->set_rate)(clk, rate);
	mutex_unlock(&clocks_mutex);

	return ret;
}

struct clk *clk_get_parent(struct clk *clk)
{
	return clk->parent;
}

int clk_set_parent(struct clk *clk, struct clk *parent)
{
	int ret = 0;

	if (IS_ERR(clk))
		return -EINVAL;

	mutex_lock(&clocks_mutex);

	if (clk->set_parent)
		ret = (clk->set_parent)(clk, parent);

	mutex_unlock(&clocks_mutex);

	return ret;
}

EXPORT_SYMBOL(clk_get);
EXPORT_SYMBOL(clk_put);
EXPORT_SYMBOL(clk_enable);
EXPORT_SYMBOL(clk_disable);
EXPORT_SYMBOL(clk_get_rate);
EXPORT_SYMBOL(clk_round_rate);
EXPORT_SYMBOL(clk_set_rate);
EXPORT_SYMBOL(clk_get_parent);
EXPORT_SYMBOL(clk_set_parent);

static int clk_default_setrate(struct clk *clk, unsigned long rate)
{
	clk->rate = rate;
	return 0;
}

/* Core Clocks */
struct clk clk_apll= {
	.name		= "apll",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
	.set_rate	= clk_default_setrate,
};

struct clk clk_dpll= {
	.name		= "dpll",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
};

struct clk clk_epll = {
	.name		= "epll",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
};

struct clk clk_c = {
	.name		= "cpuclk",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
	.set_rate	= clk_default_setrate,
};

struct clk clk_h = {
	.name		= "hclk",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
	.set_rate	= clk_default_setrate,
};

struct clk clk_p = {
	.name		= "pclk",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
	.set_rate	= clk_default_setrate,
};

struct clk clk_hx2 = {
	.name		= "hclkx2",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
	.set_rate	= clk_default_setrate,
};

struct clk clk_xtal = {
	.name		= "xtal",
	.id		= -1,
	.rate		= 0,
	.parent		= NULL,
	.ctrlbit	= 0,
};

struct clk clk_pseudo = {
	.name           = "pseudo",
	.id             = -1,
	.rate           = 0,
	.parent         = NULL,
	.ctrlbit        = 0,
};

#if defined(FPGA_TEST)
struct clk clk_xext = {
	.name		= "xext",
	.id		= -1,
	.rate		= 48000000,
	.parent		= NULL,
	.ctrlbit	= 0,
};
#endif


/* Registe clocks functions */

int imap_register_clock(struct clk *clk)
{
	clk->owner = THIS_MODULE;

	if (clk->enable == NULL)
		clk->enable = clk_null_enable;

	/* add to the list of available clocks */

	mutex_lock(&clocks_mutex);
	list_add(&clk->list, &clocks);
	mutex_unlock(&clocks_mutex);

	return 0;
}

int imap_register_clocks(struct clk **clks, int nr_clks)
{
	int fails = 0;

	for (; nr_clks > 0; nr_clks--, clks++) {
		if (imap_register_clock(*clks) < 0)
			fails++;
	}

	return fails;
}


/* initalise all the core clocks */

int __init imap_register_coreclks(unsigned long xtal)
{
	printk(KERN_INFO "IMAP Clocks, (c) 2009 Infotm Micro Electronics\n");

        clk_xtal.rate = xtal;

        /* register our clocks */

        if (imap_register_clock(&clk_xtal) < 0)
                printk(KERN_ERR "failed to register master xtal\n");

#if defined(FPGA_TEST)
        if (imap_register_clock(&clk_xext) < 0)
                printk(KERN_ERR "failed to register xext clock\n");
#endif

        if (imap_register_clock(&clk_apll) < 0)
                printk(KERN_ERR "failed to register apll clock\n");

        if (imap_register_clock(&clk_dpll) < 0)
                printk(KERN_ERR "failed to register dpll clock\n");

        if (imap_register_clock(&clk_epll) < 0)
                printk(KERN_ERR "failed to register epll clock\n");

        if (imap_register_clock(&clk_c) < 0)
                printk(KERN_ERR "failed to register cpu clock\n");

        if (imap_register_clock(&clk_h) < 0)
                printk(KERN_ERR "failed to register hclk\n");

        if (imap_register_clock(&clk_p) < 0)
                printk(KERN_ERR "failed to register pclk\n");

        if (imap_register_clock(&clk_hx2) < 0)
                printk(KERN_ERR "failed to register hclkx2\n");

        if (imap_register_clock(&clk_pseudo) < 0)
                printk(KERN_ERR "failed to register pseudo clock\n");

        return 0;
}

