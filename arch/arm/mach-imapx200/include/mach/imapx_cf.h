#ifndef __IMAPX_CF__
#define __IMAPX_CF__

#define rCF_DEV_PIO_DATA    		 (0x00 )
#define rCF_DEV_ERR_FUNC    		 (0x04 )
#define rCF_DEV_SEC_CNT     		 (0x08 )
#define rCF_DEV_SEC_NO      		 (0x0C )
#define rCF_DEV_CYL_LOW     		 (0x10 ) 
#define rCF_DEV_CYL_HIGH    		 (0x14 )
#define rCF_DEV_HEAD_DRV    		 (0x18 )
#define rCF_DEV_CMD_STAT    		 (0x1C )
#define rCF_DEV_ALT_STAT    		 (0x20 )

#define rCF_HOST_STAT_FORCE    	 	 (0x28 )
#define rCF_HOST_CON        		 (0x2c )

#define rCF_HOST_INT_UNMASK    	 	 (0x30 )
#define rCF_HOST_STAT_EN    		 (0x34 )
#define rCF_HOST_STAT	  		 (0x38 )
#define rCF_PIN_STATUS    		 (0x3c )
#define rCF_PIO_TPARAM0		 	 (0x40 )
#define rCF_PIO_TPARAM1    		 (0x44 )
#define rCF_PIO_TPARAM2    		 (0x48 )

#define rCF_UDMA_TPARAM0     	 	 (0x4C )
#define rCF_UDMA_TPARAM1     	 	 (0x50 )
#define rCF_UDMA_TPARAM2     	 	 (0x54 )
#define rCF_UDMA_TPARAM3     	 	 (0x58 )
#define rCF_UDMA_TPARAM4     	 	 (0x5C )
#define rCF_UDMA_TPARAM5     	 	 (0x60 )
#define rCF_UDMA_TRIGGER     	 	 (0x64 )
#define rCF_UDMA_XFER_INFO     	 	 (0x68 )
#define rCF_UDMA_FIFO_STATUS    	 (0x6C )
#define rCF_UDMA_HTBSDT_CNT      	 (0x70 )

#define rCF_UDMA_BSTI_HP_CNT      	 (0x74 )
#define rCF_UDMA_BSTO_HP_CNT      	 (0x78 )
#define rCF_UDMA_BSTO_DP_CNT      	 (0x7C )


#define rCF_DMA_ST_ADDR      		 (0x80 )
#define rCF_DMA_CNTL     		 (0x84 ) 
#define rCF_DMA_ALT_ST_ADDR      	 (0x88 )
#define rCF_DMA_ALT_CNTL    		 (0x8C )
#define rCF_DMA_ALT_ST_ADDR2     	 (0x90 )
#define rCF_DMA_ALT_CNTL2   		 (0x94 )
#define rCF_DMA_ALT_ST_ADDR3     	 (0x98 )
#define rCF_DMA_ALT_CNTL3        	 (0x9C )

#define rCF_ADMA_DESCRIPTOR_POINT   	 (0xA0 )

//rCF_DEV_HEAD_DRV(0x18)
#define	IDE_DEVREG_DEV0  		0x00	// select device 0
#define	IDE_DEVREG_DEV1  		0x10	// select device 1
#define	IDE_DEVREG_B5   		0x20	// reserved, set to 1
#define	IDE_DEVREG_LBA  		0x40	// address by LBA(0 : address by CHS)
#define	IDE_DEVREG_B7   		0x80	// reserved, set to 1

//IDE_HOST_CON(0x2C)
#define CFH_EN      			(1 << 31)
#define UDMA_REG_EN  			(1 << 30)
#define CF_BYTE_ACCESS			(1 << 29)
#define RESET_CF              		(1 << 28)
#define UDMA_FIFO_RST			(1 << 27)
#define UDMA_HTBSTO_CNT_EN   		(1 << 26)
#define UDMA_HTBSTI_CNT_EN   		(1 << 25)
#define UDMA_DAUTOC_CNT_EN   		(1 << 24)
#define UDMA_BSTI_HP_CNT_EN   		(1 << 23)
#define UDMA_BSTO_HP_CNT_EN   		(1 << 22)
#define UDMA_BSTO_DP_CNT_EN   		(1 << 21)
#define	ADMA_MODE_EN			(1 << 1)
#define DEBUG_MODE_EN               	(1 << 0)

//IDE_HOST_INT_UNMASK(0x30)
#define UDMA_OVERFLOW_UNMASK		(1 << 9)
#define UDMA_UNDERFLOW_UNMASK		(1 << 8)
#define UDMA_UNEXPECT_UNMASK		(1 << 7)
#define UDMA_BSTO_UNMASK     		(1 << 6)
#define UDMA_BSTI_UNMASK     		(1 << 5)
#define UDMA_IORDY_UNMASK     		(1 << 4)
#define DMA_UNMASK     			(1 << 3)
#define CARD_REMOVE_UNMASK     		(1 << 2)
#define CARD_INSERT_UNMASK     		(1 << 1)
#define INTRQ_SYN_UNMASK     		(1 << 0)

//IDE_HOST_STAT_EN(0x34)
#define UDMA_OVERFLOW_STS_EN     	(1 << 9)
#define UDMA_UNDERFLOW_STS_EN     	(1 << 8)
#define UDMA_UNEXPECT_STS_EN     	(1 << 7)
#define UDMA_BSTO_STS_EN     		(1 << 6)
#define UDMA_BSTI_STS_EN     		(1 << 5)
#define PIO_IORDY_STS_EN     		(1 << 4)
#define DMA_STS_EN     			(1 << 3)
#define CARD_REMOVE_STS_EN     		(1 << 2)
#define CARD_INSERT_STS_EN     		(1 << 1)

//IDE_HOST_STAT(0x38)
#define INTRQ_SYN			(1 << 0)

//rCF_UDMA_TRIGGER(0x64)
#define UDMA_START			(1 << 0)

//rCF_UDMA_XFER_INFO(0x68)
#define UDMA_BURST_IN			(1 << 31)
#define UDMA_BURST_OUT			(0 << 31)

//rCF_DMA_CNTL(0x84)
#define SDMA_EN				(1 << 31)
#define BUF_RST				(1 << 30)
#define ALT_EN				(1 << 27)
#define AUTO_EN				(1 << 26)
#define TRANS_RD			(1 << 25)
#define TRANS_WR			(0 << 25)

#endif
