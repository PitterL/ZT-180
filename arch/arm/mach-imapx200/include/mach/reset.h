/***************************************************************************** 
 * ** linux/arch/arm/mach-imap/include/mach/reset.h 
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: IMAPX200 CPU reset controls.
 * **
 * ** Author:
 * **     Alex Zhang   <tao_zhang@infotm.com>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.1  18/09/2009  Alex Zhang   
 * *****************************************************************************/ 
#ifndef __ASM_ARCH_RESET_H
#define __ASM_ARCH_RESET_H __FILE__

/* This allows the over-ride of the default reset code
*/

extern void (*imap_reset_hook)(void);

#endif /* __ASM_ARCH_RESET_H */
