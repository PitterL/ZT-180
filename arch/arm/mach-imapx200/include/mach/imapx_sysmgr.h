#ifndef __IMAPX_SYSMGR__
#define __IMAPX_SYSMGR__


#define IMAP_PLL_EN     (1<<31)
#define IMAP_PLLVAL(_o,_d,_m) ((_o) << 12 | (_d) << 7 | ((_m)))

#define rPLL_LOCKED				     (IMAP_VA_SYSMGR+0x004 )
#define rPLL_OCLKSEL       			     (IMAP_VA_SYSMGR+0x008 )
#define rPLL_CLKSEL        			     (IMAP_VA_SYSMGR+0x00C )
#define rAPLL_CFG        			     (IMAP_VA_SYSMGR+0x010 )
#define rDPLL_CFG          			     (IMAP_VA_SYSMGR+0x014 )
#define rEPLL_CFG          			     (IMAP_VA_SYSMGR+0x018 )
#define rDIV_CFG0         		 	     (IMAP_VA_SYSMGR+0x01C )
#define rDIV_CFG1          			     (IMAP_VA_SYSMGR+0x020 )
#define rHCLK_MASK         			     (IMAP_VA_SYSMGR+0x024 )
#define rPCLK_MASK         			     (IMAP_VA_SYSMGR+0x028 )
#define rSCLK_MASK         			     (IMAP_VA_SYSMGR+0x02C )
#define rCLKOUT0_CFG     			     (IMAP_VA_SYSMGR+0x030 )		
#define rCLKOUT1_CFG     			     (IMAP_VA_SYSMGR+0x034 )
#define rCPUSYNC_CFG                     	     (IMAP_VA_SYSMGR+0x038 )
#define rDIV_CFG2                                    (IMAP_VA_SYSMGR+0x03C )
#define rUSB_SRST                                    (IMAP_VA_SYSMGR+0x040 )
#define rPERSIM_CFG                                  (IMAP_VA_SYSMGR+0x044 )
#define rPAD_CFG                                     (IMAP_VA_SYSMGR+0x048 )
#define rGPU_CFG                                     (IMAP_VA_SYSMGR+0x04C )
#define rDIV_CFG3                                    (IMAP_VA_SYSMGR+0x05C )
#define rDIV_CFG4                                    (IMAP_VA_SYSMGR+0x060 )

#define rSW_RST            			     (IMAP_VA_SYSMGR+0x100 )
#define rMEM_CFG           		 	     (IMAP_VA_SYSMGR+0x104 )
#define rMEM_SWAP          			     (IMAP_VA_SYSMGR+0x108 )
#define rBOOT_MD          	 		     (IMAP_VA_SYSMGR+0x10C )
#define rRST_ST            			     (IMAP_VA_SYSMGR+0x110 )
#define rPORT_PS_CFG       			     (IMAP_VA_SYSMGR+0x114 )
#define rIVA_PS_CFG	         	    	     (IMAP_VA_SYSMGR+0x118 )			
#define rSM_PS_CFG	         		     (IMAP_VA_SYSMGR+0x11C )	
#define rMP_ACCESS_CFG				     (IMAP_VA_SYSMGR+0x120 )			
#define rGPOW_CFG          			     (IMAP_VA_SYSMGR+0x200 )
#define rWP_MASK           			     (IMAP_VA_SYSMGR+0x204 )
#define rPOW_STB         			     (IMAP_VA_SYSMGR+0x208 )
#define rWP_ST             			     (IMAP_VA_SYSMGR+0x20C )
#define rNPOW_CFG          			     (IMAP_VA_SYSMGR+0x210 )
#define rPOW_ST            			     (IMAP_VA_SYSMGR+0x214 )
#define rMD_ISO           			     (IMAP_VA_SYSMGR+0x218 )
#define rMD_RST            			     (IMAP_VA_SYSMGR+0x21C )
#define rAHBP_RST            			     (IMAP_VA_SYSMGR+0x220 )
#define rAPBP_RST            			     (IMAP_VA_SYSMGR+0x224 )
#define rAHBP_EN            			     (IMAP_VA_SYSMGR+0x228 )

#define rINFO0            			     (IMAP_VA_SYSMGR+0x22C )
#define rINFO1            			     (IMAP_VA_SYSMGR+0x230 )
#define rINFO2            			     (IMAP_VA_SYSMGR+0x234 )
#define rINFO3            			     (IMAP_VA_SYSMGR+0x238 )

