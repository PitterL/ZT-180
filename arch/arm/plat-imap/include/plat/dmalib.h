/***************************************************************************** 
** arch/arm/plat-imap/include/plat/regs-dma.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Head file of DMA.
**
** Author:
**     csl <joe_chen@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  12/29/2009  csl
*****************************************************************************/

#ifndef __ASM_ARM_REGS_DMA_ZH_H
#define __ASM_ARM_REGS_DMA_ZH_H

#define DMA_MAX_CHANNELS	8


/*
 * Redefine this macro to handle differences between 32- and 64-bit
 * addressing, big vs. little endian, etc.
 */
#define DW_REG(name)		u32 name; u32 __pad_##name

/* Hardware register definitions. */
struct dw_dma_chan_regs {
	DW_REG(SAR);		/* Source Address Register */
	DW_REG(DAR);		/* Destination Address Register */
	DW_REG(LLP);		/* Linked List Pointer */
	u32	CTL_LO;		/* Control Register Low */
	u32	CTL_HI;		/* Control Register High */
	DW_REG(SSTAT);
	DW_REG(DSTAT);
	DW_REG(SSTATAR);
	DW_REG(DSTATAR);
	u32	CFG_LO;		/* Configuration Register Low */
	u32	CFG_HI;		/* Configuration Register High */
	DW_REG(SGR);
	DW_REG(DSR);
};

struct dw_dma_irq_regs {
	DW_REG(XFER);
	DW_REG(BLOCK);
	DW_REG(SRC_TRAN);
	DW_REG(DST_TRAN);
	DW_REG(ERROR);
};

struct dw_dma_regs {
	/* per-channel registers */
	struct dw_dma_chan_regs	CHAN[DMA_MAX_CHANNELS];

	/* irq handling */
	struct dw_dma_irq_regs	RAW;		/* r */
	struct dw_dma_irq_regs	STATUS;		/* r (raw & mask) */
	struct dw_dma_irq_regs	MASK;		/* rw (set = irq enabled) */
	struct dw_dma_irq_regs	CLEAR;		/* w (ack, affects "raw") */

	DW_REG(STATUS_INT);			/* r */

	/* software handshaking */
	DW_REG(REQ_SRC);
	DW_REG(REQ_DST);
	DW_REG(SGL_REQ_SRC);
	DW_REG(SGL_REQ_DST);
	DW_REG(LAST_SRC);
	DW_REG(LAST_DST);

	/* miscellaneous */
	DW_REG(CFG);
	DW_REG(CH_EN);
	DW_REG(ID);
	DW_REG(TEST);

	/* optional encoded params, 0x3c8..0x3 */
};

/* Bitfields in CTL_LO */
#define DWC_CTLL_INT_EN		(1 << 0)	/* irqs enabled? */
#define DWC_CTLL_DST_WIDTH(n)	((n)<<1)	/* bytes per element */
#define DWC_CTLL_SRC_WIDTH(n)	((n)<<4)
#define DWC_CTLL_DST_INC	(0<<7)		/* DAR update/not */
#define DWC_CTLL_DST_DEC	(1<<7)
#define DWC_CTLL_DST_FIX	(2<<7)
#define DWC_CTLL_SRC_INC	(0<<7)		/* SAR update/not */
#define DWC_CTLL_SRC_DEC	(1<<9)
#define DWC_CTLL_SRC_FIX	(2<<9)
#define DWC_CTLL_DST_MSIZE(n)	((n)<<11)	/* burst, #elements */
#define DWC_CTLL_SRC_MSIZE(n)	((n)<<14)
#define DWC_CTLL_S_GATH_EN	(1 << 17)	/* src gather, !FIX */
#define DWC_CTLL_D_SCAT_EN	(1 << 18)	/* dst scatter, !FIX */
#define DWC_CTLL_FC_MASK	(7 << 20)
#define DWC_CTLL_FC_M2M		(0 << 20)	/* mem-to-mem */
#define DWC_CTLL_FC_M2P		(1 << 20)	/* mem-to-periph */
#define DWC_CTLL_FC_P2M		(2 << 20)	/* periph-to-mem */
#define DWC_CTLL_FC_P2P		(3 << 20)	/* periph-to-periph */
/* plus 4 transfer types for peripheral-as-flow-controller */
#define DWC_CTLL_DMS(n)		((n)<<23)	/* dst master select */
#define DWC_CTLL_SMS(n)		((n)<<25)	/* src master select */
#define DWC_CTLL_LLP_D_EN	(1 << 27)	/* dest block chain */
#define DWC_CTLL_LLP_S_EN	(1 << 28)	/* src block chain */

