/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-n.h
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

#define IMAP_GPN_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPN_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPN_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPN0_IDS_VD16		(0x02 << 0)
#define IMAP_GPN0_KB_COL14		(0x03 << 0)
#define IMAP_GPN0_EINT_G5_21		(0x00 << 0)

#define IMAP_GPN1_IDS_VD17		(0x02 << 2)
#define IMAP_GPN1_KB_COL15		(0x03 << 2)
#define IMAP_GPN1_EINT_G5_22		(0x00 << 2)

#define IMAP_GPN2_IDS_VD18		(0x02 << 4)
#define IMAP_GPN2_KB_COL16		(0x03 << 4)
#define IMAP_GPN2_EINT_G5_23		(0x00 << 4)

#define IMAP_GPN3_IDS_VD19		(0x02 << 6)

#define IMAP_GPN4_IDS_VD20		(0x02 << 8)

#define IMAP_GPN5_IDS_VD21		(0x02 << 10)

#define IMAP_GPN6_IDS_VD22		(0x02 << 12)

#define IMAP_GPN7_IDS_VD23		(0x02 << 14)

#define IMAP_GPN8_IDS_VCLK		(0x02 << 16)

#define IMAP_GPN9_IDS_VSYNC		(0x02 << 18)

#define IMAP_GPN10_IDS_HSYNC		(0x02 << 20)

#define IMAP_GPN11_IDS_VDEN		(0x02 << 22)

#define IMAP_GPN12_IDS_PWREN		(0x02 << 24)
