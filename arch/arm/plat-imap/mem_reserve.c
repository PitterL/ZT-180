/********************************************************************************
** linux-2.6.31.6/arch/arm/plat-imap/mem_reserve.c
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
**     1.0  12/17/2009    Raymond Wang
********************************************************************************/
#include <linux/mm.h>
#include <linux/bootmem.h>
#include <linux/swap.h>
#include <asm/setup.h>
#include <linux/io.h>
#include <mach/memory.h>

#include "plat/mem_reserve.h"

static struct imap_reservemem_device reservemem_devs[RESERVEMEM_DEV_MAX] = {
        {
                .id = RESERVEMEM_DEV_ETH,
                .name = "iMAP_Ethernet",

#ifdef CONFIG_IMAP_RESERVEMEM_SIZE_ETH
                .size = CONFIG_IMAP_RESERVEMEM_SIZE_ETH * SZ_1K,
#else
                .size = 0,
#endif
                .paddr = 0,
        },
#if 0	/* modified by sololz */
	/*
	 * First we decide to reserve memory by Linux kernel reserve 
	 * method for pmem, then we use last 48MB(to be fixed) of 
	 * physical address to be used for pmem.
	 */
	{
                .id = RESERVEMEM_DEV_PMEM,
                .name = "android_pmem",

#ifdef CONFIG_IMAP_RESERVEMEM_SIZE_PMEM
                .size = CONFIG_IMAP_RESERVEMEM_SIZE_PMEM * SZ_1K,
#else
                .size = 0,
#endif
                .paddr = 0,
        },
#endif
#ifdef CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE_SIZE
	{
		.id	= RESERVEMEM_DEV_MEMALLOC,
		.name	= "memalloc",
		.size	= CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE_SIZE * SZ_1M,
		.paddr	= 0,
	},
#endif	/* CONFIG_IMAP_MEMALLOC_SYSTEM_RESERVE_SIZE */
};

static struct imap_reservemem_device *get_reservemem_device(int dev_id)
{
	struct imap_reservemem_device *dev = NULL;
	int i, found;

	if (dev_id < 0 || dev_id >= RESERVEMEM_DEV_MAX)
		return NULL;

	i = 0;
	found = 0;
	while (!found && (i < RESERVEMEM_DEV_MAX)) {
		dev = &reservemem_devs[i];
		if (dev->id == dev_id)
			found = 1;
		else
			i++;
	}

	if (!found)
		dev = NULL;

	return dev;
}

dma_addr_t imap_get_reservemem_paddr(int dev_id)
{
	struct imap_reservemem_device *dev;

	dev = get_reservemem_device(dev_id);
	if (!dev){
		printk(KERN_ERR "invalid device!\n");
		return 0;
	}

	if (!dev->paddr) {
		printk(KERN_ERR "no memory for %s\n", dev->name);
		return 0;
	}

	return dev->paddr;
}
EXPORT_SYMBOL(imap_get_reservemem_paddr);

size_t imap_get_reservemem_size(int dev_id)
{
	struct imap_reservemem_device *dev;

	dev = get_reservemem_device(dev_id);
	if (!dev){
		printk(KERN_ERR "invalid device!\n");
		return 0;
	}

	return dev->size;
}
EXPORT_SYMBOL(imap_get_reservemem_size);

void imap_mem_reserve(void)
{
	struct imap_reservemem_device *dev;
	int i;

	for(i = 0; i < sizeof(reservemem_devs) / sizeof(reservemem_devs[0]); i++) {
		dev = &reservemem_devs[i];
		if (dev->size > 0) {
			dev->paddr = virt_to_phys(alloc_bootmem_low(dev->size));
			printk(KERN_INFO \
				"iMAPx200: %lu bytes SDRAM reserved "
				"for %s at 0x%08x\n",
				(unsigned long) dev->size, dev->name, dev->paddr);
		}
	}
}

EXPORT_SYMBOL(imap_mem_reserve);
