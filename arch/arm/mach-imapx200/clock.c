/***************************************************************************** 
 * ** linux/arch/arm/mach-imapx200/clock.c
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** Author:
 * **     Alex Zhang   <tao.zhang@infotmic.com.cn>
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.2  25/11/2009  Alex Zhang
 * ** 1.3  21/01/2010  Raymond Wang 
 * *****************************************************************************/ 

#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/list.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/sysdev.h>
#include <linux/clk.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/serial_core.h>
#include <linux/io.h>

#include <asm/mach/map.h>

#include <mach/hardware.h>
#include <plat/cpu.h>
#include <plat/imapx200.h>
#include <plat/clock.h>
#include <plat/imapx.h>

#ifdef CONFIG_IMAP_FPGA
#include <plat/fpga_test.h>
#define XTERNAL_CYSTAL CONFIG_FPGA_EXT_CLOCK
#endif

void inline imap_clk_enable(uint clocks, uint enable, ulong gate_reg)
{
        unsigned long clkcon;
        unsigned long flags;

        local_irq_save(flags);

        clkcon = __raw_readl(gate_reg);

        if (enable)
                clkcon &= ~clocks;
        else
                clkcon |= clocks;

        __raw_writel(clkcon, gate_reg);

        local_irq_restore(flags);
}

/*This function returns the virtual address of gating register*/
ulong clk_get_gate_reg(struct clk *clk)
{
        struct clk *parent_clk = clk_get_parent(clk);

        if(strcmp(parent_clk->name, "hclk") == 0)
                return (unsigned long) rHCLK_MASK;
        else if(strcmp(parent_clk->name, "pclk") == 0)
                return (unsigned long) rPCLK_MASK;
        else if(strcmp(parent_clk->name, "hclkx2") == 0)
                return (unsigned long) rHCLK_MASK;
        else 
                return (unsigned long) rSCLK_MASK;
}

int imap_clkcon_enable(struct clk *clk, int enable)
{
        unsigned long gate_reg;

        gate_reg = clk_get_gate_reg(clk);
        imap_clk_enable(clk->ctrlbit, enable, gate_reg);
        return 0;
}

static unsigned long imapx200_clk_getrate(struct clk *clk)
{
	/*set clock rate to use in drivers*/
	if(!clk->rate){
		if(clk->parent){
			clk->rate = clk->parent->rate;
		}
	}
	return clk->rate;
}

static struct clk init_clocks[] = {
	{
		.name           = "mempool",
		.id             = -1,
		.parent         = &clk_h,
		.enable         = imap_clkcon_enable,
		.ctrlbit        = IMAP_CLKCON_HCLK_MEMPOOL,
	}, {
		.name           = "sdio2",
		.id             = 2,
		.parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_SDIO2,
        }, {
                .name           = "sdio1",
                .id             = 1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_SDIO1,
        }, {
                .name           = "eth",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_ETH,
        }, {
                .name           = "eth-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_ETH,
        }, {
                .name           = "intc",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_INTC,
        }, {
                .name           = "sdio0",
                .id             = 0,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_SDIO0,
        }, {
                .name           = "cfide",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_CF_IDE,
        }, {
                .name           = "cfide-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_CF_IDE,
        }, {
                .name           = "gpio",
                .id             = -1,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_GPIO,
        }, {
                .name           = "otg",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_OTG,
        }, {
                .name           = "usb-device",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_OTG,
        }, {
                .name           = "emif",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_EMIF,
        }, {
                .name           = "rtc",
                .id             = -1,
                .parent         = &clk_p,
				.enable			= imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_RTC,
        }, {
                .name           = "i2s",
                .id             = -1,
                .parent         = &clk_epll/*&clk_p*/,
                .ctrlbit        = IMAP_CLKCON_SCLK_IIS/*IMAP_CLKCON_PCLK_IIS*/,
        }, {
                .name           = "usb-host",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_USBH,
        }, {
                .name           = "ids",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_IDS,
        }, {
                .name           = "ids-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_IDS,
        }, {
                .name           = "dsp",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_DSP,
        }, {
                .name           = "dsp-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_DSP,
        }, {
                .name           = "uart0",
                .id             = 0,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_UART0,
        }, {
                .name           = "uart1",
                .id             = 1,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_UART1,
        }, {
                .name           = "uart2",
                .id             = 2,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_UART2,
        }, {
                .name           = "uart3",
                .id             = 3,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_UART3,
	}, {
		.name           = "cmn-timer",
		.id             = -1,
		.parent         = &clk_p,
		.enable         = imap_clkcon_enable,
		.ctrlbit        = IMAP_CLKCON_PCLK_CMN_TIMER,
	}, {
		.name           = "dma",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_DMA,
        },{
                .name           = "kb",
                .id             = -1,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_KB,
        },
};

