/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/clock.h
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
**     1.1  09/15/2009    Raymond Wang
**     1.2  01/21/2010    Raymond Wang
********************************************************************************/

struct clk {
	struct list_head      list;
	struct module        *owner;
	struct clk           *parent;
	const char           *name;
	int		      id;
	int		      usage;
	unsigned long         rate;
	unsigned long         ctrlbit;

	int		    (*enable)(struct clk *, int enable);
	int		    (*set_rate)(struct clk *c, unsigned long rate);
	unsigned long	    (*get_rate)(struct clk *c);
	unsigned long	    (*round_rate)(struct clk *c, unsigned long rate);
	int		    (*set_parent)(struct clk *c, struct clk *parent);
};

/*core clock support*/
extern struct clk clk_apll;
extern struct clk clk_dpll;
extern struct clk clk_epll;
extern struct clk clk_c;
extern struct clk clk_h;
extern struct clk clk_hx2;
extern struct clk clk_p;
extern struct clk clk_xtal;
extern struct clk clk_xext;

#if 0
/* other clocks which may be registered by board support */

extern struct clk clk_usb_bus;
#endif //if0


extern struct mutex clocks_mutex;

extern int imapx200_clkcon_enable(struct clk *clk, int enable);

extern int imap_register_clock(struct clk *clk);
extern int imap_register_clocks(struct clk **clk, int nr_clks);

extern int imap_register_coreclks(unsigned long xtal);