#define rSLP_ORST                        	     (IMAP_VA_SYSMGR+0x300 )
#define rSLP_GPADAT                      	     (IMAP_VA_SYSMGR+0x304 )
#define rSLP_GPAPUD                      	     (IMAP_VA_SYSMGR+0x308 )
#define rSLP_GPACON                      	     (IMAP_VA_SYSMGR+0x30C )
#define rSLP_GPBDAT                      	     (IMAP_VA_SYSMGR+0x310 )
#define rSLP_GPBPUD                      	     (IMAP_VA_SYSMGR+0x314 )
#define rSLP_GPBCON                      	     (IMAP_VA_SYSMGR+0x318 )
#define rSLP_GPODAT                      	     (IMAP_VA_SYSMGR+0x31C )
#define rSLP_GPOPUD                      	     (IMAP_VA_SYSMGR+0x320 )
#define rSLP_GPOCON                      	     (IMAP_VA_SYSMGR+0x324 )
#define rGPA_SLP_CTRL                    	     (IMAP_VA_SYSMGR+0x328 )
#define rGPB_SLP_CTRL                    	     (IMAP_VA_SYSMGR+0x32C )
#define rGPO_SLP_CTRL                    	     (IMAP_VA_SYSMGR+0x330 )
#define rRTC_INT_CFG                     	     (IMAP_VA_SYSMGR+0x334 )                                                          

/*clock div CFG bit*/
#define IMAP_DIV_CFG0_PCLK	(0x3<<16)
#define IMAP_DIV_CFG0_HCLKX2	(0xf<<8)
#define IMAP_DIV_CFG0_HCLK	(0xf<<4)
#define IMAP_DIV_CFG0_CPU	(0xf<<0)

#define IMAP_DIV_CFG1_TIM0      (0x3<<0)
#define IMAP_DIV_CFG1_TIMRAT0	(0x7<<2)


/*pll locked status bit field*/
#define IMAP_EPLL_LOCKED	(1<<2)
#define IMAP_DPLL_LOCKED	(1<<1)
#define IMAP_APLL_LOCLED	(1<<0)

/*Pll output selection*/
#define IMAP_EPiLL_OCLK_SEL	(1<<2)		//pll put
#define IMAP_DPLL_OCLK_SEL	(1<<1)		//pll out
#define IMAP_APLL_OCLK_SEL	(1<<0)		//pll out

/*pll clock source select*/
#define IMAP_EPLL_SEL		(1<<2)		//external clock
#define IMAP_DPLL_SEL		(1<<1)		//external clock
#define IMAP_APLL_SEL		(1<<0)		//external clock

/*PLL configuration*/
#define IMAP_PLL_EN_BIT		(1<<31)	
#define IMAP_PLL_BYPASS_EN_BIT	(1<<15)
#define IMAP_PLL_ODIV		(0x3<<12)
#define IMAP_PLL_DDIV		(0x1f<<7)
#define IMAP_PLL_MULTIV		(0x7f<<0)

/*GPU Ram Clock Mode CFG*/
#define IMAP_RAMCLKGDSB		(1<<0)		//memory clock gate disable

/*soft reset control bit*/
#define IMAP_SWRST		(0x6565)

/*HCLK gating control*/
#define IMAP_CLKCON_HCLK_IVA_AXI	(1<<24)
#define IMAP_CLKCON_HCLK_DSP_AHB	(1<<23)
#define IMAP_CLKCON_HCLK_MEMPOOL	(1<<22)
#define IMAP_CLKCON_HCLK_SDIO2		(1<<21)
#define IMAP_CLKCON_HCLK_SDIO1		(1<<20)
#define IMAP_CLKCON_HCLK_GPS		(1<<19)
#define IMAP_CLKCON_HCLK_ETH		(1<<18)
#define IMAP_CLKCON_HCLK_NAND		(1<<17)
#define IMAP_CLKCON_HCLK_NORFLASH	(1<<16)
#define IMAP_CLKCON_HCLK_INTC		(1<<15)
#define IMAP_CLKCON_HCLK_SDIO0		(1<<14)
#define IMAP_CLKCON_HCLK_CF_IDE		(1<<13)
#define IMAP_CLKCON_HCLK_APB		(1<<12)
#define IMAP_CLKCON_HCLK_OTG		(1<<11)
#define IMAP_CLKCON_HCLK_MONITOR_DEBUG	(1<<10)
#define IMAP_CLKCON_HCLK_EMIF		(1<<9)
#define IMAP_CLKCON_HCLK_USBH		(1<<8)
#define IMAP_CLKCON_HCLK_CAMIF		(1<<7)
#define IMAP_CLKCON_HCLK_IDS		(1<<6)
#define IMAP_CLKCON_HCLK_DMA		(1<<5)
#define IMAP_CLKCON_HCLK_DSP		(1<<4)
#define IMAP_CLKCON_HCLK_GPU		(1<<3)
#define IMAP_CLKCON_HCLK_VDEC		(1<<2)
#define IMAP_CLKCON_HCLK_VENC		(1<<1)

