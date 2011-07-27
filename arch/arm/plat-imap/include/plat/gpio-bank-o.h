/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/gpio-bank-o.h
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

#define IMAP_GPO_CONMASK(__gpio)	(0x3 << ((__gpio) * 2))
#define IMAP_GPO_INPUT(__gpio)		(0x0 << ((__gpio) * 2))
#define IMAP_GPO_OUTPUT(__gpio)		(0x1 << ((__gpio) * 2))

#define IMAP_GPO0_SDIO1_CLK			(0x02 << 0)
#define IMAP_GPO0_USB_HOST_P3_VPI_I		(0x03 << 0)

#define IMAP_GPO1_SDIO1_CMD			(0x02 << 2)
#define IMAP_GPO1_USB_HOST_P3_SPEED_O		(0x03 << 2)

#define IMAP_GPO2_SDIO1_DAT0			(0x02 << 4)
#define IMAP_GPO2_USB_HOST_P3_FS_SE0_O		(0x03 << 4)
#define IMAP_GP02_EINT_G4_24			(0x00 << 4)

#define IMAP_GPO3_SDIO1_DAT1			(0x02 << 6)
#define IMAP_GPO3_USB_HOST_P3_DATA_O		(0x03 << 6)
#define IMAP_GP03_EINT_G4_25			(0x00 << 6)

#define IMAP_GPO4_SDIO1_DAT2			(0x02 << 8)
#define IMAP_GPO4_USB_HOST_SUSPEND_O		(0x03 << 8)
#define IMAP_GP04_EINT_G4_26			(0x00 << 8)

#define IMAP_GPO5_SDIO1_DAT3			(0x02 << 10)
#define IMAP_GPO5_USB_HOST_P3_FS_OE_O		(0x03 << 10)
#define IMAP_GP05_EINT_G4_27			(0x00 << 10)

#define IMAP_GPO6_SDIO2_CLK			(0x02 << 12)
#define IMAP_GPO6_MASTER_SSI2_CLK		(0x03 << 12)

#define IMAP_GPO7_SDIO2_CMD			(0x02 << 14)
#define IMAP_GPO7_MASTER_SSI2_RXD		(0x03 << 14)

#define IMAP_GPO8_SDIO2_DAT0			(0x02 << 16)
#define IMAP_GPO8_MASTER_SSI2_TXD		(0x03 << 16)
#define IMAP_GPO8_EINT_G4_28			(0x00 << 16)

#define IMAP_GPO9_SDIO2_DAT1			(0x02 << 18)
#define IMAP_GPO9_EINT_G4_29			(0x00 << 18)

#define IMAP_GPO10_SDIO2_DAT2			(0x02 << 20)
#define IMAP_GPO10_EINT_G4_30			(0x00 << 20)

#define IMAP_GPO11_SDIO2_DAT3			(0x02 << 22)
#define IMAP_GPO11_EINT_G4_31			(0x00 << 22)
#define IMAP_GPO11_MASTER_SSI2_CSN             	(0x03 << 22)

#define IMAP_GPO12_UART1_DSR			(0x02 << 24)
#define IMAP_GPO12_CF_DMA_ACK			(0x03 << 24)

#define IMAP_GPO13_UART1_DCD			(0x02 << 26)
#define IMAP_GPO13_USB_HOST_P3_LS_FS_RCV_I	(0x03 << 26)

#define IMAP_GPO14_UART1_RI			(0x02 << 28)
#define IMAP_GPO14_CF_DMA_REQ			(0x03 << 28)

#define IMAP_GPO15_UART1_DTR			(0x02 << 30)
#define IMAP_GPO15_USB_HOST_P3_VMI_I		(0x03 << 30)
