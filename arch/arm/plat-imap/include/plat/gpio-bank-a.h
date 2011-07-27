/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-a.h
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


#define IMAP_GPA_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPA_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPA_OUTPUT(__gpio)	(0x1 << ((__gpio) * 2))

#define IMAP_GPA0_UART_RXD0		(0x02 << 0)
#define IMAP_GPA0_EINT_G1_0		(0x00 << 0)

#define IMAP_GPA1_UART_TXD0		(0x02 << 2)
#define IMAP_GPA1_EINT_G1_1		(0x00 << 2)

#define IMAP_GPA2_UART_nCTS0		(0x02 << 4)
#define IMAP_GPA2_EINT_G1_2		(0x00 << 4)

#define IMAP_GPA3_UART_nRTS0		(0x02 << 6)
#define IMAP_GPA3_EINT_G1_3		(0x00 << 6)

#define IMAP_GPA4_UART_RXD1		(0x02 << 8)
#define IMAP_GPA4_EINT_G1_4		(0x00 << 8

#define IMAP_GPA5_UART_TXD1		(0x02 << 10)
#define IMAP_GPA5_EINT_G1_5		(0x00 << 10)

#define IMAP_GPA6_UART_nCTS1		(0x02 << 12)
#define IMAP_GPA6_EINT_G1_6		(0x00 << 12)

#define IMAP_GPA7_UART_nRTS1		(0x02 << 14)
#define IMAP_GPA7_EINT_G1_7		(0x00 << 14)