/* Bitfields in CTL_HI */
#define DWC_CTLH_DONE		0x00001000
#define DWC_CTLH_BLOCK_TS_MASK	0x00000fff

/* Bitfields in CFG_LO. Platform-configurable bits are in <linux/dw_dmac.h> */
#define DWC_CFGL_CH_SUSP	(1 << 8)	/* pause xfer */
#define DWC_CFGL_FIFO_EMPTY	(1 << 9)	/* pause xfer */
#define DWC_CFGL_HS_DST		(1 << 10)	/* handshake w/dst */
#define DWC_CFGL_HS_SRC		(1 << 11)	/* handshake w/src */
#define DWC_CFGL_MAX_BURST(x)	((x) << 20)
#define DWC_CFGL_RELOAD_SAR	(1 << 30)
#define DWC_CFGL_RELOAD_DAR	(1 << 31)

/* Bitfields in CFG_HI. Platform-configurable bits are in <linux/dw_dmac.h> */
#define DWC_CFGH_FCMODE		(1 << 0)
#define DWC_CFGH_FIFO_MODE	(1 << 1)
#define DWC_CFGH_PROTCTL(x)	((x) << 2)
#define DWC_CFGH_DS_UPD_EN	(1 << 5)
#define DWC_CFGH_SS_UPD_EN	(1 << 6)
#define DWC_CFGH_SRC_PER(x)	((x) << 7)
#define DWC_CFGH_DST_PER(x)	((x) << 11)


/* Bitfields in SGR */
#define DWC_SGR_SGI(x)		((x) << 0)
#define DWC_SGR_SGC(x)		((x) << 20)

/* Bitfields in DSR */
#define DWC_DSR_DSI(x)		((x) << 0)
#define DWC_DSR_DSC(x)		((x) << 20)

/* Bitfields in CFG */
#define DW_CFG_DMA_EN		(1 << 0)

#define DW_REGLEN		0x400
#define DW_CH_STRIDE    0x58
/* LLI == Linked List Item; a.k.a. DMA block descriptor */
struct dma_lli {
	/* values that are not changed by hardware */
	dma_addr_t	sar;
	dma_addr_t	dar;
	dma_addr_t	llp;		/* chain to next lli */
	u32		ctllo;
	/* values that may get written back: */
	u32		ctlhi;
	/* sstat and dstat can snapshot peripheral register state.
	 * silicon config may discard either or both...
	 */
	u32		sstat;
	u32		dstat;
	//u32		next_lli;
};

#define FC_DMA_M2M 0
#define FC_DMA_M2P 1
#define FC_DMA_P2M 2
#define FC_DMA_P2P 3
#define FC_PHI_P2M 4
#define FC_SPHI_P2P 5
#define FC_PHI_M2P 6
#define FC_DPHI_P2P 7


#define TRANS_TYPE_SINGLE 1
#define TRANS_TYPE_LLI 2
#define TRANS_TYPE_AUTO 4

//DMA Handshake type
#define HANDSHAKE_NO 0
#define HANDSHAKE_HARD 1
#define HANDSHAKE_SOFR 2

#define INCRE_CONST 0
#define INCRE_ADD 1 
#define INCRE_SUB 2

