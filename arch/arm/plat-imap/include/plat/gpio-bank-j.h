/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-j.h
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
#define IMAP_GPJ_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPJ_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPJ_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPJ0_SRAM_ADDR18		(0x02 << 0)
#define IMAP_GPJ0_KB_ROW0		(0x03 << 0)
#define IMAP_GPJ0_EINT_G6_18		(0x00 << 0)

#define IMAP_GPJ1_SRAM_ADDR19		(0x02 << 2)
#define IMAP_GPJ1_KB_ROW1		(0x03 << 2)
#define IMAP_GPJ1_EINT_G6_19		(0x00 << 2)

#define IMAP_GPJ2_SRAM_ADDR20		(0x02 << 4)
#define IMAP_GPJ2_KB_ROW2		(0x03 << 4)
#define IMAP_GPJ2_EINT_G6_20		(0x00 << 4)

#define IMAP_GPJ3_SRAM_ADDR21		(0x02 << 6)
#define IMAP_GPJ3_KB_ROW3		(0x03 << 6)
#define IMAP_GPJ3_EINT_G6_21		(0x00 << 6)

#define IMAP_GPJ4_SRAM_ADDR22		(0x02 << 8)
#define IMAP_GPJ4_KB_ROW4		(0x03 << 8)
#define IMAP_GPJ4_EINT_G6_22		(0x00 << 8)

#define IMAP_GPJ5_SRAM_CSN0		(0x02 << 10)
#define IMAP_GPJ5_KB_ROW5		(0x03 << 10)
#define IMAP_GPJ5_EINT_G6_23		(0x00 << 10)

#define IMAP_GPJ6_SRAM_OEN		(0x02 << 12)
#define IMAP_GPJ6_KB_ROW6		(0x03 << 12)
#define IMAP_GPJ6_EINT_G6_24		(0x00 << 12)

#define IMAP_GPJ7_SRAM_WEN		(0x02 << 14)
#define IMAP_GPJ7_KB_ROW7		(0x03 << 14)
#define IMAP_GPJ7_EINT_G6_25		(0x00 << 14)

#define IMAP_GPJ8_ETHERNET_COL		(0x02 << 16)
#define IMAP_GPJ8_KB_ROW8		(0x03 << 16)
#define IMAP_GPJ8_EINT_G6_26		(0x00 << 16)