static struct clk init_clocks_disable[] = {
        {
                .name           = "gps",
                .id             = -1,
                .parent         = &clk_h,
		.enable		= imap_clkcon_enable,
		.ctrlbit	= IMAP_CLKCON_HCLK_GPS,
        }, {
                .name           = "gps-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_GPS,
        }, {
                .name           = "nand",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_NAND,
        }, {
                .name           = "norflash",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_NORFLASH,
        }, {
                .name           = "camif",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_CAMIF,
        }, {
                .name           = "camif-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_CAMIF,
        }, {
                .name           = "gpu",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_GPU,
        }, {
                .name           = "gpu-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_GPU,
        }, {
                .name           = "vdec",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_VDEC,
        }, {
                .name           = "vdec-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_VDEC,
        }, {
                .name           = "venc",
                .id             = -1,
                .parent         = &clk_h,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_VENC,
        }, {
                .name           = "venc-hx2",
                .id             = -1,
                .parent         = &clk_hx2,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_HCLK_VENC,
        }, {
                .name           = "spi",
                .id             = 0,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_SPI,
        }, {
                .name           = "spi",
                .id             = 4,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_SPI,
        }, {
                .name           = "ac97",
                .id             = -1,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_AC97,
        }, {
                .name           = "i2c0",
                .id             = -1,
                .parent         = &clk_p,
                .enable         = imap_clkcon_enable,
                .ctrlbit        = IMAP_CLKCON_PCLK_IIC0,
	}, {
		.name           = "i2c1",
		.id             = -1,
		.parent         = &clk_p,
		.enable         = imap_clkcon_enable,
		.ctrlbit        = IMAP_CLKCON_PCLK_IIC1,
	},
};


/*Wakeup source Mask*/

static inline unsigned int 
imapx200_get_pll(unsigned long pllval, unsigned long baseclk)
{
        unsigned long odiv, ddiv, mdiv;
    
        /*To prevent overflow in calculation*/
        baseclk /= 1000;

        odiv = (pllval & IMAP_PLL_ODIV)>>12;
        ddiv = (pllval & IMAP_PLL_DDIV)>>7;
        mdiv = (pllval & IMAP_PLL_MULTIV);
        return (baseclk*(2*(mdiv + 1))) / ((ddiv + 1)*(1<<(odiv)))*1000;
}


#if defined (FPGA_TEST)
static int imapx200_setrate_ext_clk(struct clk *clk, unsigned long rate)
{
	return 0;
}

/*function for UART clock with external cystal*/
static unsigned long imapx200_ext_get_clk(struct clk *clk)
	
{
#if 0
	unsigned long ext_con;
	unsigned long od = 0;
	unsigned long div = 0;
	unsigned long mult = 0;
	unsigned long ret;

	ext_con = (readl(IMAP_DPLL_CFG));
	ret =  ext_con | (0x07 << 16);
	return ret;
#endif
	return 0;
}	


static unsigned long imapx200_getrate_ext_clk(struct clk *clk){
	imapx200_setrate_ext_clk(clk,0);
	return imapx200_ext_get_clk(clk);
}


static struct clk clk_ext_uart = {
	.name		= "ext_uart",
	.id		= -1,
	.parent		= &clk_xext,
	.enable		= imap_clkcon_enable,
	.ctrlbit	= IMAP_CLKCON_SCLK_UART,
	.set_rate	= imapx200_setrate_ext_clk,
	//.get_rate	= imapx200_getrate_ext_clk,
};
#endif

static struct clk clk_ext_lcd = {
	.name           = "lcd",
	.id             = -1,
	.parent		= &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_IDS_EITF,
};

static struct clk clk_ext_osd = {
	.name           = "osd",
	.id             = -1,
	.parent		= &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_IDS,
};

static struct clk clk_ext_timer0 = {
	.name           = "timer0",
	.id             = -1,
	.parent		= &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_TIM0,
};

static struct clk clk_ext_gpu = {
	.name           = "gpu-ext",
	.id             = -1,
	.parent		    = &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_GPU,
};

static struct clk clk_ext_sdio0 = {
	.name           = "sdio0-ext",
	.id             = -1,
	.parent         = &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_SDIO0,
};
static struct clk clk_ext_sdio1 = {
	.name		    = "sdio1-ext",
	.id		        = -1,
	.parent         = &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_SDIO1,
};
static struct clk clk_ext_sdio2 = {
	.name		    = "sdio2-ext",
	.id		        = -1,
	.parent         = &clk_epll,
	.enable         = imap_clkcon_enable,
	.ctrlbit        = IMAP_CLKCON_SCLK_SDIO2,
};


