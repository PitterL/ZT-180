#ifndef __IMAPX_SDI__
#define __IMAPX_SDI__


#define rSDICON     	 	 (0x00 )		//SDI control
#define rSDIPRE     	 	 (0x04 )		//SDI baud rate prescaler
#define rSDICARG		 (0x08 )		//SDI command argument
#define rSDICCON		 (0x0c )		//SDI command control
#define rSDICSTA		 (0x10 )		//SDI command status
#define rSDIRSP0		 (0x14 )		//SDI response 0
#define rSDIRSP1		 (0x18 )		//SDI response 1
#define rSDIRSP2		 (0x1c )		//SDI response 2
#define rSDIRSP3		 (0x20 )		//SDI response 3
#define rSDIDTIMER	 	 (0x24 )		//SDI data/busy timer
#define rSDIBSIZE	 	 (0x28 )		//SDI block size
#define rSDIDCON	 	 (0x2c )		//SDI data control
#define rSDIDCNT		 (0x30 )		//SDI data remain counter
#define rSDIDSTA		 (0x34 )		//SDI data status
#define rSDIFSTA		 (0x38 )		//SDI FIFO status
#define rSDIIMSK		 (0x3c )		//SDI interrupt mask. edited for 2440A
#define rSDIDAT    	 	 (0x40 )		//SDI data 
#define rSDIDMAADRA	 	 (0x80 )
#define rSDIDMACA		 (0x84 )
#define rSDIDMAADRB		 (0x88 )
#define rSDIDMACB		 (0x8C )
#define rSDIDMAADRC		 (0x90 )
#define rSDIDMACC		 (0x94 )
#define rSDIDMAADRD	 	 (0x98 )
#define rSDIDMACD		 (0x9C )

#endif
