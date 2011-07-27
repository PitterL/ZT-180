#ifndef __IMAPX_GPIO__
#define __IMAPX_GPIO__

#define rGPADAT    			 (IMAP_VA_GPIO+0x00 )     //Port A Data Register
#define rGPACON   			 (IMAP_VA_GPIO+0x04 )     //Port A Data Direction Register
#define rGPAPUD     		 	 (IMAP_VA_GPIO+0x08 )     //Port A Data Source
#define rGPBDAT    			 (IMAP_VA_GPIO+0x10 )     //Port A Data Register
#define rGPBCON   			 (IMAP_VA_GPIO+0x14 )     //Port A Data Direction Register
#define rGPBPUD     		 	 (IMAP_VA_GPIO+0x18 )     //Port A Data Source
#define rGPCDAT    			 (IMAP_VA_GPIO+0x20 )     //Port A Data Register
#define rGPCCON   			 (IMAP_VA_GPIO+0x24 )     //Port A Data Direction Register
#define rGPCPUD     		 	 (IMAP_VA_GPIO+0x28 )     //Port A Data Source
#define rGPDDAT    			 (IMAP_VA_GPIO+0x30 )     //Port A Data Register
#define rGPDCON   			 (IMAP_VA_GPIO+0x34 )     //Port A Data Direction Register
#define rGPDPUD     		 	 (IMAP_VA_GPIO+0x38 )     //Port A Data Source
#define rGPEDAT    			 (IMAP_VA_GPIO+0x40 )     //Port A Data Register
#define rGPECON   			 (IMAP_VA_GPIO+0x44 )     //Port A Data Direction Register
#define rGPEPUD    	 		 (IMAP_VA_GPIO+0x48 )     //Port A Data Source
#define rGPFDAT    			 (IMAP_VA_GPIO+0x50 )     //Port A Data Register
#define rGPFCON   			 (IMAP_VA_GPIO+0x54 )     //Port A Data Direction Register
#define rGPFPUD     		 	 (IMAP_VA_GPIO+0x58 )     //Port A Data Source
#define rGPGDAT    			 (IMAP_VA_GPIO+0x60 )     //Port A Data Register
#define rGPGCON   			 (IMAP_VA_GPIO+0x64 )     //Port A Data Direction Register
#define rGPGPUD    	 		 (IMAP_VA_GPIO+0x68 )     //Port A Data Source
#define rGPHDAT    			 (IMAP_VA_GPIO+0x70 )     //Port A Data Register
#define rGPHCON   			 (IMAP_VA_GPIO+0x74 )     //Port A Data Direction Register
#define rGPHPUD     		 	 (IMAP_VA_GPIO+0x78 )     //Port A Data Source
#define rGPIDAT    			 (IMAP_VA_GPIO+0x80 )     //Port A Data Register
#define rGPICON   			 (IMAP_VA_GPIO+0x84 )     //Port A Data Direction Register
#define rGPIPUD     			 (IMAP_VA_GPIO+0x88 )     //Port A Data Source
#define rGPJDAT    			 (IMAP_VA_GPIO+0x90 )     //Port A Data Register
#define rGPJCON   			 (IMAP_VA_GPIO+0x94 )     //Port A Data Direction Register
#define rGPJPUD     			 (IMAP_VA_GPIO+0x98 )     //Port A Data Source
#define rGPKDAT   	 		 (IMAP_VA_GPIO+0xA0 )     //Port A Data Register
#define rGPKCON   			 (IMAP_VA_GPIO+0xA4 )     //Port A Data Direction Register
#define rGPKPUD     		 	 (IMAP_VA_GPIO+0xA8 )     //Port A Data Source
#define rGPLDAT    			 (IMAP_VA_GPIO+0xB0 )     //Port A Data Register
#define rGPLCON   			 (IMAP_VA_GPIO+0xB4 )     //Port A Data Direction Register
#define rGPLPUD     		 	 (IMAP_VA_GPIO+0xB8 )     //Port A Data Source
#define rGPMDAT    			 (IMAP_VA_GPIO+0xC0 )     //Port A Data Register
#define rGPMCON    		 	 (IMAP_VA_GPIO+0xC4 )     //Port A Data Direction Register
#define rGPMPUD     		 	 (IMAP_VA_GPIO+0xC8 )     //Port A Data Source
#define rGPNDAT     		 	 (IMAP_VA_GPIO+0xD0 )     //Port A Data Register
#define rGPNCON   			 (IMAP_VA_GPIO+0xD4 )     //Port A Data Direction Register
#define rGPNPUD     		 	 (IMAP_VA_GPIO+0xD8 )     //Port A Data Source
#define rGPODAT    			 (IMAP_VA_GPIO+0xE0 )     //Port A Data Register
#define rGPOCON   			 (IMAP_VA_GPIO+0xE4 )     //Port A Data Direction Register
#define rGPOPUD     		 	 (IMAP_VA_GPIO+0xE8 )     //Port A Data Source
#define rGPPDAT    			 (IMAP_VA_GPIO+0xF0 )     //Port A Data Register
#define rGPPCON   			 (IMAP_VA_GPIO+0xF4 )     //Port A Data Direction Register
#define rGPPPUD     		 	 (IMAP_VA_GPIO+0xF8 )     //Port A Data Source
#define rGPQDAT    			 (IMAP_VA_GPIO+0x100 )     //Port A Data Register
#define rGPQCON   			 (IMAP_VA_GPIO+0x104 )     //Port A Data Direction Register
#define rGPQPUD     		 	 (IMAP_VA_GPIO+0x108 )     //Port A Data Source

#define rEINTCON    			 (IMAP_VA_GPIO+0x200 )     //Port A Data Register
#define rEINTFLTCON0   		 	 (IMAP_VA_GPIO+0x204 )     //Port A Data Direction Register
#define rEINTFLTCON1     		 (IMAP_VA_GPIO+0x208 )     //Port A Data Source
#define rEINTGCON    			 (IMAP_VA_GPIO+0x210 )     //Port A Data Register
#define rEINTGFLTCON0  		 	 (IMAP_VA_GPIO+0x214 )     //Port A Data Direction Register
#define rEINTGFLTCON1     		 (IMAP_VA_GPIO+0x218 )     //Port A Data Source
#define rEINTG1MASK    		 	 (IMAP_VA_GPIO+0x21C )     //Port A Data Register
#define rEINTG2MASK   			 (IMAP_VA_GPIO+0x220 )     //Port A Data Direction Register
#define rEINTG3MASK     		 (IMAP_VA_GPIO+0x224 )     //Port A Data Source
#define rEINTG4MASK    		 	 (IMAP_VA_GPIO+0x228 )     //Port A Data Register
#define rEINTG5MASK   			 (IMAP_VA_GPIO+0x22C )     //Port A Data Direction Register
#define rEINTG6MASK     		 (IMAP_VA_GPIO+0x230 )     //Port A Data Source
#define rEINTG1PEND    			 (IMAP_VA_GPIO+0x234 )     //Port A Data Register
#define rEINTG2PEND   			 (IMAP_VA_GPIO+0x238 )     //Port A Data Direction Register
#define rEINTG3PEND     		 (IMAP_VA_GPIO+0x23C )     //Port A Data Source
#define rEINTG4PEND    			 (IMAP_VA_GPIO+0x240 )     //Port A Data Register
#define rEINTG5PEND   			 (IMAP_VA_GPIO+0x244 )     //Port A Data Direction Register
#define rEINTG6PEND     		 (IMAP_VA_GPIO+0x248 )     //Port A Data Source

#endif
