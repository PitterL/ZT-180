/***************************************************************************** 
** arch/arm/plat-imap/include/plat/regs-serial.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Head file of UART.
**
** Author:
**     Feng Jiaxing <jiaxing_feng@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  09/14/2009  Feng Jiaxing
*****************************************************************************/

#ifndef __ASM_ARM_REGS_SERIAL_H
#define __ASM_ARM_REGS_SERIAL_H

/* Register virtual addrss and physical address */

//#define IMAPX200_VA_UART0      (IMAP_VA_UART)
//#define IMAPX200_VA_UART1      (IMAP_VA_UART + 0x1000)
//#define IMAPX200_VA_UART2      (IMAP_VA_UART + 0x2000)
//#define IMAPX200_VA_UART3      (IMAP_VA_UART + 0x3000)

#define IMAPX200_PA_UART0      (UART0_BASE_ADDR)
#define IMAPX200_PA_UART1      (UART0_BASE_ADDR + 0x1000)
#define IMAPX200_PA_UART2      (UART0_BASE_ADDR + 0x2000)
#define IMAPX200_PA_UART3      (UART0_BASE_ADDR + 0x3000)

/* Register definition */
#define IMAPX200_RBR	(0x000)
#define IMAPX200_THR	(0x000)
#define IMAPX200_DLL	(0x000)
#define IMAPX200_DLH	(0x004)
#define IMAPX200_IER	(0x004)
#define IMAPX200_IIR	(0x008)
#define IMAPX200_FCR	(0x008)
#define IMAPX200_LCR	(0x00C)
#define IMAPX200_MCR	(0x010)
#define IMAPX200_LSR	(0x014)
#define IMAPX200_CSR	(0x018)
#define IMAPX200_LPDLL	(0x020)
#define IMAPX200_LPDLH	(0x024)
#define IMAPX200_USR	(0x07C)
#define IMAPX200_TFL	(0x080)
#define IMAPX200_RFL	(0x084)
#define IMAPX200_HTX	(0x0A4)
#define IMAPX200_DMASA	(0x0A8)
#define IMAPX200_CKSR	(0x100)

#define IMAPX200_DLL_DIVISOR_LOW_BYTE(x)	(((x)&0xff)<<0)
#define IMAPX200_DLH_DIVISOR_HIGH_BYTE(x)	(((x)&0xff)<<0)

#define IMAPX200_IER_PTIME_THRE_INT_ENABLE			(1<<7)
#define IMAPX200_IER_PTIME_THRE_INT_DISABLE			(0<<7)
#define IMAPX200_IER_ELSI_LINE_STATUS_INT_ENABLE	(1<<2)
#define IMAPX200_IER_ELSI_LINE_STATUS_INT_DISABLE	(0<<2)
#define IMAPX200_IER_ETBEI_TX_INT_ENABLE				(1<<1)
#define IMAPX200_IER_ETBEI_TX_INT_DISABLE				(0<<1)
#define IMAPX200_IER_ERBFI_RX_INT_ENABLE				(1<<0)
#define IMAPX200_IER_ERBFI_RX_INT_DISABLE				(0<<0)

#define IMAPX200_IIR_FIFOSE_MASK		(0x3<<6)
#define IMAPX200_IIR_FIFOSE_ENABLE	(0x3<<6)
#define IMAPX200_IIR_FIFOSE_DISABLE	(0x0<<6)
#define IMAPX200_IIR_IID_MASK			(0xf<<0)
#define IMAPX200_IIR_IID_NO_INT		(0x1<<0)
#define IMAPX200_IIR_IID_TX				(0x2<<0)
#define IMAPX200_IIR_IID_RX				(0x4<<0)
#define IMAPX200_IIR_IID_LINE_STATUS	(0x6<<0)
#define IMAPX200_IIR_IID_BUSY_DETECT	(0x7<<0)
#define IMAPX200_IIR_IID_TIMEOUT		(0xC<<0)

#define IMAPX200_FCR_RT_RX_TRIGGER_LEVEL_ONE_CHAR			(0x0<<6)
#define IMAPX200_FCR_RT_RX_TRIGGER_LEVEL_QUARTER_FULL		(0x1<<6)
#define IMAPX200_FCR_RT_RX_TRIGGER_LEVEL_HALF_FULL			(0x2<<6)
#define IMAPX200_FCR_RT_RX_TRIGGER_LEVEL_TWO_LESS_FULL		(0x3<<6)
#define IMAPX200_FCR_TET_TX_THRESHOLD_LEVEL_EMPTY			(0x0<<4)
#define IMAPX200_FCR_TET_TX_THRESHOLD_LEVEL_TWO_CHAR		(0x1<<4)
#define IMAPX200_FCR_TET_TX_THRESHOLD_LEVEL_QUARTER_FULL	(0x2<<4)
#define IMAPX200_FCR_TET_TX_THRESHOLD_LEVEL_HALF_FULL		(0x3<<4)
#define IMAPX200_FCR_XFIFOR_TX_FIFO_RESET					(1<<2)
#define IMAPX200_FCR_RFIFOR_RX_FIFO_RESET					(1<<1)
#define IMAPX200_FCR_FIFOE_FIFO_ENABLE						(1<<0)
#define IMAPX200_FCR_FIFOE_FIFO_DISABLE						(0<<0)

