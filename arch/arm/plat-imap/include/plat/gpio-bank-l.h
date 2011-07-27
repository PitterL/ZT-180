/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-l.h
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

#define IMAP_GPL_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPL_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPL_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPL0_CAMIF_OCLK		(0x02 << 0)
#define IMAP_GPL0_EINT_G6_27		(0x00 << 0)

#define IMAP_GPL1_CAMIF_HREF		(0x02 << 2)
#define IMAP_GPL1_EINT_G6_28		(0x00 << 2)
#define IMAP_GPL1_TS_ENABLE		(0x03 << 2)

#define IMAP_GPL2_CAMIF_PCLK		(0x02 << 4)
#define IMAP_GPL2_EINT_G6_29		(0x00 << 4)
#define IMAP_GPL2_TS_CLK		(0x03 << 4)

#define IMAP_GPL3_CAMIF_VSYNC		(0x02 << 6)
#define IMAP_GPL3_EINT_G6_30		(0x00 << 6)
#define IMAP_GPL3_TS_SYNC		(0x03 << 6)

#define IMAP_GPL4_CAMIF_RST		(0x02 << 8)
#define IMAP_GPL4_EINT_G6_31		(0x00 << 8)

#define IMAP_GPL5_CAMIF_DAT0		(0x02 << 10)
#define IMAP_GPL5_EINT_G5_24		(0x00 << 10)
#define IMAP_GPL5_TS_DAT0		(0x03 << 10)

#define IMAP_GPL6_CAMIF_DAT1		(0x02 << 12)
#define IMAP_GPL6_EINT_G5_25		(0x00 << 12)
#define IMAP_GPL6_TS_DAT1		(0x03 << 12)

#define IMAP_GPL7_CAMIF_DAT2		(0x02 << 14)
#define IMAP_GPL7_EINT_G5_26		(0x00 << 14)
#define IMAP_GPL7_TS_DAT2		(0x03 << 14)

#define IMAP_GPL8_CAMIF_DAT3		(0x02 << 16)
#define IMAP_GPL8_EINT_G5_27		(0x00 << 16)
#define IMAP_GPL8_TS_DAT3		(0x03 << 16)

#define IMAP_GPL9_CAMIF_DAT4		(0x02 << 18)
#define IMAP_GPL9_EINT_G5_28		(0x00 << 18)
#define IMAP_GPL9_TS_DAT4		(0x03 << 18)

#define IMAP_GPL10_CAMIF_DAT5		(0x02 << 20)
#define IMAP_GPL10_EINT_G5_29		(0x00 << 20)
#define IMAP_GPL10_TS_DAT5		(0x03 << 20)

#define IMAP_GPL11_CAMIF_DAT6		(0x02 << 22)
#define IMAP_GPL11_EINT_G5_30		(0x00 << 22)
#define IMAP_GPL11_TS_DAT6		(0x03 << 22)

#define IMAP_GPL12_CAMIF_DAT7		(0x02 << 24)
#define IMAP_GPL12_EINT_G5_31		(0x00 << 24)
#define IMAP_GPL12_TS_DAT7		(0x03 << 24)

