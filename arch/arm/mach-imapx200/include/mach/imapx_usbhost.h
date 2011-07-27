#ifndef __IMAPX_USBHOST__
#define __IMAPX_USBHOST__

#define rUHPHcRevision 	           (0x00 )   // Revision
#define rUHPHcControl 	           (0x04 )   // Operating modes for the Host Controller
#define rUHPHcCommandStatus 	   (0x08 )   // Command & status Register
#define rUHPHcInterruptStatus 	   (0x0C )   // Interrupt Status Register
#define rUHPHcInterruptEnable 	   (0x10 )   // Interrupt Enable Register
#define rUHPHcInterruptDisable 	   (0x14 )   // Interrupt Disable Register
#define rUHPHcHCCA 	           (0x18 )   // Pointer to the Host Controller Communication Area
#define rUHPHcPeriodCurrentED 	   (0x1C )   // Current Isochronous or Interrupt Endpoint Descriptor
#define rUHPHcControlHeadED 	   (0x20 )   // First Endpoint Descriptor of the Control list
#define rUHPHcControlCurrentED 	   (0x24 )   // Endpoint Control and Status Register
#define rUHPHcBulkHeadED 	   (0x28 )   // First endpoint register of the Bulk list
#define rUHPHcBulkCurrentED 	   (0x2C )   // Current endpoint of the Bulk list
#define rUHPHcBulkDoneHead 	   (0x30 )   // Last completed transfer descriptor
#define rUHPHcFmInterval 	   (0x34 )   // Bit time between 2 consecutive SOFs
#define rUHPHcFmRemaining 	   (0x38 )   // Bit time remaining in the current Frame
#define rUHPHcFmNumber 	           (0x3C )   // Frame number
#define rUHPHcPeriodicStart 	   (0x40 )   // Periodic Start
#define rUHPHcLSThreshold 	   (0x44 )   // LS Threshold
#define rUHPHcRhDescriptorA 	   (0x48 )   // Root Hub characteristics A
#define rUHPHcRhDescriptorB 	   (0x4C )   // Root Hub characteristics B
#define rUHPHcRhStatus 	           (0x50 )   // Root Hub Status register
#define rUHPHcRhPortStatus0 	   (0x54 )   // Root Hub Port Status Register                                                                                           
#define rUHPHcRhPortStatus1 	   (0x58 )   // Root Hub Port Status Register  

#endif