#define IMAPX200_LCR_DLAB_ENABLE					(1<<7)
#define IMAPX200_LCR_DLAB_DISABLE					(0<<7)
#define IMAPX200_LCR_Break_ENABLE					(1<<6)
#define IMAPX200_LCR_Break_DISABLE				(0<<6)
#define IMAPX200_LCR_EPS_EVEN_PARITY				(1<<4)
#define IMAPX200_LCR_EPS_ODD_PARITY				(0<<4)
#define IMAPX200_LCR_PEN_PARITY_ENABLE			(1<<3)
#define IMAPX200_LCR_PEN_PARITY_DISABLE			(0<<3)
#define IMAPX200_LCR_STOP_1POINT5_2_STOP_BIT	(1<<2)
#define IMAPX200_LCR_STOP_ONE_STOP_BIT			(0<<2)
#define IMAPX200_LCR_DLS_MASK						(0x3<<0)
#define IMAPX200_LCR_DLS_5BIT						(0x0<<0)
#define IMAPX200_LCR_DLS_6BIT						(0x1<<0)
#define IMAPX200_LCR_DLS_7BIT						(0x2<<0)
#define IMAPX200_LCR_DLS_8BIT						(0x3<<0)

#define IMAPX200_MCR_SIRE_IRDA_ENABLE	(1<<6)
#define IMAPX200_MCR_SIRE_IRDA_DISABLE	(0<<6)
#define IMAPX200_MCR_AFCE_AFC_ENABLE		(1<<5)
#define IMAPX200_MCR_AFCE_AFC_DISABLE	(0<<5)
#define IMAPX200_MCR_LB_LoopBack_ENABLE	(1<<4)
#define IMAPX200_MCR_LB_LoopBack_DISABLE	(0<<4)
#define IMAPX200_MCR_RTS_ENABLE			(1<<1)
#define IMAPX200_MCR_RTS_DISABLE			(0<<1)

#define IMAPX200_LSR_RFE_MASK					(1<<7)
#define IMAPX200_LSR_RFE_RX_FIFO_ERR			(1<<7)
#define IMAPX200_LSR_RFE_RX_FIFO_NO_ERR		(0<<7)
#define IMAPX200_LSR_TEMT_MASK				(1<<6)
#define IMAPX200_LSR_TEMT_XMITER_EMPTY		(1<<6)
#define IMAPX200_LSR_TEMT_XMITER_NO_EMPTY	(0<<6)
#define IMAPX200_LSR_THRE_MASK				(1<<5)
#define IMAPX200_LSR_THRE_TX_HOLD_EMPTY		(1<<5)
#define IMAPX200_LSR_THRE_TX_HOLD_NO_EMPTY	(0<<5)
#define IMAPX200_LSR_BI_MASK					(1<<4)
#define IMAPX200_LSR_BI_Break_INT				(1<<4)
#define IMAPX200_LSR_BI_NO_Break_INT			(0<<4)
#define IMAPX200_LSR_FE_MASK					(1<<3)
#define IMAPX200_LSR_FE_FRAME_ERR				(1<<3)
#define IMAPX200_LSR_FE_NO_FRAME_ERR			(0<<3)
#define IMAPX200_LSR_PE_MASK					(1<<2)
#define IMAPX200_LSR_PE_PARITY_ERR			(1<<2)
#define IMAPX200_LSR_PE_NO_PARITY_ERR		(0<<2)
#define IMAPX200_LSR_OE_MASK					(1<<1)
#define IMAPX200_LSR_OE_OVERRUN_ERR			(1<<1)
#define IMAPX200_LSR_OE_NO_OVERRUN_ERR		(0<<1)
#define IMAPX200_LSR_DR_MASK					(1<<0)
#define IMAPX200_LSR_DR_DATA_READY			(1<<0)
#define IMAPX200_LSR_DR_DATA_NOT_READY		(0<<0)
#define IMAPX200_LSR_ANY	(IMAPX200_LSR_BI_Break_INT |				\
	IMAPX200_LSR_FE_NO_FRAME_ERR | IMAPX200_LSR_PE_PARITY_ERR |		\
	IMAPX200_LSR_OE_OVERRUN_ERR)

