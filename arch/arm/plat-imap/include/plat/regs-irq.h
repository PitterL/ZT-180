/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/regs-irq.h
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
**     Raymond Wang   <raymond.wang@infotmic.com.cn>
**
** Revision History:
**     1.1  09/15/2009    Raymond Wang
********************************************************************************/

#ifndef ___ASM_ARCH_REGS_IRQ_H
#define ___ASM_ARCH_REGS_IRQ_H

/* interrupt controller */
#define IMAP_INTCONREG(x)	((x) + IMAP_VA_IRQ)

#if defined (CONFIG_CPU_IMAPX200)

#define IMAP_SRCPND0		IMAP_INTCONREG(0x00)
#define IMAP_SRCPND1		IMAP_INTCONREG(0x24)

#define IMAP_INTMOD0		IMAP_INTCONREG(0x04)
#define IMAP_INTMOD1		IMAP_INTCONREG(0x28)

#define IMAP_INTMSK0		IMAP_INTCONREG(0x08)
#define IMAP_INTMSK1		IMAP_INTCONREG(0x2C)

#define IMAP_PRIORITY0		IMAP_INTCONREG(0x0C)
#define IMAP_PRIORITY1		IMAP_INTCONREG(0x30)

#define IMAP_INTPND0		IMAP_INTCONREG(0x10)
#define IMAP_INTPND1		IMAP_INTCONREG(0x34)

#define IMAP_INTOFFSET		IMAP_INTCONREG(0x14)
#define IMAP_SUBSRCPND		IMAP_INTCONREG(0x18)
#define IMAP_INTSUBMSK		IMAP_INTCONREG(0x1C)
#define IMAP_CONT		IMAP_INTCONREG(0x20)

#define IMAP_DEINTMSK1		IMAP_INTCONREG(0x38)
#define IMAP_DEINTMSK2		IMAP_INTCONREG(0x3C)

#endif

#endif /* ___ASM_ARCH_REGS_IRQ_H */
