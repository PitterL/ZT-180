#ifndef __IMAPX_IIC__
#define __IMAPX_IIC__

#define rIC_CON              		 (0x00 )     //IIC Control register
#define rIC_TAR              		 (0x04 )     //IIC Targer address register
#define rIC_SAR              		 (0x08 )     //IIC Slave address register
#define rIC_HS_MADDR         		 (0x0C )     //IIC High speed master mode code address register
#define rIC_DATA_CMD         		 (0x10 )     //IIC Rx/Tx data buffer and command register
#define rIC_SS_SCL_HCNT      		 (0x14 )     //Standard speed IIC clock SCL high count register
#define rIC_SS_SCL_LCNT      		 (0x18 )     //Standard speed IIC clock SCL low count register
#define rIC_FS_SCL_HCNT      		 (0x1C )     //Fast speed IIC clock SCL high count register
#define rIC_FS_SCL_LCNT      		 (0x20 )     //Fast speed IIC clock SCL low count register
#define rIC_HS_SCL_HCNT      		 (0x24 )     //High speed IIC clock SCL high count register
#define rIC_HS_SCL_LCNT      		 (0x28 )     //High speed IIC clock SCL low count register
#define rIC_INTR_STAT        		 (0x2C )     //IIC Interrupt Status register
#define rIC_INTR_MASK        		 (0x30 )     //IIC Interrupt Mask register
#define rIC_RAW_INTR_STAT    		 (0x34 )     //IIC raw interrupt status register
#define rIC_RX_TL            		 (0x38 )     //IIC receive FIFO Threshold register
#define rIC_TX_TL            		 (0x3C )     //IIC transmit FIFO Threshold register
#define rIC_CLR_INTR         		 (0x40 )     //clear combined and individual interrupt register
#define rIC_CLR_RX_UNDER     		 (0x44 )     //clear RX_UNDER interrupt register
#define rIC_CLR_RX_OVER      		 (0x48 )     //clear RX_OVER interrupt register
#define rIC_CLR_TX_OVER      		 (0x4C )     //clear TX_OVER interrupt register
#define rIC_CLR_RD_REQ       		 (0x50 )     //clear RD_REQ interrupt register
#define rIC_CLR_TX_ABRT      		 (0x54 )     //clear TX_ABRT interrupt register
#define rIC_CLR_RX_DONE      		 (0x58 )     //clear RX_DONE interrupt register
#define rIC_CLR_ACTIVITY     		 (0x5C )     //clear ACTIVITY interrupt register
#define rIC_CLR_STOP_DET     		 (0x60 )     //clear STOP_DET interrupt register
#define rIC_CLR_START_DET    		 (0x64 )     //clear START_DET interrupt register
#define rIC_CLR_GEN_CALL     		 (0x68 )     //clear GEN_CALL interrupt register
#define rIC_ENABLE           		 (0x6C )     //IIC enable register
#define rIC_STATUS           		 (0x70 )     //IIC status register
#define rIC_TXFLR            		 (0x74 )     //IIC transmit FIFO level register
#define rIC_RXFLR            		 (0x78 )     //IIC receive FIFO level register
#define rIC_TX_ABRT_SOURCE   		 (0x80 )     //IIC transmit abort source register
#define rIC_SLV_DATA_NACK_ONLY	 	 (0x84 )     //generate slave data NACK register
#define rIC_DMA_CR           		 (0x88 )     //DMA control register
#define rIC_DMA_TDLR         		 (0x8C )     //DAM transmit data level register
#define rIC_DMA_RDLR         		 (0x90 )     //IIC receive data level register
#define rIC_SDA_SETUP        		 (0x94 )     //IIC SDA setup register
#define rIC_ACK_GENERAL_CALL 		 (0x98 )     //IIC ACK general call register
#define rIC_ENABLE_STATUS    		 (0x9C )     //IIC enable status register
#define rIC_IGNORE_ACK0    		 (0xA0 )     //IIC enable status register
#define rIC_COMP_PARAM_1     		 (0xF4 )     //component parameter register 1
#define rIC_COMP_VERSION     		 (0xF8 )     //IIC component version register
#define rIC_COMP_TYPE        		 (0xFC )     //IIC component type register


