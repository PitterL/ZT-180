/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-k.h
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

#define IMAP_GPK_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPK_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPK_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPK0_ETH_TCLK		(0x02 << 0)
#define IMAP_GPK0_KB_COL0		(0x03 << 0)
#define IMAP_GPK0_EINT_G5_0		(0x00 << 0)

#define IMAP_GPK1_ETH_RCLK		(0x02 << 2)
#define IMAP_GPK1_KB_COL1		(0x03 << 2)
#define IMAP_GPK1_EINT_G5_1		(0x00 << 2)

#define IMAP_GPK2_ETH_MDC		(0x02 << 4)
#define IMAP_GPK2_KB_COL2		(0x03 << 4)
#define IMAP_GPK2_EINT_G5_2		(0x00 << 4)

#define IMAP_GPK3_ETH_MDIO		(0x02 << 6)
#define IMAP_GPK3_KB_COL3		(0x03 << 6)
#define IMAP_GPK3_EINT_G5_3		(0x00 << 6)

#define IMAP_GPK4_ETH_TXEN		(0x02 << 8)
#define IMAP_GPK4_KB_COL4		(0x03 << 8)
#define IMAP_GPK4_EINT_G5_4		(0x00 << 8)

#define IMAP_GPK5_ETH_TXD0		(0x02 << 10)
#define IMAP_GPK5_KB_ROW4		(0x03 << 10)
#define IMAP_GPK5_EINT_G5_5		(0x00 << 10)

#define IMAP_GPK6_ETH_TXD1		(0x02 << 12)
#define IMAP_GPK6_KB_ROW5		(0x03 << 12)
#define IMAP_GPK6_EINT_G5_6		(0x00 << 12)

#define IMAP_GPK7_ETH_TXD2		(0x02 << 14)
#define IMAP_GPK7_KB_ROW6		(0x03 << 14)
#define IMAP_GPK7_EINT_G5_7		(0x00 << 14)

#define IMAP_GPK8_ETH_TXD3		(0x02 << 16)
#define IMAP_GPK8_KB_ROW7		(0x03 << 16)
#define IMAP_GPK8_EINT_G5_8		(0x00 << 16)

#define IMAP_GPK9_ETH_RXDV		(0x02 << 18)
#define IMAP_GPK9_KB_COL5		(0x03 << 18)
#define IMAP_GPK9_EINT_G5_9		(0x00 << 18)

#define IMAP_GPK10_ETH_RXER		(0x02 << 20)
#define IMAP_GPK10_KB_COL6		(0x03 << 20)
#define IMAP_GPK10_EINT_G5_10		(0x00 << 20)

#define IMAP_GPK11_ETH_RXD0		(0x02 << 22)
#define IMAP_GPK11_KB_ROW0		(0x03 << 22)
#define IMAP_GPK11_EINT_G5_11		(0x00 << 22)

#define IMAP_GPK12_ETH_RXD1		(0x02 << 24)
#define IMAP_GPK12_KB_ROW1		(0x03 << 24)
#define IMAP_GPK12_EINT_G5_12		(0x00 << 24)

#define IMAP_GPK13_ETH_RXD2		(0x02 << 26)
#define IMAP_GPK13_KB_ROW2		(0x03 << 26)
#define IMAP_GPK13_EINT_G5_13		(0x00 << 26)

#define IMAP_GPK14_ETH_RXD3		(0x02 << 28)
#define IMAP_GPK14_KB_ROW3		(0x03 << 28)
#define IMAP_GPK14_EINT_G5_14		(0x00 << 28)

#define IMAP_GPK15_ETH_CRS		(0x02 << 30)
#define IMAP_GPK15_KB_COL7		(0x03 << 30)
#define IMAP_GPK15_EINT_G5_15		(0x00 << 30)

