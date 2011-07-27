#ifndef __IMAPX_MAC__
#define __IMAPX_MAC__

#define rMACConfig     	  		 	 (0x00 ) //This is the operation mode register for the MAC.
#define rMACFrameFilter   		 	 (0x04 ) //Contains the frame filtering controls.
#define rHashTableH     			 (0x08 ) //Contains the higher 32 bits of the Multicast Hash table.This register is present only when the Hash filter function is selected in coreConsultant.
#define rHashTableL     			 (0x0c ) //Contains the lower 32 bits of the Multicast Hash table.This register is present only when the Hash filter function is selected in coreConsultant.
#define rGMIIAddr		     		 (0x10 ) //Controls the management cycles to an external PHY. This register is present only when the Station Management (MDIO) feature is selected in coreConsultant.
#define rGMIIData		     		 (0x14 ) //Contains the data to be written to or read from the PHY register. This register is present only when the Station Management (MDIO) feature is selected in coreConsultant.
#define rFlowControl     			 (0x18 ) //Controls the generation of control frames.
#define rVLANTag		    		 (0x1c ) //Identifies IEEE 802.1Q VLAN type frames.
                                                    
#define rIdeftifiesVersion  	 		 (0x20 ) //Identifies the version of the Core
#define rReserved	   			 (0x24 ) //Reserved
#define rRemoteWakeUp   			 (0x28 ) //This is the address through which the remote Wake-up Frame Filter registers
#define rPMTControl     			 (0x2c ) //This register is present only when the PMT module is selected in coreConsultant.
#define rInterruptStatus   		 	 (0x38 ) //Contains the interrupt status.
#define rInterruptMask     		 	 (0x3c ) //Contains the masks for generating the interrupts.
#define rMACAddr0H     				 (0x40 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr0L	    			 (0x44 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr1H    				 (0x48 ) //Contains the interrupt status.
#define rMACAddr1L     				 (0x4c ) //Contains the masks for generating the interrupts.
#define rMACAddr2H      			 (0x50 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr2L 	    			 (0x54 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr3H    				 (0x58 ) //Contains the interrupt status.
#define rMACAddr3L      			 (0x5c ) //Contains the masks for generating the interrupts.
#define rMACAddr4H     				 (0x60 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr4L 	    			 (0x64 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr5H     				 (0x68 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr5L 	    			 (0x6C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr6H     				 (0x70 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr6L 	    			 (0x74 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr7H     				 (0x78 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr7L 	    			 (0x7C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr8H     				 (0x80 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr8L 	    			 (0x84 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr9H     				 (0x88 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr9L 	    			 (0x8C ) //Contains the lower 32 bits of the first MAC address.
                                                      
#define rMACAddr10H     			 (0x90 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr10L 	    		 	 (0x94 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr11H     			 (0x98 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr11L 	    		 	 (0x9C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr12H     			 (0xA0 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr12L 	    		 	 (0xA4 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr13H     			 (0xA8 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr13L 	    		 	 (0xAC ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr14H     			 (0xB0 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr14L 	    		 	 (0xB4 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr15H     			 (0xB8 ) //Contains the higher 16 bits of the first MAC address.
#define rMACAddr15L 	    		 	 (0xBC ) //Contains the lower 32 bits of the first MAC address.
                                                 
#define rANControl 	    			 (0xC0 ) //Enables and/or restarts auto-negotiation. It also enables PCS loopback.
#define rANStatus		    		 (0xC4 ) //Indicates the link and auto-negotiation status.
#define rANAdvertise    			 (0xC8 ) //This register is configured before auto-negotiation begins. It contains the advertised ability of the GMAC.
#define rANegoLink  				 (0xCC ) //Contains the advertised ability of the link partner.
#define rANExpansion 	    		 	 (0xD0 ) //Indicates whether a new base page has been received from the link partner.
#define rTBIExStatus	    		 	 (0xD4 ) //Indicates all modes of operation of the GMAC.
#define rSGRGStatus	    			 (0xD8 ) //Indicates the status signals received from the PHY through the SGMII/RGMII interface.
                                                
