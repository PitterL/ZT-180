/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-p.h
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

#define IMAP_GPP_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPP_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPP_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPP0_SRAM_ADDR23		(0x02 << 0)
#define IMAP_GPP0_CF_ADDR0             	(0x02 << 0)  

#define IMAP_GPP1_SRAM_ADDR24		(0x02 << 2)
#define IMAP_GPP1_CF_ADDR1             	(0x02 << 2)  

#define IMAP_GPP2_SRAM_ADDR25		(0x02 << 4)
#define IMAP_GPP2_CF_ADDR2             	(0x02 << 4)  

#define IMAP_GPP3_CF_IORD		(0x02 << 6)

#define IMAP_GPP4_CF_IOWR		(0x02 << 8)

#define IMAP_GPP5_CF_RESET		(0x02 << 10)

#define IMAP_GPP6_CF_POWER		(0x02 << 12)

#define IMAP_GPP7_CF_CS0N		(0x02 << 14)

#define IMAP_GPP8_CF_CS1N		(0x02 << 16)

#define IMAP_GPP9_CF_IORDY		(0x02 << 18)
#define IMAP_GPP9_SRAM_WAIT		(0x02 << 18)

#define IMAP_GPP10_CF_INT		(0x02 << 20)

#define IMAP_GPP11_CF_CDIN		(0x02 << 22)
