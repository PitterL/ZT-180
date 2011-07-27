#ifndef __IMAPX_UART__
#define __IMAPX_UART__

#define IMAP_VA_UART0      (IMAP_VA_UART)
#define IMAP_VA_UART1      (IMAP_VA_UART + 0x1000)
#define IMAP_VA_UART2      (IMAP_VA_UART + 0x2000)
#define IMAP_VA_UART3      (IMAP_VA_UART + 0x3000)


#define rUARTRBR0_THR_DLL    		 (IMAP_VA_UART0+0x000 )			// receive buffer register
// receive buffer register
// divisor latch low          (0x00)
#define rUART0_IER_DLH    		 (IMAP_VA_UART0+0x004 )			// interrupt enable register               
// divisor latch high         (0x04)
#define rUART0_IIR_FCR    		 (IMAP_VA_UART0+0x008 )			// interrupt identity register
// FIFO control register      (0x08)
#define rUART0_LCR			 (IMAP_VA_UART0+0x00C )			// line control register      (0x0c)
#define rUART0_MCR    			 (IMAP_VA_UART0+0x010 )     		// modem control register     (0x10)
#define rUART0_LSR    			 (IMAP_VA_UART0+0x014 )   			// line status register       (0x14)
#define rUART0_MSR    			 (IMAP_VA_UART0+0x018 )     		// modem status register      (0x18)
#define rUART0_SCR    			 (IMAP_VA_UART0+0x01C )    		 // scratch register           (0x1c)
#define rUART0_LPDLL    		 (IMAP_VA_UART0+0x020 )
#define rUART0_LPDLH    		 (IMAP_VA_UART0+0x024 )

#define rUART0_FAR    			 (IMAP_VA_UART0+0x070 )			// FIFO access register       (0x70)
#define rUART0_TFR    			 (IMAP_VA_UART0+0x074 )        		// transmit FIFO read         (0x74)
#define rUART0_RFW    			 (IMAP_VA_UART0+0x078 )        		// receiver FIFO write        (0x78)
#define rUART0_USR    			 (IMAP_VA_UART0+0x07C )        		// uart status register       (0x7c)
#define rUART0_TFL    			 (IMAP_VA_UART0+0x080 )     	 	// transmit FIFO level        (0x80)
#define rUART0_RFL    			 (IMAP_VA_UART0+0x084 )      		// receive FIFO level         (0x84)
#define rUART0_SRR    			 (IMAP_VA_UART0+0x088 )        		// software reset register    (0x88)
#define rUART0_SRTS    			 (IMAP_VA_UART0+0x08C )     		// shadow request to send     (0x8c) 
#define rUART0_SBCR    			 (IMAP_VA_UART0+0x090 )    		 // shadow break control       (0x90) 
#define rUART0_SDMAM    		 (IMAP_VA_UART0+0x094 )    		// shadow dma mode            (0x94)  
#define rUART0_SFE    			 (IMAP_VA_UART0+0x098 )        		// shadow FIFO enable         (0x98)
#define rUART0_SRT    			 (IMAP_VA_UART0+0x09C )        		// shadow receiver trigger    (0x9c)
#define rUART0_STET    			 (IMAP_VA_UART0+0x0A0 )     		// shadow transmitter trigger (0xa0) 
#define rUART0_HTX    			 (IMAP_VA_UART0+0x0A4 )        		// halt Tx                    (0xa4)
#define rUART0_DMASA   			 (IMAP_VA_UART0+0x0A8 )    		// dma software acknowledge   (0xa8)  
#define rUART0_COMP_PARAM_1		 (IMAP_VA_UART0+0x0F4 )			// component parameters       (0xf4)
#define rUART0_COMP_VERSION	 	 (IMAP_VA_UART0+0x0F8 )     		// component version          (0xf8)
#define rUART0_COMP_TYPE   		 (IMAP_VA_UART0+0x0FC )   			// component type             (0xfc)
#define rUART0_CLKSEL			 (IMAP_VA_UART0+0x100 )
//UART1
#define rUARTRBR1_THR_DLL    		 (IMAP_VA_UART1+0x000 )			// receive buffer register
// receive buffer register
// divisor latch low          (0x00)
#define rUART1_IER_DLH    		 (IMAP_VA_UART1+0x004 )			// interrupt enable register               
// divisor latch high         (0x04)
#define rUART1_IIR_FCR    		 (IMAP_VA_UART1+0x008 )			// interrupt identity register
// FIFO control register      (0x08)
#define rUART1_LCR			 (IMAP_VA_UART1+0x00C )			// line control register      (0x0c)
#define rUART1_MCR    			 (IMAP_VA_UART1+0x010 )     		// modem control register     (0x10)
#define rUART1_LSR    			 (IMAP_VA_UART1+0x014 )   			// line status register       (0x14)
#define rUART1_MSR    			 (IMAP_VA_UART1+0x018 )     		// modem status register      (0x18)
#define rUART1_SCR    			 (IMAP_VA_UART1+0x01C )    		 // scratch register           (0x1c)
#define rUART1_LPDLL    		 (IMAP_VA_UART1+0x020 )
#define rUART1_LPDLH    		 (IMAP_VA_UART1+0x024 )

