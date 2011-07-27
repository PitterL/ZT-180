/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/cpu.h
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

#define IODESC_ENT(x) { (unsigned long)IMAP_VA_##x, __phys_to_pfn(IMAP_PA_##x), IMAP_SZ_##x, MT_DEVICE }

#ifndef MHZ
#define MHZ (1000*1000)
#endif

#define print_mhz(m) ((m) / MHZ), ((m / 1000) % 1000)

/* forward declaration */
struct imap_uart_resources;
struct platform_device;
struct imapx200_uartcfg;
struct map_desc;

/* core initialisation functions */
extern void imap_init_irq(void);

extern void imap_init_io(struct map_desc *mach_desc, int size);

extern void imap_init_uarts(struct imapx200_uartcfg *cfg, int no);

extern void imap_init_clocks(int xtal);

extern void imap_init_uartdevs(char *name,
				  struct imap_uart_resources *res,
				  struct imapx200_uartcfg *cfg, int no);

struct imap_board {
        struct platform_device  **devices;
        unsigned int              devices_count;

        struct clk              **clocks;
        unsigned int              clocks_count;
};

extern void imap_set_board(struct imap_board *board);

/* timer */
struct sys_timer;
extern struct sys_timer imap_timer;

/* system device classes */

extern struct sysdev_class imapx200_sysclass;
