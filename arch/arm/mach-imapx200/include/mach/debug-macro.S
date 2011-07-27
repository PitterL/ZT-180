/* arch/arm/mach-imap/include/mach/debug-macro.S
 *
 * Debugging macro include header
 *
 *  Copyright (C) 1994-1999 Russell King
 *  Copyright (C) 2005 Simtec Electronics
 *
 *  Moved from linux/arch/arm/kernel/debug.S by Ben Dooks
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <mach/imap_addr.h>
#include <plat/imapx.h>
//#include <mach/regs-gpio.h>

//Add by Raymond
#define S3C2410_GPIOREG(x) ((x) + IMAP_VA_GPIO)
#define S3C2410_URXH      (0x24)
#define S3C2410_UTXH      (0x20)
#define S3C2410_ULCON     (0x00)
#define S3C2410_UCON      (0x04)
#define S3C2410_UFCON     (0x08)
#define S3C2410_UMCON     (0x0C)
#define S3C2410_UBRDIV    (0x28)
#define S3C2410_UTRSTAT   (0x10)
#define S3C2410_UERSTAT   (0x14)
#define S3C2410_UFSTAT    (0x18)
#define S3C2410_UMSTAT    (0x1C)
#define S3C2410_UFSTAT_TXFULL     (1<<9)
#define S3C2410_UFSTAT_RXFULL     (1<<8)
#define S3C2410_UFSTAT_TXMASK     (15<<4)
#define S3C2410_UFSTAT_TXSHIFT    (4)
#define S3C2410_UFSTAT_RXMASK     (15<<0)
#define S3C2410_UFSTAT_RXSHIFT    (0)
#define S3C2410_GSTATUS0   S3C2410_GPIOREG(0x0AC)
#define S3C2410_GSTATUS1   S3C2410_GPIOREG(0x0B0)
#define S3C2410_GSTATUS2   S3C2410_GPIOREG(0x0B4)
#define S3C2410_GSTATUS3   S3C2410_GPIOREG(0x0B8)
#define S3C2410_GSTATUS4   S3C2410_GPIOREG(0x0BC)
#define S3C2410_UFCON_FIFOMODE    (1<<0)
#define S3C2440_UFSTAT_TXMASK     (63<<8)
#define S3C2410_UTRSTAT_TXFE      (1<<1)
//End add


#define S3C2410_UART1_OFF (0x4000)
#define SHIFT_2440TXF (14-9)

	.macro addruart, rx
		mrc	p15, 0, \rx, c1, c0
		tst	\rx, #1
		ldreq	\rx, = UART0_BASE_ADDR 
		ldrne	\rx, = IMAP_VA_UART
#if CONFIG_DEBUG_S3C_UART != 0
		add	\rx, \rx, #(S3C2410_UART1_OFF * CONFIG_DEBUG_S3C_UART)
#endif
	.endm

	.macro fifo_full_s3c24xx rd, rx
		@ check for arm920 vs arm926. currently assume all arm926
		@ devices have an 64 byte FIFO identical to the s3c2440
		mrc	p15, 0, \rd, c0, c0
		and	\rd, \rd, #0xff0
		teq	\rd, #0x260
		beq	1004f
		mrc	p15, 0, \rd, c1, c0
		tst	\rd, #1
		addeq	\rd, \rx, #(UART0_BASE_ADDR - GPIO_BASE_REG_PA)
		addne	\rd, \rx, #(IMAP_VA_UART - IMAP_VA_GPIO)
		bic	\rd, \rd, #0xff000
		ldr	\rd, [ \rd, # S3C2410_GSTATUS1 - S3C2410_GPIOREG(0) ]
		and	\rd, \rd, #0x00ff0000
		teq	\rd, #0x00440000		@ is it 2440?
1004:
		ldr	\rd, [ \rx, # S3C2410_UFSTAT ]
		moveq	\rd, \rd, lsr #SHIFT_2440TXF
		tst	\rd, #S3C2410_UFSTAT_TXFULL
	.endm

	.macro  fifo_full_s3c2410 rd, rx
		ldr	\rd, [ \rx, # S3C2410_UFSTAT ]
		tst	\rd, #S3C2410_UFSTAT_TXFULL
	.endm

/* fifo level reading */

	.macro fifo_level_s3c24xx rd, rx
		@ check for arm920 vs arm926. currently assume all arm926
		@ devices have an 64 byte FIFO identical to the s3c2440
		mrc	p15, 0, \rd, c0, c0
		and	\rd, \rd, #0xff0
		teq	\rd, #0x260
		beq	10000f
		mrc	p15, 0, \rd, c1, c0
		tst	\rd, #1
		addeq	\rd, \rx, #(UART0_BASE_ADDR - GPIO_BASE_REG_PA)
		addne	\rd, \rx, #(IMAP_VA_UART - IMAP_VA_GPIO)
		bic	\rd, \rd, #0xff000
		ldr	\rd, [ \rd, # S3C2410_GSTATUS1 - S3C2410_GPIOREG(0) ]
		and	\rd, \rd, #0x00ff0000
		teq	\rd, #0x00440000		@ is it 2440?

10000:
		ldr	\rd, [ \rx, # S3C2410_UFSTAT ]
		andne	\rd, \rd, #S3C2410_UFSTAT_TXMASK
		andeq	\rd, \rd, #S3C2440_UFSTAT_TXMASK
	.endm

	.macro fifo_level_s3c2410 rd, rx
		ldr	\rd, [ \rx, # S3C2410_UFSTAT ]
		and	\rd, \rd, #S3C2410_UFSTAT_TXMASK
	.endm

/* Select the correct implementation depending on the configuration. The
 * S3C2440 will get selected by default, as these are the most widely
 * used variants of these
*/

#if defined(CONFIG_CPU_LLSERIAL_S3C2410_ONLY)
#define fifo_full  fifo_full_s3c2410
#define fifo_level fifo_level_s3c2410
#elif !defined(CONFIG_CPU_LLSERIAL_S3C2440_ONLY)
#define fifo_full  fifo_full_s3c24xx
#define fifo_level fifo_level_s3c24xx
#endif

/* include the reset of the code which will do the work */

#include <plat/debug-macro.S>