//u32 imapx200_dma_init_lli(struct imapx200_dma_chan *chan);
//DMA_CTL
//-----------------------------------------------------------------------------
#define DMA_CTL_INT_EN					0		// Interrupt Enable Bit
#define DMA_CTL_DST_TR_WIDTH			1		// Destination Transfer Width
#define DMA_CTL_SRC_TR_WIDTH			4		// Source Transfer Width
#define DMA_CTL_DINC					7		// Destination Address Increment
#define DMA_CTL_SINC					9		// Source Address Increment
#define DMA_CTL_DEST_MSIZE			11		// Destination Burst Transaction Length
#define DMA_CTL_SRC_MSIZE				14		// Source Burst Transaction Length
#define DMA_CTL_SRC_GATHER_EN			17		// Source gather enable bit
#define DMA_CTL_DST_SCATTER_EN		18		// Destination scatter enable 
#define DMA_CTL_TT_FC					20		// Transfer Type and Flow Control
#define DMA_CTL_DMS					23		// Destination Master Select 
#define DMA_CTL_SMS					25		// Source Master Select
#define DMA_CTL_LLP_DST_EN				27		// Block chaining is enabled on the destination
#define DMA_CTL_LLP_SRC_EN				28		// Block chaining is enabled on the source side
#define DMA_CTL_BLOCK_TS				32		// Block Transfer Size
#define DMA_CTL_DONE					44		// Done bit
//-----------------------------------------------------------------------------
#define DMA_CFG_CH_PRIOR				5		// Channel priority
#define DMA_CFG_CH_SUSP				8		// Channel Suspend.
#define DMA_CFG_FIFO_EMPTY			9		// Indicates if there is data left in the channel FIFO
#define DMA_CFG_HS_SEL_DST			10		// Destination Software or Hardware Handshaking Select
#define DMA_CFG_HS_SEL_SRC			11		// Source Software or Hardware Handshaking Select
#define DMA_CFG_LOCK_CH_L				12		// Channel Lock Level
#define DMA_CFG_LOCK_B_L				14		// Bus Lock Level
#define DMA_CFG_LOCK_CH				16		// Channel Lock Bit
#define DMA_CFG_LOCK_B					17		// Bus Lock Bit
#define DMA_CFG_DST_HS_POL			18		// Destination Handshaking Interface Polarity
#define DMA_CFG_SRC_HS_POL			19		// Source Handshaking Interface Polarity
#define DMA_CFG_MAX_ABRST				20		// Maximum AMBA Burst Length
#define DMA_CFG_RELOAD_SRC			30		// Automatic Source Reload
#define DMA_CFG_RELOAD_DST			31		// Automatic Destination Reload
#define DMA_CFG_FCMODE				0		// Flow Control Mode
#define DMA_CFG_FIFO_MODE				1		// FIFO Mode Select
#define DMA_CFG_PROTCTL				2		// Protection Control
#define DMA_CFG_DS_UPD_EN				5		// Destination Status Update Enable
#define DMA_CFG_SS_UPD_EN				6		// Source Status Update Enable
#define DMA_CFG_SRC_PER				7		// Assigns a hardware handshaking interface to the source of channel x
#define DMA_CFG_DEST_PER				11		// Assigns a hardware handshaking interface to the destination of channel x
//DMA-SCATTER
#define DMA_SGR_SGI					0		// Source gather count
#define DMA_SGR_SGC					20		// Source gather interval.
//DMA-GATHER
#define DMA_DSR_DSI					0		// Destination scatter count
#define DMA_DSR_DSC					20		// Destination scatter interval

#define HANDSHAKE_NO 0
#define HANDSHAKE_HARD 1
#define HANDSHAKE_SOFT 2

//MULTI-BLOCK type
#define TRANS_TYPE_SINGLE	1
#define TRANS_TYPE_MULTI	2
//-------------------------------
#define RELOAD_LLI_SRC		0x1
#define RELOAD_LLI_DST		0x2
#define RELOAD_SRC			0x4
#define RELOAD_DST			0x8


// 2D
#define ENABLE_SRC_2D		0x1
#define ENABLE_DST_2D		0x2
//-------------------------------

//WRITE-BACK
#define WRITEBACK_SRC		0x1
#define WRITEBACK_DST		0x2


//MASTER0/1
#define M1_BASELINE			0x40000000

//DMA_LLP
//-----------------------------------------------------------------------------
#define DMA_LLP_LOC						2		// Starting Address In Memory
#define DMA_LLP_LMS						0		// List Master Select
#define HANDSHAKE_INDEX_UART0_RX		0
#define HANDSHAKE_INDEX_UART1_RX		1
#define HANDSHAKE_INDEX_UART2_RX		2
#define HANDSHAKE_INDEX_UART0_TX		3
#define HANDSHAKE_INDEX_UART1_TX		4
#define HANDSHAKE_INDEX_UART2_TX		5
#define HANDSHAKE_INDEX_IIS0_TX		6
#define HANDSHAKE_INDEX_IIS0_RX		7	
#define HANDSHAKE_INDEX_UART3_RX		8
#define HANDSHAKE_INDEX_UART3_TX		9
#define HANDSHAKE_INDEX_EXT0_REQ		10
#define HANDSHAKE_INDEX_EXT1_REQ		11
#define HANDSHAKE_INDEX_PWM			14	
//DMA_STATUSInt
#define STATUSINT_TFRE				1<<0
#define STATUSINT_BLK				1<<1 
#define STATUSINT_SRCT				1<<2
#define STATUSINT_DSTT				1<<3 
#define STATUSINT_ERR				1<<4 


#endif

