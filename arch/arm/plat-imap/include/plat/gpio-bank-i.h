/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-i.h
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

#define IMAP_GPI_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPI_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPI_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPI0_SRAM_ADDR0		(0x02 << 0)
#define IMAP_GPI0_KB_COL0		(0x03 << 0)
#define IMAP_GPI0_EINT_G6_4		(0x00 << 0)

#define IMAP_GPI1_SRAM_ADDR1		(0x02 << 2)
#define IMAP_GPI1_KB_COL1		(0x03 << 2)
#define IMAP_GPI1_EINT_G6_5		(0x00 << 2)

#define IMAP_GPI2_SRAM_ADDR2		(0x02 << 4)
#define IMAP_GPI2_KB_COL2		(0x03 << 4)
#define IMAP_GPI2_EINT_G6_6		(0x00 << 4)

#define IMAP_GPI3_SRAM_ADDR3		(0x02 << 6)
#define IMAP_GPI3_KB_COL3		(0x03 << 6)
#define IMAP_GPI3_EINT_G6_7		(0x00 << 6)

#define IMAP_GPI4_SRAM_ADDR4		(0x02 << 8)
#define IMAP_GPI4_KB_COL4		(0x03 << 8)
#define IMAP_GPI4_EINT_G6_8		(0x00 << 8)

#define IMAP_GPI5_SRAM_ADDR5		(0x02 << 10)
#define IMAP_GPI5_KB_COL5		(0x03 << 10)
#define IMAP_GPI5_EINT_G6_9		(0x00 << 10)

#define IMAP_GPI6_SRAM_ADDR6		(0x02 << 12)
#define IMAP_GPI6_KB_COL6		(0x03 << 12)
#define IMAP_GPI6_EINT_G6_10		(0x00 << 12)

#define IMAP_GPI7_SRAM_ADDR7		(0x02 << 14)
#define IMAP_GPI7_KB_COL7		(0x03 << 14)
#define IMAP_GPI7_EINT_G6_11		(0x00 << 14)

#define IMAP_GPI8_SRAM_ADDR8		(0x02 << 16)
#define IMAP_GPI8_KB_COL8		(0x03 << 16)
#define IMAP_GPI8_EINT_G6_12		(0x00 << 16)

#define IMAP_GPI9_SRAM_ADDR9		(0x02 << 18)
#define IMAP_GPI9_KB_COL9		(0x03 << 18)
#define IMAP_GPI9_EINT_G6_13		(0x00 << 18)

#define IMAP_GPI10_SRAM_ADDR10		(0x02 << 20)
#define IMAP_GPI10_KB_COL10		(0x03 << 20)
#define IMAP_GPI10_EINT_G6_14		(0x00 << 20)

#define IMAP_GPI11_SRAM_ADDR11		(0x02 << 22)
#define IMAP_GPI11_KB_COL11		(0x03 << 22)
#define IMAP_GPI11_EINT_G6_15		(0x00 << 22)

#define IMAP_GPI12_SRAM_ADDR12		(0x02 << 24)
#define IMAP_GPI12_KB_COL12		(0x03 << 24)
#define IMAP_GPI12_EINT_G6_16		(0x00 << 24)

#define IMAP_GPI13_SRAM_ADDR13		(0x02 << 26)
#define IMAP_GPI13_KB_COL13		(0x03 << 26)
#define IMAP_GPI13_EINT_G6_17		(0x00 << 26)

