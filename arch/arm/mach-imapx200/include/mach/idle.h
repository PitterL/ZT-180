/***************************************************************************** 
 * ** linux/arch/arm/mach-imap/include/mach/idle.h 
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: IMAPX200 Idle controls.
 * **
 * ** Author:
 * **     Alex Zhang   <tao_zhang@infotm.com>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.1  17/09/2009  Alex Zhang   
 * *****************************************************************************/ 

#ifndef __ASM_ARCH_IDLE_H
#define __ASM_ARCH_IDLE_H __FILE__

/* This allows the over-ride of the default idle code, in case there
* is any other things to be done over idle (like DVS)
*/
extern void (*imap_idle)(void);

extern void imap_default_idle(void);

#endif /* __ASM_ARCH_IDLE_H */

