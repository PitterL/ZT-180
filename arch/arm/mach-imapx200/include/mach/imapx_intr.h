#ifndef __IMAPX_IRQ__
#define __IMAPX_IRQ__

#define rSRCPND     					 (IMAP_VA_IRQ+0x00 )	//Interrupt request status
#define rINTMOD     					 (IMAP_VA_IRQ+0x04 )	//Interrupt mode control
#define rINTMSK     					 (IMAP_VA_IRQ+0x08 )	//Interrupt mask control
#define rPRIORITY   					 (IMAP_VA_IRQ+0x0c )	//IRQ priority control
#define rINTPND     					 (IMAP_VA_IRQ+0x10 )	//Interrupt request status
#define rINTOFFSET  					 (IMAP_VA_IRQ+0x14 )	//Interruot request source offset
#define rSUBSRCPND  					 (IMAP_VA_IRQ+0x18 )	//Sub source pending
#define rINTSUBMSK  					 (IMAP_VA_IRQ+0x1c )	//Interrupt sub mask
#define rCONT						 (IMAP_VA_IRQ+0x20 )
#define rSRCPND2    					 (IMAP_VA_IRQ+0x24 )  //Interrupt reguest status 2
#define rINTMOD2    					 (IMAP_VA_IRQ+0x28 )  //Interrupt mode control 2  
#define rINTMSK2    					 (IMAP_VA_IRQ+0x2c )  //Interrupt mask control 2  
#define rPRIORITY2  					 (IMAP_VA_IRQ+0x30 )  //IRQ prioity control 2     
#define rINTPND2    					 (IMAP_VA_IRQ+0x34 )  //Interrupt request status 2
#define rDEINTMSK1    					 (IMAP_VA_IRQ+0x38 )
#define rDEINTMSK2    					 (IMAP_VA_IRQ+0x3C )


// PENDING BIT
#define BIT_EINT0					(0x1<<0)
#define BIT_EINT1					(0x1<<1)
#define BIT_EINT2					(0x1<<2)
#define BIT_EINT3					(0x1<<3)
#define BIT_EINT4_7					(0x1<<4)
#define BIT_EINT8_23				(0x1<<5)
#define BIT_AC97						(0x1<<6)
#define BIT_IIS						(0x1<<7)
#define BIT_TICK						(0x1<<8)
//
#define BIT_WDT						(0x1<<9)
#define BIT_PWM0					(0x1<<10)
#define BIT_PWM1					(0x1<<11)
#define BIT_PWM2					(0x1<<12)
#define BIT_PWM3					(0x1<<13)
#define BIT_PWM4					(0x1<<14)
#define BIT_UART2					(0x1<<15)
//
#define BIT_LCD						(0x1<<16)
#define BIT_CAMIF					(0x1<<17)
#define BIT_TIMER0					(0x1<<18)
#define BIT_TIMER1					(0x1<<19) 
#define BIT_DMA						(0x1<<20)
#define BIT_SDI						(0x1<<21)
#define BIT_SPI0						(0x1<<22)
#define BIT_UART1					(0x1<<23)
//
#define BIT_NAND					(0x1<<24)
#define BIT_USBO					(0x1<<25)
#define BIT_USBH					(0x1<<26)
#define BIT_IIC						(0x1<<27)
#define BIT_UART0					(0x1<<28)
#define BIT_SPI1						(0x1<<29)
#define BIT_POWERMODE				(0x1<<30)
#define BIT_RTC						(0x1<<30)
#define BIT_G12						(0x1<<31)
//
#define BIT_CF						(0x1<<0)
#define BIT_UART3					(0x1<<1)
#define BIT_IIC1						(0x1<<2)
#define BIT_USBD						(0x1<<4)

#define BIT_KB						(0x1<<6)
#define BIT_KB_WAKEUP				(0x1<<7)
#define BIT_VENC						(0x1<<8)
#define BIT_VDEC						(0x1<<9)
#define BIT_GPS						(0x1<<10)
#define BIT_DSP						(0x1<<11)
#define BIT_EHCI						(0x1<<12)
#define BIT_PIC_0					(0x1<<13)
#define BIT_PIC_1         				(0x1<<18)

#endif
