/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-c.h
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

#define IMAP_GPC_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPC_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPC_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPC0_IIC_SCL0             (0x02 << 0)
#define IMAP_GPC0_EINT_G3_0		(0x00 << 0)

#define IMAP_GPC1_IIC_SDA0		(0x02 << 2)
#define IMAP_GPC1_EINT_G3_1		(0x00 << 2)

#define IMAP_GPC2_IIC_SCL1		(0x02 << 4)
#define IMAP_GPC2_EINT_G3_2		(0x00 << 4)

#define IMAP_GPC3_IIC_SDA1		(0x02 << 6)
#define IMAP_GPC3_EINT_G3_3		(0x00 << 6)

#define IMAP_GPC4_PIC_CLK0		(0x02 << 8)
#define IMAP_GPC4_EINT_G3_4		(0x00 << 8)

#define IMAP_GPC5_PIC_DATA0		(0x02 << 10)
#define IMAP_GPC5_EINT_G3_5		(0x00 << 10)

#define IMAP_GPC6_PIC_CLK1		(0x02 << 12)
#define IMAP_GPC6_EINT_G3_6		(0x00 << 12)

#define IMAP_GPC7_PIC_DATA1		(0x02 << 14)
#define IMAP_GPC7_EINT_G3_7		(0x00 << 14)