#define rUART1_FAR    			 (IMAP_VA_UART1+0x070 )			// FIFO access register       (0x70)
#define rUART1_TFR    			 (IMAP_VA_UART1+0x074 )        		// transmit FIFO read         (0x74)
#define rUART1_RFW    			 (IMAP_VA_UART1+0x078 )        		// receiver FIFO write        (0x78)
#define rUART1_USR    			 (IMAP_VA_UART1+0x07C )        		// uart status register       (0x7c)
#define rUART1_TFL    			 (IMAP_VA_UART1+0x080 )     	 	// transmit FIFO level        (0x80)
#define rUART1_RFL    			 (IMAP_VA_UART1+0x084 )      		// receive FIFO level         (0x84)
#define rUART1_SRR    			 (IMAP_VA_UART1+0x088 )        		// software reset register    (0x88)
#define rUART1_SRTS    			 (IMAP_VA_UART1+0x08C )     		// shadow request to send     (0x8c) 
#define rUART1_SBCR    			 (IMAP_VA_UART1+0x090 )    		 // shadow break control       (0x90) 
#define rUART1_SDMAM    		 (IMAP_VA_UART1+0x094 )    		// shadow dma mode            (0x94)  
#define rUART1_SFE    			 (IMAP_VA_UART1+0x098 )        		// shadow FIFO enable         (0x98)
#define rUART1_SRT    			 (IMAP_VA_UART1+0x09C )        		// shadow receiver trigger    (0x9c)
#define rUART1_STET    			 (IMAP_VA_UART1+0x0A0 )     		// shadow transmitter trigger (0xa0) 
#define rUART1_HTX    			 (IMAP_VA_UART1+0x0A4 )        		// halt Tx                    (0xa4)
#define rUART1_DMASA   			 (IMAP_VA_UART1+0x0A8 )    		// dma software acknowledge   (0xa8)  
#define rUART1_COMP_PARAM_1	 	 (IMAP_VA_UART1+0x0F4 )			// component parameters       (0xf4)
#define rUART1_COMP_VERSION	 	 (IMAP_VA_UART1+0x0F8 )     		// component version          (0xf8)
#define rUART1_COMP_TYPE   		 (IMAP_VA_UART1+0x0FC )   			// component type             (0xfc)
#define rUART1_CLKSEL			 (IMAP_VA_UART1+0x100 )
//UART2
#define rUARTRBR2_THR_DLL    		 (IMAP_VA_UART2+0x000 )			// receive buffer register
// receive buffer register
// divisor latch low          (0x00)
#define rUART2_IER_DLH    		 (IMAP_VA_UART2+0x004 )			// interrupt enable register               
// divisor latch high         (0x04)
#define rUART2_IIR_FCR    		 (IMAP_VA_UART2+0x008 )			// interrupt identity register
// FIFO control register      (0x08)
#define rUART2_LCR			 (IMAP_VA_UART2+0x00C )			// line control register      (0x0c)
#define rUART2_MCR    			 (IMAP_VA_UART2+0x010 )     		// modem control register     (0x10)
#define rUART2_LSR    			 (IMAP_VA_UART2+0x014 )   			// line status register       (0x14)
#define rUART2_MSR    			 (IMAP_VA_UART2+0x018 )     		// modem status register      (0x18)
#define rUART2_SCR    			 (IMAP_VA_UART2+0x01C )    		 // scratch register           (0x1c)
#define rUART2_LPDLL    		 (IMAP_VA_UART2+0x020 )
#define rUART2_LPDLH    		 (IMAP_VA_UART2+0x024 )

