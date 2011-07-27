/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-h.h
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
**     James Xu   <James Xu@infotmic.com.cn>
**
** Revision History:
**     1.0  09/15/2009    James Xu
********************************************************************************/

#define IMAP_GPH_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPH_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPH_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPH0_SRAM_ADDR14		(0x02 << 0)
#define IMAP_GPH0_KB_COL14		(0x03 << 0)
#define IMAP_GPH0_EINT_G6_0		(0x00 << 0)

#define IMAP_GPH1_SRAM_ADDR15		(0x02 << 2)
#define IMAP_GPH1_KB_COL15		(0x03 << 2)
#define IMAP_GPH1_EINT_G6_1		(0x00 << 2)

#define IMAP_GPH2_SRAM_ADDR16		(0x02 << 4)
#define IMAP_GPH2_KB_COL16		(0x03 << 4)
#define IMAP_GPH2_EINT_G6_2		(0x00 << 4)

#define IMAP_GPH3_SARM_ADDR17		(0x02 << 6)
#define IMAP_GPH3_KB_COL17		(0x03 << 6)
#define IMAP_GPH3_EINT_G6_3		(0x00 << 6)


