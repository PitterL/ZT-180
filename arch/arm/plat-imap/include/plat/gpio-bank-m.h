/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-m.h
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

#define IMAP_GPM_CONMASK(__gpio) (0x3 << ((__gpio) * 2))
#define IMAP_GPM_INPUT(__gpio)   (0x0 << ((__gpio) * 2))
#define IMAP_GPM_OUTPUT(__gpio)  (0x1 << ((__gpio) * 2))

#define IMAP_GPM0_IDS_VD0      	(0x02 << 0)
#define IMAP_GPM0_KB_COL9      	(0x03 << 0)
#define IMAP_GPM0_EINT_G5_16    (0x00 << 0)

#define IMAP_GPM1_IDS_VD1      	(0x02 << 2)
#define IMAP_GPM1_KB_COL10      (0x03 << 2)
#define IMAP_GPM1_EINT_G5_17    (0x00 << 2)

#define IMAP_GPM2_IDS_VD2      	(0x02 << 4)
#define IMAP_GPM2_KB_COL11      (0x03 << 4)
#define IMAP_GPM2_EINT_G5_18    (0x00 << 4)

#define IMAP_GPM3_IDS_VD3      	(0x02 << 6)

#define IMAP_GPM4_IDS_VD4      	(0x02 << 8)

#define IMAP_GPM5_IDS_VD5      	(0x02 << 10)

#define IMAP_GPM6_IDS_VD6      	(0x02 << 12)

#define IMAP_GPM7_IDS_VD7      	(0x02 << 14)

#define IMAP_GPM8_IDS_VD8      	(0x02 << 16)
#define IMAP_GPM8_KB_COL12      (0x03 << 16)
#define IMAP_GPM8_EINT_G5_19    (0x00 << 16)

#define IMAP_GPM9_IDS_VD9      	(0x02 << 18)
#define IMAP_GPM9_KB_COL13      (0x03 << 18)
#define IMAP_GPM9_EINT_G5_20    (0x00 << 18)

#define IMAP_GPM10_IDS_VD10      (0x02 << 20)

#define IMAP_GPM11_IDS_VD11      (0x02 << 22)

#define IMAP_GPM12_IDS_VD12      (0x02 << 24)

#define IMAP_GPM13_IDS_VD13      (0x02 << 26)

#define IMAP_GPM14_IDS_VD14      (0x02 << 28)

#define IMAP_GPM15_IDS_VD15      (0x02 << 30)