/*PCLK gating control */
#define IMAP_CLKCON_PCLK_RTC		(1<<20)
#define IMAP_CLKCON_PCLK_SPI		(1<<19)
#define IMAP_CLKCON_PCLK_SLAVESSI	(1<<18)
#define IMAP_CLKCON_PCLK_MASTERSSI2	(1<<17)
#define IMAP_CLKCON_PCLK_PS2_1		(1<<16)
#define IMAP_CLKCON_PCLK_PS2_0		(1<<15)
#define IMAP_CLKCON_PCLK_KB		(1<<14)
#define IMAP_CLKCON_PCLK_UART3		(1<<13)
#define IMAP_CLKCON_PCLK_UART2		(1<<12)
#define IMAP_CLKCON_PCLK_UART1		(1<<11)
#define IMAP_CLKCON_PCLK_UART0		(1<<10)
#define IMAP_CLKCON_PCLK_GPIO		(1<<9)
#define IMAP_CLKCON_PCLK_MASTERSSI1	(1<<8)
#define IMAP_CLKCON_PCLK_MASTERSSI0	(1<<7)
#define IMAP_CLKCON_PCLK_AC97		(1<<6)
#define IMAP_CLKCON_PCLK_IIS		(1<<5)
#define IMAP_CLKCON_PCLK_IIC1		(1<<4)
#define IMAP_CLKCON_PCLK_IIC0		(1<<3)
#define IMAP_CLKCON_PCLK_WATCHDOG	(1<<2)
#define IMAP_CLKCON_PCLK_PWM_TIEMR	(1<<1)
#define IMAP_CLKCON_PCLK_CMN_TIMER	(1<<0)

/*SCLK gating control*/
#define IMAP_CLKCON_SCLK_BOOTSRAM	(1<<17)
#define IMAP_CLKCON_SCLK_IDS_EITF	(1<<15)
#define IMAP_CLKCON_SCLK_IDS		(1<<14)
#define IMAP_CLKCON_SCLK_USB_PHY	(1<<13)
#define IMAP_CLKCON_SCLK_SDIO2		(1<<12)
#define IMAP_CLKCON_SCLK_SDIO1		(1<<11)
#define IMAP_CLKCON_SCLK_SDIO0		(1<<10)
#define IMAP_CLKCON_SCLK_TV		(1<<9)
#define IMAP_CLKCON_SCLK_TIM1		(1<<8)
#define IMAP_CLKCON_SCLK_TIM0		(1<<7)
#define IMAP_CLKCON_SCLK_CAM		(1<<6)
#define IMAP_CLKCON_SCLK_OTG		(1<<5)
#define IMAP_CLKCON_SCLK_USBH		(1<<4)
#define IMAP_CLKCON_SCLK_IIS		(1<<3)
#define IMAP_CLKCON_SCLK_GPU		(1<<2)
#define IMAP_CLKCON_SCLK_DSP		(1<<1)
#define IMAP_CLKCON_SCLK_UART		(1<<0)

#if 0
/*Wakeup source Mask*/

static inline unsigned int
imapx200_get_pll(unsigned long pllval, unsigned long baseclk)
{
	unsigned long odiv, ddiv, mdiv;
	
	/*To prevent overflow in calculation*/
	baseclk /= 1000;

	odiv = (pllval & (0x3<<12))>>12;
	ddiv = (pllval & (0x1f<<7))>>7;
	mdiv = (pllval & (0x7f<<0))>>0;
	return (baseclk*(2*(mdiv + 1))) / ((ddiv + 1)*(2<<(odiv)))*1000;
}
#endif
#endif
