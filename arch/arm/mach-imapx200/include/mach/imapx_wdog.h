#ifndef __IMAPX_WATCHDOG__
#define __IMAPX_WATCHDOG__

#define rWDT_CR   			 (IMAP_VA_WATCHDOG+0x00 )			// receive buffer register
#define rWDT_TORR    			 (IMAP_VA_WATCHDOG+0x04 )			// interrupt enable register                                                                                   									// divisor latch high         (0x04)
#define rWDT_CCVR    			 (IMAP_VA_WATCHDOG+0x08 )			// interrupt identity register
#define rWDT_CRR			 (IMAP_VA_WATCHDOG+0x0C )			// line control register      (0x0c)
#define rWDT_STAT    			 (IMAP_VA_WATCHDOG+0x10 )     		// modem control register     (0x10)
#define rWDT_EOI    			 (IMAP_VA_WATCHDOG+0x14 )   			// line status register       (0x14)
#define rWDT_COMP_PARAMS_5  		 (IMAP_VA_WATCHDOG+0xE4 )     		// modem status register      (0x18)
#define rWDT_COMP_PARAMS_4  		 (IMAP_VA_WATCHDOG+0xE8 )    		 // scratch register           (0x1c)
#define rWDT_COMP_PARAMS_3  		 (IMAP_VA_WATCHDOG+0xEC )
#define rWDT_COMP_PARAMS_2  		 (IMAP_VA_WATCHDOG+0xF0 )                    	
#define rWDT_COMP_PARAMS_1 		 (IMAP_VA_WATCHDOG+0xF4 )			// FIFO access register       (0x70)
#define rWDT_COMP_VERSION  	 	 (IMAP_VA_WATCHDOG+0xF8 )        		// transmit FIFO read         (0x74)
#define rWDT_COMP_TYPE    		 (IMAP_VA_WATCHDOG+0xFC )        		// receiver FIFO write        (0x78)

#endif

