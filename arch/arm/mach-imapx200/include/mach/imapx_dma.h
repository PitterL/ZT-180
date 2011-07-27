#ifndef __IMAPX_DMA__
#define __IMAPX_DMA__

#define DMA_MAX_NUM		8
#define DMA_MAX_BLOCKSIZE	4096
#define HANDSHAKE_INDEX_MAX 		16

#define DMA_0				0
#define DMA_1				1
#define DMA_2				2
#define DMA_3				3
#define DMA_4				4
#define DMA_5				5


#define MASTER_SELECT		0x30000000

#define rDMA_SAR0 			 (0x0000 )			// Channel 1 Source Address Register
#define rDMA_SAR0H 			 (0x0004 )			// Channel 1 Source Address Register
#define rDMA_DAR0			 (0x0008 )			// Channel 1 Destination Address Register              
#define rDMA_DAR0H			 (0x000C )			// Channel 1 Destination Address Register              
#define rDMA_LLP0 			 (0x0010 )			// Channel 1 Linked List Pointer Register
#define rDMA_LLP0H 			 (0x0014 )			// Channel 1 Linked List Pointer Register
#define rDMA_CTL0			 (0x0018 )			// Channel 1 Control Register
#define rDMA_CTL0H			 (0x001C )			// Channel 1 Control Register
#define rDMA_SSTAT0			 (0x0020 )     		// Channel 1 Source Status Register
#define rDMA_SSTAT0H		 	 (0x0024 )     		// Channel 1 Source Status Register
#define rDMA_DSTAT0		 	 (0x0028 )   			// Channel 1 Destination Status Register
#define rDMA_DSTAT0H		 	 (0x002C )   			// Channel 1 Destination Status Register
#define rDMA_SSTATAR0		 	 (0x0030 )     		// Channel 1 Source Status Address Register
#define rDMA_SSTATAR0H		 	 (0x0034 )     		// Channel 1 Source Status Address Register
#define rDMA_DSTATAR0		 	 (0x0038 )    			// Channel 1 Destination Status Address Register
#define rDMA_DSTATAR0H		 	 (0x003C )    			// Channel 1 Destination Status Address Register
#define rDMA_CFG0			 (0x0040 )    			// Channel 1 Configuration Register
#define rDMA_CFG0H			 (0x0044 )    			// Channel 1 Configuration Register
#define rDMA_SGR0			 (0x0048 )			// Channel 1 Source Gather Register
#define rDMA_SGR0H			 (0x004C )			// Channel 1 Source Gather Register
#define rDMA_DSR0			 (0x0050 )			// Channel 1 Destination Scatter Register
#define rDMA_DSR0H			 (0x0054 )			// Channel 1 Destination Scatter Register
                                                                        
#define rDMA_SAR1 			 (0x0058 )			// Channel 1 Source Address Register
#define rDMA_SAR1H 			 (0x005C )			// Channel 1 Source Address Register
#define rDMA_DAR1			 (0x0060 )			// Channel 1 Destination Address Register              
#define rDMA_DAR1H			 (0x0064 )			// Channel 1 Destination Address Register              
#define rDMA_LLP1 			 (0x0068 )			// Channel 1 Linked List Pointer Register
#define rDMA_LLP1H 			 (0x006C )			// Channel 1 Linked List Pointer Register
#define rDMA_CTL1			 (0x0070 )			// Channel 1 Control Register
#define rDMA_CTL1H			 (0x0074 )			// Channel 1 Control Register
#define rDMA_SSTAT1			 (0x0078 )     		// Channel 1 Source Status Register
#define rDMA_SSTAT1H		 	 (0x007C )     		// Channel 1 Source Status Register
#define rDMA_DSTAT1		 	 (0x0080 )   			// Channel 1 Destination Status Register
#define rDMA_DSTAT1H		 	 (0x0084 )   			// Channel 1 Destination Status Register
#define rDMA_SSTATAR1		 	 (0x0088 )     		// Channel 1 Source Status Address Register
#define rDMA_SSTATAR1H		 	 (0x008C )     		// Channel 1 Source Status Address Register
#define rDMA_DSTATAR1		 	 (0x0090 )    			// Channel 1 Destination Status Address Register
#define rDMA_DSTATAR1H		 	 (0x0094 )    			// Channel 1 Destination Status Address Register
#define rDMA_CFG1			 (0x0098 )    			// Channel 1 Configuration Register
#define rDMA_CFG1H			 (0x009C )    			// Channel 1 Configuration Register
#define rDMA_SGR1			 (0x00A0 )			// Channel 1 Source Gather Register
#define rDMA_SGR1H			 (0x00A4 )			// Channel 1 Source Gather Register
#define rDMA_DSR1			 (0x00A8 )			// Channel 1 Destination Scatter Register
#define rDMA_DSR1H			 (0x00AC )			// Channel 1 Destination Scatter Register