#define IMAPX200_CSR_MASK			(1<<4)
#define IMAPX200_CSR_CTS_ASSERT	(1<<4)
#define IMAPX200_CSR_CTS_DEASSERT	(0<<4)

#define IMAPX200_LPDLL_LOW_POWER_DIVISOR_LOW_BYTE(x)	(((x)&0xf)<<0)
#define IMAPX200_LPDLH_LOW_POWER_DIVISOR_HIGH_BYTE(x)	(((x)&0xf)<<0)

#define IMAPX200_USR_RFF_MASK						(1<<4)
#define IMAPX200_USR_RFF_RX_FIFO_FULL			(1<<4)
#define IMAPX200_USR_RFF_RX_FIFO_NOT_FULL		(0<<4)
#define IMAPX200_USR_RFNE_MASK					(1<<3)
#define IMAPX200_USR_RFNE_RX_FIFO_NOT_EMPTY		(1<<3)
#define IMAPX200_USR_RFNE_RX_FIFO_EMPTY			(0<<3)
#define IMAPX200_USR_TFE_MASK						(1<<2)
#define IMAPX200_USR_TFE_TX_FIFO_EMPTY			(1<<2)
#define IMAPX200_USR_TFE_TX_FIFO_NOT_EMPTY		(0<<2)
#define IMAPX200_USR_TFNF_MASK					(1<<1)
#define IMAPX200_USR_TFNF_TX_FIFO_NOT_FULL		(1<<1)
#define IMAPX200_USR_TFNF_TX_FIFO_FULL			(0<<1)
#define IMAPX200_USR_BUSY_MASK					(1<<0)
#define IMAPX200_USR_BUSY_UART_BUSY				(1<<0)
#define IMAPX200_USR_BUSY_UART_IDLE				(0<<0)

#define IMAPX200_TFL_TX_FIFO_LEVEL_MASK	(0x3f<<0)
#define IMAPX200_RFL_RX_FIFO_LEVEL_MASK	(0x3f<<0)

#define IMAPX200_HTX_HALT_TX_ENABLE	(1<<0)
#define IMAPX200_HTX_HALT_TX_DISABLE	(0<<0)

#define IMAPX200_DMASA_DMA_SOFT_ACK_ENABLE		(1<<0)
#define IMAPX200_DMASA_DMA_SOFT_ACK_DISABLE	(0<<0)

#define IMAPX200_CKSR_CKSEL_MASK		(0x3<<0)
#define IMAPX200_CKSR_CKSEL_PCLK		(0x0<<0)
#define IMAPX200_CKSR_CKSEL_UEXTCLK	(0x1<<0)

#define UART_RX_INT			(1<<0)
#define UART_TX_INT			(1<<1)
#define UART_ERR_INT		(1<<2)
#define UART_THRE_INT		(1<<7)

#ifndef __ASSEMBLY__

/* struct imapx200_uart_clksrc
 *
 * this structure defines a named clock source that can be used for the uart,
 * so that the best clock can be selected for the requested baud rate.
 *
 * min_baud and max_baud define the range of baud-rates this clock is
 * acceptable for, if they are both zero, it is assumed any baud rate that
 * can be generated from this clock will be used.
 *
 * divisor gives the divisor from the clock to the one seen by the uart
*/
#define NR_PORTS 			4
#define UART_FIFO_SIZE		64
#define UART_HAS_INTMSK
#define UART_C_CFLAG
#define UART_UMCON
#define UART_CLK			115200

struct imapx200_uart_clksrc {
	const char	*name;
	unsigned int 	divisor;
	unsigned int	min_baud;
	unsigned int	max_baud;
};

/* configuration structure for per-machine configurations for the serial port
 *
 * the pointer is setup by the machine specific initialisation from the
 * arch/arm/mach-imapx200/ directory.
*/
struct imapx200_uartcfg {
	unsigned char		hwport;	 	/* hardware port number */
	unsigned char	   	unused;
	unsigned short	flags;
	upf_t		   	uart_flags;	/* default uart flags */

	unsigned long	   	fcr;	 		/* value of fifo control register for port */
	unsigned long	   	lcr;	 		/* value of line control register for port */
	unsigned long	   	mcr;	 	/* value of mode control register for port */

	struct imapx200_uart_clksrc *clocks;
	unsigned int		clocks_size;
};

/* imapx200_uart_devs
 *
 * this is exported from the core as we cannot use driver_register(),
 * or platform_add_device() before the console_initcall()
*/

extern struct platform_device *imap_uart_devs[4];

#endif /* __ASSEMBLY__ */

#endif /* __ASM_ARM_REGS_SERIAL_H */

