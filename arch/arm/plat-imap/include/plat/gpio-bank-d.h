/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-d.h
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

#define IMAP_GPD_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPD_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPD_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPD0_I2S_SCLK		(0x02 << 0)
#define IMAP_GPD0_AC97_BITCLK		(0x03 << 0)

#define IMAP_GPD1_I2S_CDCLK		(0x02 << 2)
#define IMAP_GPD1_AC97_nRESET		(0x03 << 2)

#define IMAP_GPD2_I2S_LRCLK		(0x02 << 4)
#define IMAP_GPD2_AC97_SYNC		(0x03 << 4)

#define IMAP_GPD3_I2S_SDI		(0x02 << 6)
#define IMAP_GPD3_AC97_SDIN		(0x04 << 6)

#define IMAP_GPD4_I2S_SDO		(0x02 << 8)
#define IMAP_GPD4_AC97_SDOUT		(0x03 << 8)