#define rDMA_SAR2 			 (0x00B0 )			// Channel 2 Source Address Register
#define rDMA_SAR2H 			 (0x00B4 )			// Channel 2 Source Address Register
#define rDMA_DAR2			 (0x00B8 )			// Channel 2 Destination Address Register              
#define rDMA_DAR2H			 (0x00BC )			// Channel 2 Destination Address Register              
#define rDMA_LLP2 			 (0x00C0 )			// Channel 2 Linked List Pointer Register
#define rDMA_LLP2H 			 (0x00C4 )			// Channel 2 Linked List Pointer Register
#define rDMA_CTL2			 (0x00C8 )			// Channel 2 Control Register
#define rDMA_CTL2H			 (0x00CC )			// Channel 2 Control Register
#define rDMA_SSTAT2			 (0x00D0 )     		// Channel 2 Source Status Register
#define rDMA_SSTAT2H		 	 (0x00D4 )     		// Channel 2 Source Status Register
#define rDMA_DSTAT2		 	 (0x00D8 )   			// Channel 2 Destination Status Register
#define rDMA_DSTAT2H		 	 (0x00DC )   			// Channel 2 Destination Status Register
#define rDMA_SSTATAR2		 	 (0x00E0 )     		// Channel 2 Source Status Address Register
#define rDMA_SSTATAR2H		 	 (0x00E4 )     		// Channel 2 Source Status Address Register
#define rDMA_DSTATAR2		 	 (0x00E8 )    			// Channel 2 Destination Status Address Register
#define rDMA_DSTATAR2H		 	 (0x00EC )    			// Channel 2 Destination Status Address Register
#define rDMA_CFG2			 (0x00F0 )    			// Channel 2 Configuration Register
#define rDMA_CFG2H			 (0x00F4 )    			// Channel 2 Configuration Register
#define rDMA_SGR2			 (0x00F8 )			// Channel 2 Source Gather Register
#define rDMA_SGR2H			 (0x00FC )			// Channel 2 Source Gather Register
#define rDMA_DSR2			 (0x0100 )			// Channel 2 Destination Scatter Register
#define rDMA_DSR2H			 (0x0104 )			// Channel 2 Destination Scatter Register

#define rDMA_SAR3 			 (0x0108 )			// Channel 3 Source Address Register
#define rDMA_SAR3H 			 (0x010C )			// Channel 3 Source Address Register
#define rDMA_DAR3			 (0x0110 )			// Channel 3 Destination Address Register              
#define rDMA_DAR3H			 (0x0114 )			// Channel 3 Destination Address Register              
#define rDMA_LLP3 			 (0x0118 )			// Channel 3 Linked List Pointer Register
#define rDMA_LLP3H 			 (0x011C )			// Channel 3 Linked List Pointer Register
#define rDMA_CTL3			 (0x0120 )			// Channel 3 Control Register
#define rDMA_CTL3H			 (0x0124 )			// Channel 3 Control Register
#define rDMA_SSTAT3			 (0x0128 )     		// Channel 3 Source Status Register
#define rDMA_SSTAT3H		 	 (0x012C )     		// Channel 3 Source Status Register
#define rDMA_DSTAT3		 	 (0x0130 )   			// Channel 3 Destination Status Register
#define rDMA_DSTAT3H		 	 (0x0134 )   			// Channel 3 Destination Status Register
#define rDMA_SSTATAR3		 	 (0x0138 )     		// Channel 3 Source Status Address Register
#define rDMA_SSTATAR3H		 	 (0x013C )     		// Channel 3 Source Status Address Register
#define rDMA_DSTATAR3		 	 (0x0140 )    			// Channel 3 Destination Status Address Register
#define rDMA_DSTATAR3H		 	 (0x0144 )    			// Channel 3 Destination Status Address Register
#define rDMA_CFG3			 (0x0148 )    			// Channel 3 Configuration Register
#define rDMA_CFG3H			 (0x014C )    			// Channel 3 Configuration Register
#define rDMA_SGR3			 (0x0150 )			// Channel 3 Source Gather Register
#define rDMA_SGR3H			 (0x0154 )			// Channel 3 Source Gather Register
#define rDMA_DSR3			 (0x0158 )			// Channel 3 Destination Scatter Register
#define rDMA_DSR3H			 (0x015C )			// Channel 3 Destination Scatter Register

