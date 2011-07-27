/* arch/arm/mach-imap/include/mach/irqs.h
 *
 * Copyright (c) 2003-2005 Simtec Electronics
 *   Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/


#ifndef __ASM_ARCH_IRQS_H
#define __ASM_ARCH_IRQS_H __FILE__

#ifndef __ASM_ARM_IRQ_H
#error "Do not include this directly, instead #include <asm/irq.h>"
#endif

/* IMAPX200 IRQ */
#define IMAPX200_IRQ_BASE	0
#define IMAPX200_IRQ(x)		((x) + IMAPX200_IRQ_BASE)

#define IRQ_EINT0		IMAPX200_IRQ(0)
#define IRQ_EINT1		IMAPX200_IRQ(1)
#define IRQ_EINT2		IMAPX200_IRQ(2)
#define IRQ_EINT3		IMAPX200_IRQ(3)
#define IRQ_EINT4		IMAPX200_IRQ(4)
#define IRQ_EINT5		IMAPX200_IRQ(5)
#define IRQ_AC97		IMAPX200_IRQ(6)
#define IRQ_IIS			IMAPX200_IRQ(7)
#define IRQ_RTCTICK		IMAPX200_IRQ(8)
#define IRQ_WDT			IMAPX200_IRQ(9)
#define IRQ_PWM0		IMAPX200_IRQ(10)
#define IRQ_PWM1		IMAPX200_IRQ(11)
#define IRQ_PWM2		IMAPX200_IRQ(12)
#define IRQ_UART2		IMAPX200_IRQ(13)
#define IRQ_IIC0			IMAPX200_IRQ(14)
#define IRQ_PWM3		IMAPX200_IRQ(15)
#define IRQ_IDS			IMAPX200_IRQ(16)
#define IRQ_CAM			IMAPX200_IRQ(17)
#define IRQ_TIMER0		IMAPX200_IRQ(18)
#define IRQ_TIMER1		IMAPX200_IRQ(19)
#define IRQ_DMA			IMAPX200_IRQ(20)
#define IRQ_SSI0_MST	IMAPX200_IRQ(21)
#define IRQ_SSI2_MST	IMAPX200_IRQ(22)
#define IRQ_UART1	IMAPX200_IRQ(23)
#define IRQ_NFCON		IMAPX200_IRQ(24)
#define IRQ_USBOTG		IMAPX200_IRQ(25)
#define IRQ_USBOHCI		IMAPX200_IRQ(26)
#define IRQ_RESERVED0		IMAPX200_IRQ(27)
#define IRQ_EMIF			IMAPX200_IRQ(28)
#define IRQ_MONDBG		IMAPX200_IRQ(29)
#define IRQ_PowerMode	IMAPX200_IRQ(30)
#define IRQ_RESERVED1           IMAPX200_IRQ(31)
#define IRQ_Ethernet		IMAPX200_IRQ(32)
#define IRQ_UART3		IMAPX200_IRQ(33)
#define IRQ_IIC1			IMAPX200_IRQ(34)
#define IRQ_IDE_CF		IMAPX200_IRQ(35)
#define IRQ_SDIO1		IMAPX200_IRQ(36)
#define IRQ_SDIO2		IMAPX200_IRQ(37)
#define IRQ_PWM4		IMAPX200_IRQ(38)
#define IRQ_DSPIPC		IMAPX200_IRQ(39)
#define IRQ_VENC		IMAPX200_IRQ(40)
#define IRQ_VDEC		IMAPX200_IRQ(41)
#define IRQ_GPS			IMAPX200_IRQ(42)
#define IRQ_MEMPOOL	IMAPX200_IRQ(43)
#define IRQ_USBEHCI		IMAPX200_IRQ(44)
#define IRQ_PS2_0		IMAPX200_IRQ(45)
#define IRQ_KeyBoard	IMAPX200_IRQ(46)
#define IRQ_GPIO			IMAPX200_IRQ(47)
#define IRQ_SDIO0		IMAPX200_IRQ(48)
#define IRQ_RESERVED2           IMAPX200_IRQ(49)
#define IRQ_PS2_1		IMAPX200_IRQ(50)
#define IRQ_UART0		IMAPX200_IRQ(51)
#define IRQ_SSI1_MST	IMAPX200_IRQ(52)
#define IRQ_RTCALARM	IMAPX200_IRQ(53)
#define IRQ_2D3DGPU		IMAPX200_IRQ(54)
#define IRQ_RESERVED3           IMAPX200_IRQ(55)

#endif /* __ASM_ARCH_IRQ_H */
