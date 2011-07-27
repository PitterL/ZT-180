/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-r.h
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

#define IMAP_GPR_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPR_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPR_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPR0_FLASH_DAT0		(0x02 << 0)

#define IMAP_GPR1_FLASH_DAT1		(0x02 << 2)

#define IMAP_GPR2_FLASH_DAT2		(0x02 << 4)

#define IMAP_GPR3_FLASH_DAT3		(0x02 << 6)

#define IMAP_GPR4_FLASH_DAT4		(0x02 << 8)

#define IMAP_GPR5_FLASH_DAT5		(0x02 << 10)

#define IMAP_GPR6_FLASH_DAT6		(0x02 << 12)

#define IMAP_GPR7_FLASH_DAT7		(0x02 << 14)

#define IMAP_GPR8_FLASH_DAT8		(0x02 << 16)

#define IMAP_GPR9_FLASH_DAT9		(0x02 << 18)

#define IMAP_GPR10_FLASH_DAT10		(0x02 << 20)

#define IMAP_GPR11_FLASH_DAT11		(0x02 << 22)

#define IMAP_GPR12_FLASH_DAT12		(0x02 << 24)

#define IMAP_GPR13_FLASH_DAT13		(0x02 << 26)

#define IMAP_GPR14_FLASH_DAT14		(0x02 << 28)

#define IMAP_GPR15_FLASH_DAT15		(0x02 << 30)