#define rDMA_SAR4 			 (0x0160 )			// Channel 4 Source Address Register
#define rDMA_SAR4H 			 (0x0164 )			// Channel 4 Source Address Register
#define rDMA_DAR4			 (0x0168 )			// Channel 4 Destination Address Register              
#define rDMA_DAR4H			 (0x016C )			// Channel 4 Destination Address Register              
#define rDMA_LLP4 			 (0x0170 )			// Channel 4 Linked List Pointer Register
#define rDMA_LLP4H 			 (0x0174 )			// Channel 4 Linked List Pointer Register
#define rDMA_CTL4			 (0x0178 )			// Channel 4 Control Register
#define rDMA_CTL4H			 (0x017C )			// Channel 4 Control Register
#define rDMA_SSTAT4			 (0x0180 )     		// Channel 4 Source Status Register
#define rDMA_SSTAT4H		 	 (0x0184 )     		// Channel 4 Source Status Register
#define rDMA_DSTAT4		 	 (0x0188 )   			// Channel 4 Destination Status Register
#define rDMA_DSTAT4H		 	 (0x018C )   			// Channel 4 Destination Status Register
#define rDMA_SSTATAR4		 	 (0x0190 )     		// Channel 4 Source Status Address Register
#define rDMA_SSTATAR4H		 	 (0x0194 )     		// Channel 4 Source Status Address Register
#define rDMA_DSTATAR4		 	 (0x0198 )    			// Channel 4 Destination Status Address Register
#define rDMA_DSTATAR4H		 	 (0x019C )    			// Channel 4 Destination Status Address Register
#define rDMA_CFG4			 (0x01A0 )    			// Channel 4 Configuration Register
#define rDMA_CFG4H			 (0x01A4 )    			// Channel 4 Configuration Register
#define rDMA_SGR4			 (0x01A8 )			// Channel 4 Source Gather Register
#define rDMA_SGR4H			 (0x01AC )			// Channel 4 Source Gather Register
#define rDMA_DSR4			 (0x01B0 )			// Channel 4 Destination Scatter Register
#define rDMA_DSR4H			 (0x01B4 )			// Channel 4 Destination Scatter Register

#define rDMA_SAR5 			 (0x01B8 )			// Channel 5 Source Address Register
#define rDMA_SAR5H 			 (0x01BC )			// Channel 5 Source Address Register
#define rDMA_DAR5			 (0x01C0 )			// Channel 5 Destination Address Register              
#define rDMA_DAR5H			 (0x01C4 )			// Channel 5 Destination Address Register              
#define rDMA_LLP5 			 (0x01C8 )			// Channel 5 Linked List Pointer Register
#define rDMA_LLP5H 			 (0x01CC )			// Channel 5 Linked List Pointer Register
#define rDMA_CTL5			 (0x01D0 )			// Channel 5 Control Register
#define rDMA_CTL5H			 (0x01D4 )			// Channel 5 Control Register
#define rDMA_SSTAT5			 (0x01D8 )     		// Channel 5 Source Status Register
#define rDMA_SSTAT5H		 	 (0x01DC )     		// Channel 5 Source Status Register
#define rDMA_DSTAT5		 	 (0x01E0 )   			// Channel 5 Destination Status Register
#define rDMA_DSTAT5H		 	 (0x01E4 )   			// Channel 5 Destination Status Register
#define rDMA_SSTATAR5		 	 (0x01E8 )     		// Channel 5 Source Status Address Register
#define rDMA_SSTATAR5H		 	 (0x01EC )     		// Channel 5 Source Status Address Register
#define rDMA_DSTATAR5		 	 (0x01F0 )    			// Channel 5 Destination Status Address Register
#define rDMA_DSTATAR5H		 	 (0x01F4 )    			// Channel 5 Destination Status Address Register
#define rDMA_CFG5			 (0x01F8 )    			// Channel 5 Configuration Register
#define rDMA_CFG5H			 (0x01FC )    			// Channel 5 Configuration Register
#define rDMA_SGR5			 (0x0200 )			// Channel 5 Source Gather Register
#define rDMA_SGR5H			 (0x0204 )			// Channel 5 Source Gather Register
#define rDMA_DSR5			 (0x0208 )			// Channel 5 Destination Scatter Register
#define rDMA_DSR5H			 (0x020C )			// Channel 5 Destination Scatter Register

