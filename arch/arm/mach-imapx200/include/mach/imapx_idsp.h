/*****************************************************************************
** imapx_idsp.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:	iDSP registers
**
** Author:
**     warits    <warits@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  08/04/2010  Initialized by warits.
*****************************************************************************/


#ifndef __ARM_IMAPX_REGS_IDSP
#define __ARM_IMAPX_REGS_IDSP

#define iDSP_REG_BASE		(0x24000000)
 
#define iDSP_DSP_CTRL			(0x00) 
#define iDSP_DSP_RESET		    (0x04) 
#define iDSP_MAILBOX0			(0x08) 
#define iDSP_MAILBOX1			(0x0c) 
#define iDSP_MAILBOX(x)			(0x08 + (x << 2))
#define iDSP_FIFOSTATUS0		(0x10) 
#define iDSP_FIFOSTATUS1		(0x14) 
#define iDSP_FIFOSTATUS(x)		(0x10 + (x << 2)) 
#define iDSP_MSG_NUM0		    (0x18) 
#define iDSP_MSG_NUM1		    (0x1c) 
#define iDSP_MSG_COUNT(x)		(0x18 + (x << 2)) 
#define iDSP_DSP_INTSTAT		(0x20) 
#define iDSP_DSP_INTEN		    (0x24) 
#define iDSP_CPU_INTSTAT		(0x28) 
#define iDSP_CPU_INTEN		    (0x2c) 
#define iDSP_INTSTAT(x)			(0x20 + (x << 3))
#define iDSP_INTEN(x)			(0x24 + (x << 3))
#define iDSP_EPM_BASE			(0x30) 
#define iDSP_EDM_BASE			(0x34) 
#define iDSP_CSR_BASE			(0x38) 
#define iDSP_EXTINT_MASK		(0x3c) 
#define iDSP_ENDIAN_MODE0		(0x40) 
#define iDSP_CACHE_ENABLE	    (0x44) 
#define iDSP_CACHE_FLUSH		(0x48) 
#define iDSP_ENDIAN_MODE1		(0x4c) 
#define iDSP_ENDIAN_MODE(x)		(0x40 + ((x) * 0xc))
#define iDSP_BOOT_INSTR		    (0x100) 

#define iDSP_FIFOSTATUS_FULL		(1 << 0)
#define iDSP_INTSTAT_NOTFULL1		(1 << 3)
#define iDSP_INTSTAT_NEWMSG1		(1 << 2)
#define iDSP_INTSTAT_NOTFULL0		(1 << 1) 
#define iDSP_INTSTAT_NEWMSG0		(1 << 0) 
#define iDSP_INTSTAT_NOFULL(x)		(1 << (1 + (x << 1)))
#define iDSP_INTSTAT_NEWMSG(x)		(1 << (x << 1)) 
#define iDSP_INTEN_NOTFULL1			(1 << 3)
#define iDSP_INTEN_NEWMSG1			(1 << 2)
#define iDSP_INTEN_NOTFULL0			(1 << 1) 
#define iDSP_INTEN_NEWMSG0			(1 << 0) 
#define iDSP_INT_ENABLE				(1)
//#define iDSP_INT_DISABLE			(0) 
//#define iDSP_CACHE_Enable			(1)
//#define iDSP_CACHE_DISABLE			(0)
//#define iDSP_MAILBOX_0				(0)
#define iDSP_MAILBOX_1				(1)

#define iDSP_DoReset				(1 << 0)
#define iDSP_MSG_COUNT_MSK			(0x7)


#if 0
#define CPU_READ32_ByteExchg		(0x800) 
#define CPU_READ32_WordExchg		(0x400) 
#define CPU_WRITE32_ByteExchg	    (0x200) 
#define CPU_WRITE32_WordExchg	    (0x100) 
#define CPU_READ16_ByteExchg		(0x80) 
#define CPU_READ16_WordExchg		(0x40) 
#define CPU_WRITE16_ByteExchg	    (0x20) 
#define CPU_WRITE16_WordExchg	    (0x10) 
#define CPU_READ8_ByteExchg		    (0x8) 
#define CPU_READ8_WordExchg		    (0x4) 
#define CPU_WRITE8_ByteExchg		(0x2) 
#define CPU_WRITE8_WordExchg		(0x1) 
 
#define DSP_READ32_ByteExchg		(0x800000) 
#define DSP_READ32_WordExchg		(0x400000) 
#define DSP_WRITE32_ByteExchg	    (0x200000) 
#define DSP_WRITE32_WordExchg	    (0x100000) 
#define DSP_READ16_ByteExchg		(0x80000) 
#define DSP_READ16_WordExchg		(0x40000) 
#define DSP_WRITE16_ByteExchg	    (0x20000) 
#define DSP_WRITE16_WordExchg	    (0x10000) 
 
// --------------------------------------- 
// DSP 外部中断MASK位 
#define DSP_BIT_USBHOST			(1<<0) 
#define DSP_BIT_USBSLAVE		(1<<1) 
#define DSP_BIT_PWM0			(1<<2) 
#define DSP_BIT_UART0			(1<<3) 
#define DSP_BIT_IIC0			(1<<4) 
#define DSP_BIT_GPIO			(1<<5) 
#define DSP_BIT_VIDEODEC		(1<<6) 
#define DSP_BIT_VIDEOENC		(1<<7) 
#define DSP_BIT_CAMIF			(1<<8) 
#define DSP_BIT_NANDFLASH		(1<<9) 
#define DSP_BIT_CF				(1<<10) 
#define DSP_BIT_PWM1			(1<<11) 
#define DSP_BIT_LCDC			(1<<12) 
#define DSP_BIT_GPS				(1<<13) 
#define DSP_BIT_DMA				(1<<14) 
#define DSP_BIT_SPI0			(1<<15) 
#define DSP_BIT_SD				(1<<16) 
#define DSP_BIT_2D				(1<<17) 
#define DSP_BIT_IIS				(1<<18) 
#define DSP_BIT_UART1			(1<<19) 
#define DSP_BIT_UART2			(1<<20) 
#define DSP_BIT_DSPEXT			(1<<21) 
#define DSP_BIT_IIC1			(1<<22) 
#define DSP_BIT_SPI1			(1<<23) 

#define TCM_ABS_OFST	(0x2C000000)	//(0x0C000000) 
// cache 
#define CACHE_BASE	    (0x0) 
#define CACHE_LEN	    (0x4800)		// 18kB 
 
// spram 
#define SPRAM_BASE	    (0x4800) 
#define SPRAM_LEN	    (0x10000)		//  64kB 
 
// dpram 
#define DPRAM_BASE	    (0x14800) 
#define DPRAM_LEN	    (0x8000)		//  32KB 
 
#define TCM_BASE        (CACHE_BASE) 
#define TCM_LEN         (0x100000) 

// --------------------------------------- 
// DSP的看到的外部内存空间 
// EPM 
#define EPM_BASE		(0x200000) 
#define EPM_LEN		    (0x600000)	// 6MB 
 
// EDM 
#define EDM_BASE		(0x800000) 
#define EDM_LEN		    (0x400000)	// 4MB 
 
// CSR 
#define CSR_BASE		(0xC00000) 
#define CSR_LEN		    (0x400000)	// 4MB 
 
// --------------------------------------- 
// 握手标志 
#define HAND_SHAKE		(0x12345678) 
#endif
 
#endif
