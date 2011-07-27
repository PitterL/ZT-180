/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-f.h
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

#define IMAP_GPF_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPF_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPF_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPF0_SDIO0_CLK		(0x02 << 0)

#define IMAP_GPF1_SDIO0_CMD		(0x02 << 2)

#define IMAP_GPF2_SDIO0_DAT0		(0x02 << 4)
#define IMAP_GPF2_EINT_G4_16		(0x00 << 4)

#define IMAP_GPF3_SDIO0_DAT1		(0x02 << 6)
#define IMAP_GPF3_EINT_G4_17		(0x00 << 6)

#define IMAP_GPF4_SDIO0_DAT2		(0x02 << 8)
#define IMAP_GPF4_EINT_G4_18		(0x00 << 8)

#define IMAP_GPF5_SDIO0_DAT3		(0x02 << 10)
#define IMAP_GPF5_EINT_G4_19		(0x00 << 10)

#define IMAP_GPF6_TOUT0			(0x02 << 12)
#define IMAP_GPF6_EINT_G4_20		(0x00 << 12)

#define IMAP_GPF7_TOUT1			(0x02 << 14)
#define IMAP_GPF7_EINT_G4_21		(0x00 << 14)

#define IMAP_GPF8_TOUT2			(0x02 << 16)
#define IMAP_GPF8_EINT_G4_22		(0x00 << 16)

#define IMAP_GPF9_TOUT3			(0x02 << 18)
#define IMAP_GPF9_EINT_G4_23		(0x00 << 18)


