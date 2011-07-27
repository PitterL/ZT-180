/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-q.h
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

#define IMAPX_GPQ_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAPX_GPQ_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAPX_GPQ_OUTPUT(__gpio)	(0x1 << ((__gpio) * 2))

#define IMAPX_GPQ0_XND_RNB		(0x02 << 0)

#define IMAPX_GPQ1_XND_CLE		(0x02 << 2)

#define IMAPX_GPQ2_XND_ALE		(0x02 << 4)

#define IMAPX_GPQ3_XND_WEN		(0x02 << 6)

#define IMAPX_GPQ4_XND_REN		(0x02 << 8)

#define IMAPX_GPQ5_XND_CSN0		(0x02 << 10)
