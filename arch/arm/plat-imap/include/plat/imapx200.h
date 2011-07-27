/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/imapx200.h
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
********************************************************************************/

extern  int imapx200_init(void);

extern void __init imdkx200_map_io(struct map_desc *mach_desc, int size);

extern void __init imdkx200_init_irq(void);


extern void imapx200_init_uarts(struct imapx200_uartcfg *cfg, int no);

extern void imapx200_init_clocks(int xtal);

extern  int imapx200_baseclk_add(void);

