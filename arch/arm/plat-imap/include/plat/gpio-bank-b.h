/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-b.h
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


#define IMAP_GPB_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPB_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPB_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPB0_UART_RXD2		(0x02 << 0)
#define IMAP_GPB0_EINT_G2_0		(0x00 << 0)
#define IMAP_GPB0_KB_COL10		(0x03 << 0)

#define IMAP_GPB1_UART_TXD2		(0x02 << 2)
#define IMAP_GPB1_EINT_G2_1		(0x00 << 2)
#define IMAP_GPB0_KB_COL11		(0x03 << 2)

#define IMAP_GPB2_UART_RXD3		(0x02 << 4)
#define IMAP_GPB2_EINT_G2_2		(0x00 << 4)
#define IMAP_GPB2_KB_COL12		(0x03 << 4)

#define IMAP_GPB3_UART_RXD3		(0x02 << 6)
#define IMAP_GPB3_EINT_G2_3		(0x00 << 6)
#define IMAP_GPB3_KB_COL13		(0x03 << 6)


#define IMAP_GPB4_IRDA_ENABLE		(0x01 << 8)
#define IMAP_GPB4_EINT_G2_4		(0x00 << 8)
#define IMAP_GPB4_KB_COL9		(0x03 << 8)
#define IMAP_GPB4_CAM_FIELD		(0x00 << 8)