#define rTimeStampControl			 (0x700 ) //Controls the time stamp generation and update logic.
#define rSubSecIncrement			 (0x704 ) //Contains the 8-bit value by which the Sub-Second register is incremented.
#define rTimeStampHigh    		 	 (0x708 ) //Contains the most significant (higher) 32 time bits.
#define rTimeStampLow  				 (0x70C ) //Contains the least significant (lower) 32 time bits.
#define rTimeStampHighUp 			 (0x710 ) //Contains the most significant (higher) 32 bits of the time to be written to, added to, or subtracted from the System Time value.
#define rTimeStampLowUp	    	 	  	 (0x714 ) //Contains the least significant (lower) 32 bits of the time to be written to, added to, or subtracted from the System Time value.
#define rTimeStampAddend			 (0x718 ) //This register is used by the software to readjust the clock frequency linearly to match the master clock frequency.
#define rTargetTimeHigh	    	 	  	 (0x71C ) //This register is used by the software to readjust the clock frequency linearly to match the master clock frequency.
#define rTargetTimeLow	    	 		 (0x720 ) //Contains the lower 32 bits of time to be compared with the system time for interrupt event generation.
                                              
#define rMACAddr16H	    			 (0x800 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr16L				 (0x804 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr17H    				 (0x808 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr17L  				 (0x80C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr18H 				 (0x810 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr18L	    			 (0x814 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr19H	    			 (0x818 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr19L	    			 (0x81C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr20H	    			 (0x820 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr20L	    			 (0x824 ) //Contains the lower 32 bits of the first MAC address.
                          		                 
#define rMACAddr21H	    			 (0x828 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr21L				 (0x82C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr22H    				 (0x830 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr22L  				 (0x834 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr23H 				 (0x838 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr23L	    			 (0x83C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr24H	    			 (0x840 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr24L	    			 (0x844 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr25H	    			 (0x848 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr25L	    			 (0x84C ) //Contains the lower 32 bits of the first MAC address.
                          		                     
#define rMACAddr26H	    			 (0x850 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr26L				 (0x854 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr27H    				 (0x858 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr27L  				 (0x85C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr28H 				 (0x860 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr28L	    			 (0x864 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr29H	    			 (0x868 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr29L	    			 (0x86C ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr30H	    			 (0x870 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr30L	    			 (0x874 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr31H	    			 (0x878 ) //Contains the lower 32 bits of the first MAC address.
#define rMACAddr31L	    			 (0x87C ) //Contains the lower 32 bits of the first MAC address.


//Ethernet DMA Register
#define rBUSMODE     	       			 (0x1000 )    //Controls the Host Interface Mode
#define rTxPollDemand      			 (0x1004 )    //Used by the host to instruct the DMA to poll the Transmit Descriptor List
#define rRxPollDemand     			 (0x1008 )    //Used by the host to instruct the DMA to poll the Receive Descriptor List
#define rRxDescriptorListAddr     		 (0x100c )    //Points the DMA to the start of the Receive Descriptor list
#define rTxDescriptorListAddr     	 	 (0x1010 )    //Points the DMA to the start of the Transmit Descriptor list
#define rStatus                			 (0x1014 )    //The Software driver reads this register during interrupt service routine or polling to determint the status of the DMA
#define rOperationMode    			 (0x1018 )    //Establishes the Receive and Transmit operating modes and command
#define rInterruptEnable   			 (0x101c )    //Enables the interrupt reported by the Status Register
#define rMissOverflow     			 (0x1020 )    //Contains the counters for discarded frames because no host Receive Descriptor was available,and discarded frames because of Receive FIFO Overflow
#define rCurrHostTxDescriptor     		 (0x1048 )    //Points to the start of current Transmit Descriptor read by the DMA.
#define rCurrHostRxDescriptor     	 	 (0x104c )	  //Points to the start of current Receive Descriptor read by the DMA.
#define rCurrHostTxBufferAddr     	 	 (0x1050 )   //Points to the current Transmit Buffer address read by the DMA.
#define rCurrHostRxBufferAddr     	 	 (0x1054 )   //Points to the current Receive Buffer address read by the DMA.

#endif
