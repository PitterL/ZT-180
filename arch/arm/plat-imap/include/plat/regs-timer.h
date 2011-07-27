/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/regs-timer.h
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


#ifndef __ASM_ARCH_REGS_TIMER_H
#define __ASM_ARCH_REGS_TIMER_H


#if defined (CONFIG_CPU_IMAPX200) 
/* TIMER */
#define IMAP_TIMERREG(x) (IMAP_VA_TIMER + (x))

#define IMAP_TLC0             IMAP_TIMERREG(0x00)    //timer0 Load Count register
#define IMAP_TCV0             IMAP_TIMERREG(0x04)    //timer0 Current Value register
#define IMAP_TCR0             IMAP_TIMERREG(0x08)    //timer0 Control register
#define IMAP_TEOI0            IMAP_TIMERREG(0x0C)    //timer0 End-of-Interrupt register
#define IMAP_TINTST0          IMAP_TIMERREG(0x10)    //timer0 Interrupt Status register

#define IMAP_TLC1             IMAP_TIMERREG(0x14)    //timer1 Load Count register
#define IMAP_TCV1             IMAP_TIMERREG(0x18)    //timer1 Current Value register
#define IMAP_TCR1             IMAP_TIMERREG(0x1C)    //timer1 Control register
#define IMAP_TEOI1            IMAP_TIMERREG(0x20)    //timer1 End-of-Interrupt register
#define IMAP_TINTST1          IMAP_TIMERREG(0x24)    //timer1 Interrupt Status register

#define IMAP_TSINTST          IMAP_TIMERREG(0xA0)    //Timers Interrupt Status register
#define IMAP_TSEOI            IMAP_TIMERREG(0xA4)    //Timers End-of-Interrupt register
#define IMAP_TSRINTST         IMAP_TIMERREG(0xA8)    //Timers Raw Interrupt Status register
#define IMAP_TSCOMPVERSION    IMAP_TIMERREG(0xAC)    //Timers Component Version

#define IMAP_TCR_TIMER_EN          (1<<0)
#define IMAP_TCR_TIMER_MD          (1<<1)
#define IMAP_TCR_TIMER_INTMASK     (1<<2)



#endif


#endif /*  __ASM_ARCH_REGS_TIMER_H */



