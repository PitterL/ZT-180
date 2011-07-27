/***************************************************************************** 
 * ** linux/arch/arm/plat-imap/include/plat/regs-clock.h 
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: IMAPX200 System Controller register definitons.
 * **
 * ** Author:
 * **     Alex Zhang   <tao_zhang@infotm.com>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.1  17/09/2009  Alex Zhang   
 * *****************************************************************************/

#ifndef __ASM_ARM_REGS_CLOCK
#define __ASM_ARM_REGS_CLOCK

#define IMAP_CLKREG(x) ((x) + IMAP_VA_SYSMGR)

#define IMAP_PLL_EN	(1<<31)
#define IMAP_PLLVAL(_o,_d,_m) ((_o) << 12 | (_d) << 7 | ((_m)))

#define IMAP_PLL_LOCKED		IAMP_CLKREG(0x04)
#define IMAP_PLL_OCLKSEL	IMAP_CLKREG(0x08)
#define IMAP_PLL_CLKSEL		IMAP_CLKREG(0x0c)
#define IMAP_APLL_CFG		IMAP_CLKREG(0x10)
#define IMAP_DPLL_CFG		IMAP_CLKREG(0x14)
#define IMAP_EPLL_CFG		IMAP_CLKREG(0x18)
#define IMAP_DIV_CFG0		IMAP_CLKREG(0x1c)
#define IMAP_DIV_CFG1		IMAP_CLKREG(0x20)
#define IMAP_HCLK_MASK		IMAP_CLKREG(0x24)
#define IMAP_PCLK_MASK		IMAP_CLKREG(0x28)
#define IMAP_SCLK_MASK		IMAP_CLKREG(0x2c)
#define IMAP_CLKOUT0_CFG	IMAP_CLKREG(0x30)
#define IMAP_CLKOUT1_CFG	IMAP_CLKREG(0x34)
#define IMAP_CPUSYNC_CFG	IMAP_CLKREG(0x38)
#define IMAP_DIV_CFG2		IMAP_CLKREG(0x3c)
#define IMAP_USB_SRST		IMAP_CLKREG(0x40)
#define IMAP_PERSIM_CFG		IMAP_CLKREG(0x44)
#define IMAP_PAD_CFG		IMAP_CLKREG(0x48)
#define IMAP_GPUCFG		IMAP_CLKREG(0x4c)

#define IMAP_DIV_CFG3		IMAP_CLKREG(0x5c)
#define IMAP_DIV_CFG4		IMAP_CLKREG(0x60)

#define IMAP_SW_RST		IMAP_CLKREG(0x100)
#define IMAP_MEM_CFG		IMAP_CLKREG(0x104)
#define IMAP_MEM_SWAP		IMAP_CLKREG(0x108)
#define IMAP_BOOT_MD		IMAP_CLKREG(0x10c)
#define IMAP_RST_ST		IMAP_CLKREG(0x110)
#define IMAP_PORT_PS_CFG	IMAP_CLKREG(0x114)
#define IMAP_IVA_PS_CFG		IMAP_CLKREG(0x118)
#define IMAP_SM_PS_CFG		IMAP_CLKREG(0x11c)

#define IMAP_GPOW_CFG		IMAP_CLKREG(0x200)
#define IMAP_WP_MASK		IMAP_CLKREG(0x204)
#define IMAP_POW_STB		IMAP_CLKREG(0x208)
#define IMAP_WP_ST		IMAP_CLKREG(0x20c)
#define IMAP_NPOW_CFG		IMAP_CLKREG(0x210)
#define IMAP_POW_ST		IMAP_CLKREG(0x214)
#define IMAP_MD_ISO		IMAP_CLKREG(0x218)
#define IMAP_MD_RST		IMAP_CLKREG(0x21c)
#define IMAP_AHBP_RST		IMAP_CLKREG(0x220)
#define IMAP_APBP_RST		IMAP_CLKREG(0x224)
#define IMAP_AHBP_ENABLE	IMAP_CLKREG(0x228)
#define IMAP_INFO0		IMAP_CLKREG(0x22c)
#define IMAP_INFO1		IMAP_CLKREG(0x230)
#define IMAP_INFO2		IMAP_CLKREG(0x234)
#define IMAP_INFO3		IMAP_CLKREG(0x238)

#define IMAP_SLP_ORST		IMAP_CLKREG(0x300)
#define IMAP_SLP_GPADAT		IMAP_CLKREG(0x304)
#define IMAP_SLP_GPAPUD		IMAP_CLKREG(0x308)
#define IMAP_SLP_GPACON		IMAP_CLKREG(0x30c)
#define IMAP_SLP_GPBDAT		IMAP_CLKREG(0x310)
#define IMAP_SLP_GPBPUD		IMAP_CLKREG(0x314)
#define IMAP_SLP_GPBCON		IMAP_CLKREG(0x318)
#define IMAP_SLP_GPODAT		IMAP_CLKREG(0x31c)
#define IMAP_SLP_GPOPUD		IMAP_CLKREG(0x320)
#define IMAP_SLP_GPOCON		IMAP_CLKREG(0x324)
#define IMAP_GPA_SLP_CTRL	IMAP_CLKREG(0x328)
#define IMAP_GPB_SLP_CTRL	IMAP_CLKREG(0x32c)
#define IMAP_GPO_SLP_CTRL	IMAP_CLKREG(0x330)
#define IMAP_TRC_INT_CFG	IMAP_CLKREG(0x334)


/*clock div CFG bit*/
#define IMAP_DIV_CFG0_PCLK	(0x3<<16)
#define IMAP_DIV_CFG0_HCLKX2	(0xf<<8)
#define IMAP_DIV_CFG0_HCLK	(0xf<<4)
#define IMAP_DIV_CFG0_CPU	(0xf<<0)


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
#define IMAP_PLL_EN_BIT	(1<<31)	
#define IMAP_PLL_BYPASS_EN_BIT	(1<<15)

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
#define IMAP_CLKCON_SCLK_SD2		(1<<12)
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
#define IMAP_CLKCON_SCLK_UART		(1<<0)

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
