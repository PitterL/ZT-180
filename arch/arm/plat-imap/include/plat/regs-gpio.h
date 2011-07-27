/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/regs-gpio.h
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

#ifndef __ASM_PLAT_IMAP_REGS_GPIO_H
#define __ASM_PLAT_IMAP_REGS_GPIO_H __FILE__

#include <plat/gpio-bank-a.h>
#include <plat/gpio-bank-b.h>
#include <plat/gpio-bank-c.h>
#include <plat/gpio-bank-d.h>
#include <plat/gpio-bank-e.h>
#include <plat/gpio-bank-f.h>
#include <plat/gpio-bank-g.h>
#include <plat/gpio-bank-h.h>
#include <plat/gpio-bank-i.h>
#include <plat/gpio-bank-j.h>
#include <plat/gpio-bank-k.h>
#include <plat/gpio-bank-l.h>
#include <plat/gpio-bank-n.h>
#include <plat/gpio-bank-m.h>
#include <plat/gpio-bank-o.h>
#include <plat/gpio-bank-p.h>
#include <plat/gpio-bank-q.h>
#include <plat/gpio-bank-r.h>
//#include <mach/map.h>

/* Base addresses for each of the banks */

#define IMAP_GPA_BASE	(IMAP_VA_GPIO + 0x0000)
#define IMAP_GPB_BASE	(IMAP_VA_GPIO + 0x0010)
#define IMAP_GPC_BASE	(IMAP_VA_GPIO + 0x0020)
#define IMAP_GPD_BASE	(IMAP_VA_GPIO + 0x0030)
#define IMAP_GPE_BASE	(IMAP_VA_GPIO + 0x0040)
#define IMAP_GPF_BASE	(IMAP_VA_GPIO + 0x0050)
#define IMAP_GPG_BASE	(IMAP_VA_GPIO + 0x0060)
#define IMAP_GPH_BASE	(IMAP_VA_GPIO + 0x0070)
#define IMAP_GPI_BASE	(IMAP_VA_GPIO + 0x0080)
#define IMAP_GPJ_BASE	(IMAP_VA_GPIO + 0x0090)
#define IMAP_GPK_BASE	(IMAP_VA_GPIO + 0x00A0)
#define IMAP_GPL_BASE	(IMAP_VA_GPIO + 0x00B0)
#define IMAP_GPM_BASE	(IMAP_VA_GPIO + 0x00C0)
#define IMAP_GPN_BASE	(IMAP_VA_GPIO + 0x00D0)
#define IMAP_GPO_BASE	(IMAP_VA_GPIO + 0x00E0)
#define IMAP_GPP_BASE	(IMAP_VA_GPIO + 0x00F0)
#define IMAP_GPQ_BASE	(IMAP_VA_GPIO + 0x0100)
#define IMAP_GPR_BASE	(IMAP_VA_GPIO + 0x0110)

#define IMAP_GPIO_CONMASK(__gpio)	(0x3<<((__gpio)*2))

#define IMAP_GPIO_INPUT(__gpio)		(0x0<<((__gpio)*2))
#define IMAP_GPIO_OUTPUT(__gpio)	(0x1<<((__gpio)*2))

#if 0
#define IMAP_MEM0CONSTOP	(IMAP_VA_GPIO + 0x01B0)
#define IMAP_MEM1CONSTOP	(IMAP_VA_GPIO + 0x01B4)
#define IMAP_MEM0CONSLP0	(IMAP_VA_GPIO + 0x01C0)
#define IMAP_MEM0CONSLP1	(IMAP_VA_GPIO + 0x01C4)
#define IMAP_MEM1CONSLP	(IMAP_VA_GPIO + 0x01C8)
#define IMAP_MEM0DRVCON	(IMAP_VA_GPIO + 0x01D0)
#define IMAP_MEM1DRVCON	(IMAP_VA_GPIO + 0x01D4)
#endif 
// external interrupt configuration register
#define IMAP_EINT0CON0	(IMAP_VA_GPIO + 0x0200)
// external interrupt filter control register 0
#define IMAP_EINT0FLTCON0	(IMAP_VA_GPIO + 0x0204)
// external interrupt filter control register 1
#define IMAP_EINT0FLTCON1	(IMAP_VA_GPIO + 0x0208)
// external interrupt group configuration register
#define IMAP_EINTGCON		(IMAP_VA_GPIO + 0x0210)
// external interrupt group filter control register 0
#define IMAP_EINTGFLTCON0      (IMAP_VA_GPIO + 0x0214)
// external interrupt group filter control register 1
#define IMAP_EINTGFLTCON1      (IMAP_VA_GPIO + 0x0218)
// external interrupt group 1 mask register
#define IMAP_EINTG1MASK	(IMAP_VA_GPIO + 0x021C)
// external interrupt group 2 mask register
#define IMAP_EINTG2MASK	(IMAP_VA_GPIO + 0x0220)
// external interrupt group 3 mask register
#define IMAP_EINTG3MASK	(IMAP_VA_GPIO + 0x0224)
// external interrupt group 4 mask register
#define IMAP_EINTG4MASK	(IMAP_VA_GPIO + 0x0228)
// external interrupt group 5 mask register
#define IMAP_EINTG5MASK	(IMAP_VA_GPIO + 0x022C)
// external interrupt group 6 mask register
#define IMAP_EINTG6MASK	(IMAP_VA_GPIO + 0x0230)
// external interrupt group 1 pending register
#define IMAP_EINTG1PEND	(IMAP_VA_GPIO + 0x0234)
// external interrupt group 2 pending register
#define IMAP_EINTG2PEND	(IMAP_VA_GPIO + 0x0238)
// external interrupt group 3 pending register
#define IMAP_EINTG3PEND	(IMAP_VA_GPIO + 0x023C)
// external interrupt group 4 pending register
#define IMAP_EINTG4PEND	(IMAP_VA_GPIO + 0x0240)
// external interrupt group 5 pending register
#define IMAP_EINTG5PEND	(IMAP_VA_GPIO + 0x0244)
// external interrupt group 6 pending register
#define IMAP_EINTG6PEND	(IMAP_VA_GPIO + 0x0248)
/* values for IMAP_EXTINT0 */
#define IMAP_EXTINT_LOWLEV	 (0x00)
#define IMAP_EXTINT_HILEV	 (0x01)
#define IMAP_EXTINT_FALLEDGE	 (0x02)
#define IMAP_EXTINT_RISEEDGE	 (0x04)
#define IMAP_EXTINT_BOTHEDGE	 (0x06)

#if 0
/* for lcd */
#define IMAP_MIFPCON			(0x800c)
#define IMAP_SPCON_LCD_SEL_RGB	(1 << 0)
#define IMAP_SPCON_LCD_SEL_MASK	(3 << 0)
#define IMAP_MIFPCON_LCD_MUX_NORMAL	(0 << 3)
#define IMAP_MIFPCON_LCD_MUX_MASK	(1 << 3)
#endif
#endif /* __ASM_PLAT_IMAP_REGS_GPIO_H */

