/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-e.h
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
#define IMAP_GPE_CONMASK(__gpio)	(0xf << ((__gpio) * 2))
#define IMAP_GPE_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPE_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPE0_MASTER_SSI1_RXD	(0x02 << 0)
#define IMAP_GPE0_SLAVE_SSI_TXD		(0x03 << 0)
#define IMAP_GPE0_EINT_G4_0		(0x00 << 0) 

#define IMAP_GPE1_MASTER_SSI1_CLK	(0x02 << 2)
#define IMAP_GPE1_SLAVE_SSI_CLK		(0x03 << 2)
#define IMAP_GPE1_EINT_G4_1		(0x00 << 2) 

#define IMAP_GPE2_MASTER_SSI1_TXD	(0x02 << 4)
#define IMAP_GPE2_SLAVE_SSI_RXD		(0x03 << 4)
#define IMAP_GPE2_EINT_G4_2		(0x00 << 4) 

#define IMAP_GPE3_MASTER_SSI1_CSN0	(0x02 << 6)
#define IMAP_GPE3_SLAVE_SSI_CSN		(0x03 << 6)
#define IMAP_GPE3_EINT_G4_3		(0x00 << 6) 

#define IMAP_GPE4_MASTER_SSI0_TXD	(0x02 << 8)
#define IMAP_GPE4_EINT_G4_4		(0x00 << 8) 

#define IMAP_GPE5_MASTER_SSI0_RXD	(0x02 << 10)
#define IMAP_GPE5_EINT_G4_5		(0x00 << 10) 

#define IMAP_GPE6_MASTER_SSI0_CLK	(0x02 << 12)
#define IMAP_GPE6_SRAM_CSN3		(0x03 << 12)
#define IMAP_GPE6_EINT_G4_6		(0x00 << 12) 

#define IMAP_GPE7_MASTER_SSI0_CSN0	(0x02 << 14)
#define IMAP_GPE7_SRAM_CSN2		(0x03 << 14)
#define IMAP_GPE7_EINT_G4_7		(0x00 << 14) 

#define IMAP_GPE8_MASTER_SSI0_CSN1    	(0x02 << 16)
#define IMAP_GPE8_SRAM_BEN0		(0x03 << 16)
#define IMAP_GPE8_EINT_G4_8		(0x00 << 16) 

#define IMAP_GPE9_MASTER_SSI0_CSN2	(0x02 << 18)
#define IMAP_GPE9_SRAM_BEN1		(0x03 << 18)
#define IMAP_GPE9_EINT_G4_9		(0x00 << 18) 

#define IMAP_GPE10_MASTER_SSI0_CSN3	(0x02 << 20)
#define IMAP_GPE10_SRAM_CSN1		(0x03 << 20)
#define IMAP_GPE10_EINT_G4_10		(0x00 << 20) 

#define IMAP_GPE11_GPS_DIN0		(0x02 << 22)
#define IMAP_GPE11_EINT_G4_11		(0x00 << 22) 

#define IMAP_GPE12_DIN1			(0x02 << 24)
#define IMAP_GPE12_EINT_G4_12		(0x00 << 24) 

#define IMAP_GPE13_CLK			(0x02 << 26)
#define IMAP_GPE13_EINT_G4_13		(0x00 << 26) 

#define IMAP_GPE14_NAND_CSN1		(0x02 << 28)
#define IMAP_GPE14_EINT_G4_14		(0x00 << 28) 

#define IMAP_GPE15_OTG_DRV_VBUS		(0x02 << 30)
#define IMAP_GPE15_EINT_G4_15		(0x00 << 30) 