#if defined (FPGA_TEST)
static struct clk *clks[] __initdata = {
 &clk_ext_uart,
};
#else

#define CLK_EXT_DISABLE_OFFSET 2
static struct clk *clks[] __initdata = {
	&clk_ext_lcd,
	&clk_ext_osd,
	&clk_ext_timer0,
	&clk_ext_gpu,
	&clk_ext_sdio0,
	&clk_ext_sdio1,
	&clk_ext_sdio2,
};
#endif

void __init imapx200_init_clocks(int xtal)
{

	unsigned long apll_clk, dpll_clk, epll_clk;
	unsigned long cpu_clk;
	unsigned long hclk;
	unsigned long pclk;
	unsigned long hclkx2;

	struct clk *clkp;
	int ret;
	int ptr;

	/* initalise all the core clocks */
	imap_register_coreclks(xtal);

#if defined (FPGA_TEST)
        apll_clk = XTERNAL_CYSTAL;
        dpll_clk = XTERNAL_CYSTAL;
        epll_clk = XTERNAL_CYSTAL;

	cpu_clk = XTERNAL_CYSTAL;
	hclk = XTERNAL_CYSTAL;
	pclk = XTERNAL_CYSTAL;
	hclkx2 = XTERNAL_CYSTAL;
#else
	unsigned long clkdiv0;

	/* get pll clocks */	
	apll_clk = imapx200_get_pll(__raw_readl(rAPLL_CFG), xtal);
	dpll_clk = imapx200_get_pll(__raw_readl(rDPLL_CFG), xtal);
	epll_clk = imapx200_get_pll(__raw_readl(rEPLL_CFG), xtal);
	

	clkdiv0 = __raw_readl(rDIV_CFG0);

	/* read cpu sync mode register, 
	   set cpu_clk from dpll when it is in async mode */
	if(!(__raw_readl(rCPUSYNC_CFG) & 0x1))
	{
		cpu_clk = apll_clk/(clkdiv0 & IMAP_DIV_CFG0_CPU);
	}
	else
	{
		cpu_clk = dpll_clk/(clkdiv0 & IMAP_DIV_CFG0_CPU);
	}

	/* set hclkx2, hclk and pclk from apll*/
	hclk = apll_clk/((clkdiv0 & IMAP_DIV_CFG0_HCLK)>>4);
	pclk = hclk/(1<<((clkdiv0 & IMAP_DIV_CFG0_PCLK)>>16));
	hclkx2 = apll_clk/((clkdiv0 & IMAP_DIV_CFG0_HCLKX2)>>8);
#endif

	clk_apll.rate = apll_clk;
	clk_dpll.rate = dpll_clk;
	clk_epll.rate = epll_clk;

	printk("IMAPX200: PLL CLOCKS, APLL %ld.%03ld MHz, DPLL %ld.%03ld MHz, EPLL %ld.%03ld MHz\n",
			print_mhz(apll_clk), print_mhz(dpll_clk), print_mhz(epll_clk));

	clk_c.rate = cpu_clk;
	clk_h.rate = hclk;
	clk_p.rate = pclk;
	clk_hx2.rate = hclkx2;

	printk("IMAPX200: CPU_CLK %ld.%03ld MHz, HCLK %ld.%03ld MHz, PCLK %ld.%03ld MHz\n",
			print_mhz(cpu_clk), print_mhz(hclk), print_mhz(pclk));


	/* registe clock arrays, which will be used on our platform */

	for (ptr = 0; ptr<ARRAY_SIZE(clks); ptr ++){
		clkp = clks[ptr];

		ret = imap_register_clock(clkp);
		if(ret < 0){
			printk(KERN_ERR "Failed to register clok %s (%d)\n",
				clkp->name, ret);
		}

		if(ptr >= CLK_EXT_DISABLE_OFFSET)
		    (clkp->enable)(clkp, 0);
	}

	clkp = init_clocks;
	for(ptr = 0; ptr < ARRAY_SIZE(init_clocks); ptr++,clkp++){
		ret = imap_register_clock(clkp);
		if(ret < 0){
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
					clkp->name, ret);
		}
		if(!clkp->rate){
			if(clkp->parent){
				clkp->rate = clkp->parent->rate;
			}
		}
	}

	clkp = init_clocks_disable;
	for(ptr = 0; ptr < ARRAY_SIZE(init_clocks_disable); ptr++, clkp++){
	
		ret = imap_register_clock(clkp);
		if(ret < 0){
			printk(KERN_ERR "Failed to register clock %s (%d)\n",
					clkp->name, ret);
		}
		(clkp->enable)(clkp, 0);
	}
}