#define rUART2_FAR    			 (IMAP_VA_UART2+0x070 )			// FIFO access register       (0x70)
#define rUART2_TFR    			 (IMAP_VA_UART2+0x074 )        		// transmit FIFO read         (0x74)
#define rUART2_RFW    			 (IMAP_VA_UART2+0x078 )        		// receiver FIFO write        (0x78)
#define rUART2_USR    			 (IMAP_VA_UART2+0x07C )        		// uart status register       (0x7c)
#define rUART2_TFL    			 (IMAP_VA_UART2+0x080 )     	 	// transmit FIFO level        (0x80)
#define rUART2_RFL    			 (IMAP_VA_UART2+0x084 )      		// receive FIFO level         (0x84)
#define rUART2_SRR    			 (IMAP_VA_UART2+0x088 )        		// software reset register    (0x88)
#define rUART2_SRTS    			 (IMAP_VA_UART2+0x08C )     		// shadow request to send     (0x8c) 
#define rUART2_SBCR    			 (IMAP_VA_UART2+0x090 )    		 // shadow break control       (0x90) 
#define rUART2_SDMAM    		 (IMAP_VA_UART2+0x094 )    		// shadow dma mode            (0x94)  
#define rUART2_SFE    			 (IMAP_VA_UART2+0x098 )        		// shadow FIFO enable         (0x98)
#define rUART2_SRT    			 (IMAP_VA_UART2+0x09C )        		// shadow receiver trigger    (0x9c)
#define rUART2_STET    			 (IMAP_VA_UART2+0x0A0 )     		// shadow transmitter trigger (0xa0) 
#define rUART2_HTX    			 (IMAP_VA_UART2+0x0A4 )        		// halt Tx                    (0xa4)
#define rUART2_DMASA   			 (IMAP_VA_UART2+0x0A8 )    		// dma software acknowledge   (0xa8)  
#define rUART2_COMP_PARAM_1	 	 (IMAP_VA_UART2+0x0F4 )			// component parameters       (0xf4)
#define rUART2_COMP_VERSION	 	 (IMAP_VA_UART2+0x0F8 )     		// component version          (0xf8)
#define rUART2_COMP_TYPE   		 (IMAP_VA_UART2+0x0FC )   			// component type             (0xfc)
#define rUART2_CLKSEL			 (IMAP_VA_UART2+0x100 )

//UART3
#define rUARTRBR3_THR_DLL    		 (IMAP_VA_UART3+0x000 )		// receive buffer register
// receive buffer register
// divisor latch low          (0x00)
#define rUART3_IER_DLH    		 (IMAP_VA_UART3+0x004 )			// interrupt enable register               
// divisor latch high         (0x04)
#define rUART3_IIR_FCR    		 (IMAP_VA_UART3+0x008 )			// interrupt identity register
// FIFO control register      (0x08)
#define rUART3_LCR			 (IMAP_VA_UART3+0x00C )			// line control register      (0x0c)
#define rUART3_MCR    			 (IMAP_VA_UART3+0x010 )     		// modem control register     (0x10)
#define rUART3_LSR    			 (IMAP_VA_UART3+0x014 )   			// line status register       (0x14)
#define rUART3_MSR    			 (IMAP_VA_UART3+0x018 )     		// modem status register      (0x18)
#define rUART3_SCR    			 (IMAP_VA_UART3+0x01C )    		 // scratch register           (0x1c)
#define rUART3_LPDLL    		 (IMAP_VA_UART3+0x020 )
#define rUART3_LPDLH    		 (IMAP_VA_UART3+0x024 )

#define rUART3_FAR    			 (IMAP_VA_UART3+0x070 )			// FIFO access register       (0x70)
#define rUART3_TFR    			 (IMAP_VA_UART3+0x074 )        		// transmit FIFO read         (0x74)
#define rUART3_RFW    			 (IMAP_VA_UART3+0x078 )        		// receiver FIFO write        (0x78)
#define rUART3_USR    			 (IMAP_VA_UART3+0x07C )        		// uart status register       (0x7c)
#define rUART3_TFL    			 (IMAP_VA_UART3+0x080 )     	 	// transmit FIFO level        (0x80)
#define rUART3_RFL    			 (IMAP_VA_UART3+0x084 )      		// receive FIFO level         (0x84)
#define rUART3_SRR    			 (IMAP_VA_UART3+0x088 )        		// software reset register    (0x88)
#define rUART3_SRTS    			 (IMAP_VA_UART3+0x08C )     		// shadow request to send     (0x8c) 
#define rUART3_SBCR    			 (IMAP_VA_UART3+0x090 )    		 // shadow break control       (0x90) 
#define rUART3_SDMAM    		 (IMAP_VA_UART3+0x094 )    		// shadow dma mode            (0x94)  
#define rUART3_SFE    			 (IMAP_VA_UART3+0x098 )        		// shadow FIFO enable         (0x98)
#define rUART3_SRT    			 (IMAP_VA_UART3+0x09C )        		// shadow receiver trigger    (0x9c)
#define rUART3_STET    			 (IMAP_VA_UART3+0x0A0 )     		// shadow transmitter trigger (0xa0) 
#define rUART3_HTX    			 (IMAP_VA_UART3+0x0A4 )        		// halt Tx                    (0xa4)
#define rUART3_DMASA   			 (IMAP_VA_UART3+0x0A8 )    		// dma software acknowledge   (0xa8)  
#define rUART3_COMP_PARAM_1	 	 (IMAP_VA_UART3+0x0F4 )			// component parameters       (0xf4)
#define rUART3_COMP_VERSION	 	 (IMAP_VA_UART3+0x0F8 )     		// component version          (0xf8)
#define rUART3_COMP_TYPE   		 (IMAP_VA_UART3+0x0FC )   			// component type             (0xfc)
#define rUART3_CLKSEL			 (IMAP_VA_UART3+0x100 )


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

#endif /* __ASSEMBLY__ */

#endif
