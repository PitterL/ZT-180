/********************************************************************************
** linux-2.6.31.6/arch/arm/plat-imap/include/plat/mem_reserve.h
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
#ifndef _MEM_RESERVE_H
#define _MEM_RESERVE_H

#include <linux/types.h>

#define RESERVEMEM_DEV_ETH		0
#define RESERVEMEM_DEV_MEMALLOC		1
#define RESERVEMEM_DEV_MAX		2

struct imap_reservemem_device {
	int		id;
	const char 	*name;
	size_t		size;
	dma_addr_t	paddr;
};

extern dma_addr_t imap_get_reservemem_paddr(int dev_id);
extern size_t imap_get_reservemem_size(int dev_id);

#endif