#define rDMA_SAR6 			 (0x0210 )			// Channel 6 Source Address Register
#define rDMA_SAR6H 			 (0x0214 )			// Channel 6 Source Address Register
#define rDMA_DAR6			 (0x0218 )			// Channel 6 Destination Address Register              
#define rDMA_DAR6H			 (0x021C )			// Channel 6 Destination Address Register              
#define rDMA_LLP6 			 (0x0220 )			// Channel 6 Linked List Pointer Register
#define rDMA_LLP6H 			 (0x0224 )			// Channel 6 Linked List Pointer Register
#define rDMA_CTL6			 (0x0228 )			// Channel 6 Control Register
#define rDMA_CTL6H			 (0x022C )			// Channel 6 Control Register
#define rDMA_SSTAT6			 (0x0230 )     		// Channel 6 Source Status Register
#define rDMA_SSTAT6H		 	 (0x0234 )     		// Channel 6 Source Status Register
#define rDMA_DSTAT6		 	 (0x0238 )   			// Channel 6 Destination Status Register
#define rDMA_DSTAT6H		 	 (0x023C )   			// Channel 6 Destination Status Register
#define rDMA_SSTATAR6		 	 (0x0240 )     		// Channel 6 Source Status Address Register
#define rDMA_SSTATAR6H		 	 (0x0244 )     		// Channel 6 Source Status Address Register
#define rDMA_DSTATAR6		 	 (0x0248 )    			// Channel 6 Destination Status Address Register
#define rDMA_DSTATAR6H		 	 (0x024C )    			// Channel 6 Destination Status Address Register
#define rDMA_CFG6			 (0x0250 )    			// Channel 6 Configuration Register
#define rDMA_CFG6H			 (0x0254 )    			// Channel 6 Configuration Register
#define rDMA_SGR6			 (0x0258 )			// Channel 6 Source Gather Register
#define rDMA_SGR6H			 (0x025C )			// Channel 6 Source Gather Register
#define rDMA_DSR6			 (0x0260 )			// Channel 6 Destination Scatter Register
#define rDMA_DSR6H			 (0x0264 )			// Channel 6 Destination Scatter Register

#define rDMA_SAR7 			 (0x0268 )			// Channel 7 Source Address Register
#define rDMA_SAR7H 			 (0x026C )			// Channel 7 Source Address Register
#define rDMA_DAR7			 (0x0270 )			// Channel 7 Destination Address Register              
#define rDMA_DAR7H			 (0x0274 )			// Channel 7 Destination Address Register              
#define rDMA_LLP7 			 (0x0278 )			// Channel 7 Linked List Pointer Register
#define rDMA_LLP7H 			 (0x027C )			// Channel 7 Linked List Pointer Register
#define rDMA_CTL7			 (0x0280 )			// Channel 7 Control Register
#define rDMA_CTL7H			 (0x0284 )			// Channel 7 Control Register
#define rDMA_SSTAT7			 (0x0288 )     		// Channel 7 Source Status Register
#define rDMA_SSTAT7H		 	 (0x028C )     		// Channel 7 Source Status Register
#define rDMA_DSTAT7		 	 (0x0290 )   			// Channel 7 Destination Status Register
#define rDMA_DSTAT7H		 	 (0x0294 )   			// Channel 7 Destination Status Register
#define rDMA_SSTATAR7		 	 (0x0298 )     		// Channel 7 Source Status Address Register
#define rDMA_SSTATAR7H		 	 (0x029C )     		// Channel 7 Source Status Address Register
#define rDMA_DSTATAR7		 	 (0x02A0 )    			// Channel 7 Destination Status Address Register
#define rDMA_DSTATAR7H		 	 (0x02A4 )    			// Channel 7 Destination Status Address Register
#define rDMA_CFG7			 (0x02A8 )    			// Channel 7 Configuration Register
#define rDMA_CFG7H			 (0x02AC )    			// Channel 7 Configuration Register
#define rDMA_SGR7			 (0x02B0 )			// Channel 7 Source Gather Register
#define rDMA_SGR7H			 (0x02B4 )			// Channel 7 Source Gather Register
#define rDMA_DSR7			 (0x02B8 )			// Channel 7 Destination Scatter Register
#define rDMA_DSR7H			 (0x02BC )			// Channel 7 Destination Scatter Register