//========================================================================
// IIC1
//========================================================================
#define rIC_CON1              		 (0x00 )     //IIC Control register
#define rIC_TAR1              		 (0x04 )     //IIC Targer address register
#define rIC_SAR1              		 (0x08 )     //IIC Slave address register
#define rIC_HS_MADDR1         		 (0x0C )     //IIC High speed master mode code address register
#define rIC_DATA_CMD1         		 (0x10 )     //IIC Rx/Tx data buffer and command register
#define rIC_SS_SCL_HCNT1      		 (0x14 )     //Standard speed IIC clock SCL high count register
#define rIC_SS_SCL_LCNT1      		 (0x18 )     //Standard speed IIC clock SCL low count register
#define rIC_FS_SCL_HCNT1      		 (0x1C )     //Fast speed IIC clock SCL high count register
#define rIC_FS_SCL_LCNT1      		 (0x20 )     //Fast speed IIC clock SCL low count register
#define rIC_HS_SCL_HCNT1      		 (0x24 )     //High speed IIC clock SCL high count register
#define rIC_HS_SCL_LCNT1      		 (0x28 )     //High speed IIC clock SCL low count register
#define rIC_INTR_STAT1        		 (0x2C )     //IIC Interrupt Status register
#define rIC_INTR_MASK1        		 (0x30 )     //IIC Interrupt Mask register
#define rIC_RAW_INTR_STAT1    		 (0x34 )     //IIC raw interrupt status register
#define rIC_RX_TL1            		 (0x38 )     //IIC receive FIFO Threshold register
#define rIC_TX_TL1            		 (0x3C )     //IIC transmit FIFO Threshold register
#define rIC_CLR_INTR1         		 (0x40 )     //clear combined and individual interrupt register
#define rIC_CLR_RX_UNDER1     		 (0x44 )     //clear RX_UNDER interrupt register
#define rIC_CLR_RX_OVER1      		 (0x48 )     //clear RX_OVER interrupt register
#define rIC_CLR_TX_OVER1      		 (0x4C )     //clear TX_OVER interrupt register
#define rIC_CLR_RD_REQ1       		 (0x50 )     //clear RD_REQ interrupt register
#define rIC_CLR_TX_ABRT1      		 (0x54 )     //clear TX_ABRT interrupt register
#define rIC_CLR_RX_DONE1      		 (0x58 )     //clear RX_DONE interrupt register
#define rIC_CLR_ACTIVITY1     		 (0x5C )     //clear ACTIVITY interrupt register
#define rIC_CLR_STOP_DET1     		 (0x60 )     //clear STOP_DET interrupt register
#define rIC_CLR_START_DET1    		 (0x64 )     //clear START_DET interrupt register
#define rIC_CLR_GEN_CALL1     		 (0x68 )     //clear GEN_CALL interrupt register
#define rIC_ENABLE1           		 (0x6C )     //IIC enable register
#define rIC_STATUS1           		 (0x70 )     //IIC status register
#define rIC_TXFLR1            		 (0x74 )     //IIC transmit FIFO level register
#define rIC_RXFLR1            		 (0x78 )     //IIC receive FIFO level register
#define rIC_TX_ABRT_SOURCE1   		 (0x80 )     //IIC transmit abort source register

#define rIC_ACK_GENERAL_CALL0 	 (0x98 )     //I2C channel 0 ACK General Call Register
#define rIC_ENABLE_STATUS    		 (0x9C )    //I2C channel 0 Enable Status Register
#define rIC_SDA_CFG0    		 	 (0xA0 )     //I2C channel 0 SDA Configuration Register


#define rIC_COMP_PARAM_11     		 (0xF4 )     //component parameter register 1
#define rIC_COMP_VERSION1     		 (0xF8 )     //IIC component version register
#define rIC_COMP_TYPE1        		 (0xFC )     //IIC component type register


//Bit Control
#define IMAPX200_IIC_ENABLE			(0x1)
#define IMAPX200_IIC_DEFAULT_SS_SCL_HCNT	(0xc0)
#define IMAPX200_IIC_DEFAULT_SS_SCL_LCNT	(0xc0)
#define IMAPX200_IIC_DEFAULT_RX_TL		(0x0)
#define IMAPX200_IIC_DEFAULT_TX_TL		(0x2)
#define IMAPX200_IIC_DEFAULT_SDA 		(0x20)
#define IMAPX200_STANDARD_SPEED			(0x1<<1)
#define IMAPX200_MATER_MODE			(0x1)
#define IMAPX200_SLAVE_DISABLE			(0x1<<6)
#define IMAPX200_IGNOREACK			(0x1<<8)
#define IMAPX200_RESTART_ENABLE			(0x1<<5)
#define IMAPX200_MASK_ALL_INT			(0x0)
#define IMAPX200_IIC_ENABLE			(0x1)
#define IMAPX200_IIC_TX_FIFO_DEPTH		(0x10)
#define IMAPX200_IIC_RX_FIFO_DEPTH		(0x10)
#define IMAPX200_IIC_READ_CMD			(0x1<<8)
#define IMAPX200_IIC_RX_FULL			(0x1<<2)
#define IMAPX200_IIC_TX_ABORT			(0x1<<6)
#define IMAPX200_IIC_TX_EMPTY			(0x1<<4)
#define IMAPX200_IIC_LOST_ARBITRATION		(0x1<<12)

// MASK BIT
#define INT_GEN_CALL    (0x1<<11)
#define INT_START_DET   (0x1<<10)
#define INT_STOP_DET    (0x1<<9)
#define INT_ACTIVE      (0x1<<8)
#define INT_RX_DONE     (0x1<<7)
#define INT_TX_ABORT    (0x1<<6)
#define INT_RD_REQ      (0x1<<5)
#define INT_TX_EMPTY    (0x1<<4)
#define INT_TX_OVER     (0x1<<3)
#define INT_RX_FULL     (0x1<<2)
#define INT_RX_OVER     (0x1<<1)
#define INT_RX_UNDER    (0x1<<0)

#endif
