#ifndef __IMAPX_IIS__
#define __IMAPX_IIS__

#define rIER                     (0x00 )     //IIS Enable Register
#define rIRER                    (0x04 )     //IIS Receiver Block Enable Register
#define rITER                    (0x08 )     //IIS Transmitter Block Enable Register
#define rCER                     (0x0C )     //Clock Enable Register
#define rCCR                     (0x10 )     //Clock Configuration Register
#define rRXFFR                   (0x14 )     //Receiver Block FIFO Register
#define rTXFFR                   (0x18 )     //Transmitter Block FIFO Register
#define rLRBR0                   (0x20 )     //Left Receive Buffer 0 
#define rLTHR0                   (0x20 )     //Left Transmit Holding Register 0
#define rRRBR0                   (0x24 )     //Right Receive Buffer 0 
#define rRTHR0                   (0x24 )     //Right Transmit Holding Register 0 
#define rRER0                    (0x28 )     //Receive Enable Register 0 
#define rTER0                    (0x2C )     //Transmit Enable Register 0
#define rRCR0                    (0x30 )     //Receive Configuration Register 0
#define rTCR0                    (0x34 )     //Transmit Configuration Register 0
#define rISR0                    (0x38 )     //Interrupt Status Register 0
#define rIMR0                    (0x3C )     //Interrupt Mask Register 0
#define rROR0                    (0x40 )     //Receive Overrun Register 0 
#define rTOR0                    (0x44 )     //Transmit Overrun Register 0
#define rRFCR0                   (0x48 )     //Receive FIFO Configuration Register 0
#define rTFCR0                   (0x4C )     //Transimit FIFO Configuration Register 0
#define rRFF0                    (0x50 )     //Receive FIFO Flush 0
#define rTFF0                    (0x54 )     //Transmit FIFO Flush 0
#define rLRBR1                   (0x60 )     //Left Receive Buffer 1 
#define rLTHR1                   (0x60 )     //Left Transmit Holding Register 1
#define rRRBR1                   (0x64 )     //Right Receive Buffer 1
#define rRTHR1                   (0x64 )     //Right Transmit Holding Register 1
#define rRER1                    (0x68 )     //Receive Enable Register 1
#define rTER1                    (0x6C )     //Transmit Enable Register 1
#define rRCR1                    (0x70 )     //Receive Configuration Register 1
#define rTCR1                    (0x74 )     //Transmit Configuration Register 1
#define rISR1                    (0x78 )     //Interrupt Status Register 1
#define rIMR1                    (0x7C )     //Interrupt Mask Register 1
#define rROR1                    (0x80 )     //Receive Overrun Register 1
#define rTOR1                    (0x84 )     //Transmit Overrun Register 1
#define rRFCR1                   (0x88 )     //Receive FIFO Configuration Register 1
#define rTFCR1                   (0x8C )     //Transmit FIFO Configuration Register 1
#define rRFF1                    (0x90 )     //Receive FIFO Flush 1
#define rTFF1                    (0x94 )     //Transmit FIFO Flush 1
#define rLRBR2                   (0xA0 )     //Left Receive Buffer 2
#define rLTHR2                   (0xA0 )     //Left Transmit Holding Register 2
#define rRRBR2                   (0xA4 )     //Right Receive Buffer2
#define rRTHR2                   (0xA4 )     //Right Transmit Holding Register 2
#define rRER2                    (0xA8 )     //Receive Enable Register 2
#define rTER2                    (0xAC )     //Transmit Enable Register 2
#define rRCR2                    (0xB0 )     //Receive Configuration Register 2
#define rTCR2                    (0xB4 )     //Transmit Configuration Register 2
#define rISR2                    (0xB8 )     //Interrupt Status Register 2
#define rIMR2                    (0xBC )     //Interrupt Mask Register 2
#define rROR2                    (0xC0 )     //Receive Overrun Register 2
#define rTOR2                    (0xC4 )     //Transmit Overrun Register 2
#define rRFCR2                   (0xC8 )     //Receive FIFO Configuration Register 2
#define rTFCR2                   (0xCC )     //Transmit FIFO Configuration Register 2
#define rRFF2                    (0xD0 )     //Receive FIFO Flush 2
#define rTFF2                    (0xD4 )     //Transmit FIFO Flush 2
#define rLRBR3                   (0xE0 )     //Left Receive Buffer 3
#define rLTHR3                   (0xE0 )     //Left Transmit Holding Register 3
#define rRRBR3                   (0xE4 )     //Right Receive Buffer3
#define rRTHR3                   (0xE4 )     //Right Transmit Holding Register 3
#define rRER3                    (0xE8 )     //Receive Enable Register 3
#define rTER3                    (0xEC )     //Transmit Enable Register 3
#define rRCR3                    (0xF0 )     //Receive Configuration Register 3
#define rTCR3                    (0xF4 )     //Transmit Configuration Register 3
#define rISR3                    (0xF8 )     //Interrupt Status Register 3
#define rIMR3                    (0xFC )     //Interrupt Mask Register 3
#define rROR3                    (0x100 )     //Receive Overrun Register 3
#define rTOR3                    (0x104 )     //Transmit Overrun Register 3
#define rRFCR3                   (0x108 )     //Receive FIFO Configuration Register 3
#define rTFCR3                   (0x10C )     //Transmit FIFO Configuration Register 3
#define rRFF3                    (0x110 )     //Receive FIFO Flush 3
#define rTFF3                    (0x114 )     //Transmit FIFO Flush 3
#define rRXDMA                   (0x1C0 )     //Receiver Block DMA Register
#define rRRXDMA                  (0x1C4 )     //Reset Receiver Block DMA Register
#define rTXDMA                   (0x1C8 )     //Transmitter Block DMA Register
#define rRTXDMA                  (0x1CC )     //Reset Transmitter Block DMA Register
#define rI2S_COMP_PARAM_2        (0x1F0 )     //Component Parameter 2 Register
#define rI2S_COMP_PARAM_1        (0x1F4 )     //Component Parameter 1 Register
#define rI2S_COMP_VERSION        (0x1F8 )     //Component Version ID   
#define rI2S_COMP_TYPE           (0x1FC )     //DesignWare Component Type 

#define rI2S_PRESCALER           (0x1C )     //[15:0]->I2SCLK_div ,[23:16]->CDCLK_div ,[25:24]->I2SCLK_mux ,[27:26]->CDCLK_mux    (0->PCLK,1->FCLK,2->EXTCLK)

#endif