//DMA-Interrupt
#define rDMA_RAWTFR 			 (0x02C0 )			// Raw Status for IntTfr Interrupt
#define rDMA_RAWBLOCK			 (0x02C8 )			// Raw Status for IntBlock Interrupt
#define rDMA_RAWSRCTRAN 		 (0x02D0 )			// Raw Status for IntSrcTran Interrupt
#define rDMA_RAWDSTTRAN			 (0x02D8 )			// Raw Status for IntDstTran Interrupt
#define rDMA_RAWERR			 (0x02E0 )     		// Raw Status for IntErr Interrupt
//DMA-Status
#define rDMA_STATUSTFR 			 (0x02E8 )			// Status for IntTfr Interrupt
#define rDMA_STATUSBLOCK		 (0x02F0 )			// Status for IntBlock Interrupt          
#define rDMA_STATUSSRCTRAN 		 (0x02F8 )			// Status for IntSrcTran Interrupt
#define rDMA_STATUSDSTTRAN		 (0x0300 )			// Status for IntDstTran Interrupt
#define rDMA_STATUSERR			 (0x0308 )     		// Status for IntErr Interrupt
//DMA-Mask
#define rDMA_MASKTFR 			 (0x0310 )			// Mask for IntTfr Interrupt
#define rDMA_MASKBLOCK			 (0x0318 )			// Mask for IntBlock Interrupt              
#define rDMA_MASKSRCTRAN 		 (0x0320 )			// Mask for IntSrcTran Interrupt
#define rDMA_MASKDSTTRAN		 (0x0328 )			// Mask for IntDstTran Interrupt
#define rDMA_MASKERR			 (0x0330 )     		// Mask for IntErr Interrupt
//DMA-Clear
#define rDMA_CLEARTFR 			 (0x0338 )			// Clear for IntTfr Interrupt
#define rDMA_CLEARBLOCK			 (0x0340 )			// Clear for IntBlock Interrupt           
#define rDMA_CLEARSRCTRAN 		 (0x0348 )			// Clear for IntSrcTran Interrupt
#define rDMA_CLEARDSTTRAN		 (0x0350 )			// Clear for IntDstTran Interrupt
#define rDMA_CLEARERR			 (0x0358 )     		// Clear for IntErr Interrupt
#define rDMA_STATUSInt			 (0x0360 )     		// Status for each interrupt type
//DMA-Req
#define rDMA_REQSRCREG 			 (0x0368 )			// Source Software Transaction Request Register
#define rDMA_REQDSTREG			 (0x0370 )			// Destination Software Transaction Request Register            
#define rDMA_SQL_REQSRCREG 		 (0x0378 )			// Single Source Transaction Request Register
#define rDMA_SGL_REQDSTREG		 (0x0380 )			// Single Destination Transaction Request Register
#define rDMA_LSTSRCREG			 (0x0388 )     		// Last Source Transaction Request Register
#define rDMA_LSTDSTREG			 (0x0390 )     		// Last Destination Transaction Request Register
//DMA-Miscellaneous
#define rDMA_CFGREG 			 (0x0398 )			// DMA Configuration Register
#define rDMA_CHENREG			 (0x03A0 )			// DMA Channel Enable Register            
#define rDMA_IDREG 			 (0x03A8 )			// DMA ID Register
#define rDMA_TESTREG			 (0x03B0 )			// DMA Test Register
#define rDMA_COMP_PARAMS_6		 (0x03C8 )     		// 
#define rDMA_COMP_PARAMS_5		 (0x03D0 )     		// 
#define rDMA_COMP_PARAMS_4		 (0x03D8 )     		// 
#define rDMA_COMP_PARAMS_3		 (0x03E0 )     		// 
#define rDMA_COMP_PARAMS_2		 (0x03E8 )     		// 
#define rDMA_COMP_PARAMS_1		 (0x03F0 )     		// 
#define rDMA_COMPONENT_IDREG		 (0x03F8 )     		// 

#endif
