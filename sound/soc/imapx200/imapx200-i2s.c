/********************************************************************************
** linux-2.6.28.5/sound/soc/imapx200/imapx200-ac97.c
**
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Author:
**     James Xu   <James Xu@infotmic.com.cn>
**
** Revision History:
**     1.0  09/15/2009    James Xu
********************************************************************************/

#include <linux/init.h>
#include <linux/module.h>
#include <linux/device.h>
#include <linux/delay.h>
#include <linux/clk.h>
#include <linux/i2c.h>
//#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <asm/dma.h>
#include <plat/gpio-bank-d.h>
#include <mach/imapx_gpio.h>
#include <mach/imapx_iis.h>
//#include <mach/regs-gpio.h>
#include "imapx200-pcm.h"
#include "imapx200-i2s.h"
#include <mach/imapx_base_reg.h>
#include <plat/dma.h>
//#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define imapx200(x...) printk(x)
#else
#define imapx200(x...)
#endif
//#define FPGA_TEST
#define IIS_PCLK 48000000
/* used to disable sysclk if external crystal is used */
static int extclk = 0;
module_param(extclk, int, 0);
MODULE_PARM_DESC(extclk, "set to 1 to disable imapx200 i2s sysclk");
struct imapx200_dma_ch imapx200_pch[2] ={
	{
		
			.block_size = 4096,
			.src_addr = 0,
			.dst_addr = IIS_BASE_REG_PA + rTXDMA,
			.src_datawidth = 16,
			.dst_datawidth = 16,
			.src_burstlen = 1,
			.dst_burstlen = 1,
			.flow_ctrl = FC_DMA_M2P,
			.src_incre = INCRE_ADD,
			.dst_incre = INCRE_CONST,
			.intr_en = 1,
			.hs_src_type = HANDSHAKE_NO ,
			.hs_src_index = 0,
			.hs_dst_type = HANDSHAKE_HARD,
			.hs_dst_index = HANDSHAKE_INDEX_IIS0_TX,
			.trans_type = TRANS_TYPE_MULTI,
			.reload_type = RELOAD_LLI_SRC|RELOAD_LLI_DST,
			.enable_2d = 0,
			.src2d_size = 0,
			.dst2d_size = 0,
			.src2d_interval = 0,
			.dst2d_interval = 0,
			.enable_wb = 0,
			.srcwb_addr = 0,
			.dstwb_addr = 0
	},
	{
		
			.block_size = 4096,
			.src_addr = 0,
			.dst_addr = IIS_BASE_REG_PA + rTXDMA,
			.src_datawidth = 16,
			.dst_datawidth = 16,
			.src_burstlen = 1,
			.dst_burstlen = 1,
			.flow_ctrl = FC_DMA_M2P,
			.src_incre = INCRE_ADD,
			.dst_incre = INCRE_CONST,
			.intr_en = 1,
			.hs_src_type = HANDSHAKE_NO ,
			.hs_src_index = 0,
			.hs_dst_type = HANDSHAKE_HARD,
			.hs_dst_index = HANDSHAKE_INDEX_IIS0_TX,
			.trans_type = TRANS_TYPE_MULTI,
			.reload_type = RELOAD_LLI_SRC|RELOAD_LLI_DST,
			.enable_2d = 0,
			.src2d_size = 0,
			.dst2d_size = 0,
			.src2d_interval = 0,
			.dst2d_interval = 0,
			.enable_wb = 0,
			.srcwb_addr = 0,
			.dstwb_addr = 0
	}
}; 
struct imapx200_dma_ch imapx200_pch_in[2] ={
	{
		
			.block_size = 4096,
			.dst_addr = 0,
			//.dst_addr = IIS_BASE_REG_PA + rRXDMA,
			.src_addr = IIS_BASE_REG_PA + rRXDMA,
			.src_datawidth = 16,
			.dst_datawidth = 16,
			.src_burstlen = 1,
			.dst_burstlen = 1,
			.flow_ctrl = FC_DMA_P2M,
			.dst_incre = INCRE_ADD,
			.src_incre = INCRE_CONST,
			.intr_en = 1,
			.hs_src_type = HANDSHAKE_HARD,
			.hs_src_index = HANDSHAKE_INDEX_IIS0_RX,
			.hs_dst_type = HANDSHAKE_NO,
			.hs_dst_index = 0,
			.trans_type = TRANS_TYPE_MULTI,
			.reload_type = RELOAD_LLI_DST|RELOAD_LLI_SRC,
			.enable_2d = 0,
			.src2d_size = 0,
			.dst2d_size = 0,
			.src2d_interval = 0,
			.dst2d_interval = 0,
			.enable_wb = 0,
			.srcwb_addr = 0,
			.dstwb_addr = 0
	},{
		
			.block_size = 4096,
			.dst_addr = 0,
			//.dst_addr = IIS_BASE_REG_PA + rRXDMA,
			.src_addr = IIS_BASE_REG_PA + rRXDMA,
			.src_datawidth = 16,
			.dst_datawidth = 16,
			.src_burstlen = 1,
			.dst_burstlen = 1,
			.flow_ctrl = FC_DMA_P2M,
			.dst_incre = INCRE_ADD,
			.src_incre = INCRE_CONST,
			.intr_en = 1,
			.hs_src_type = HANDSHAKE_HARD ,
			.hs_src_index = HANDSHAKE_INDEX_IIS0_RX,
			.hs_dst_type = HANDSHAKE_NO,
			.hs_dst_index = 0,
			.trans_type = TRANS_TYPE_MULTI,
			.reload_type = RELOAD_LLI_DST|RELOAD_LLI_SRC,
			.enable_2d = 0,
			.src2d_size = 0,
			.dst2d_size = 0,
			.src2d_interval = 0,
			.dst2d_interval = 0,
			.enable_wb = 0,
			.srcwb_addr = 0,
			.dstwb_addr = 0
	}

}; 

static struct imapx200_dma_client imapx200_dma_client_out = {
	.name = "I2S PCM Stereo out",
	.type =	IMAPX200_DMA_LLI, 
	.block_num = 2,
	.pch = &imapx200_pch,
	.is_loop = 1,
};
/**********************************************************/
static u32 rIER_pm;
static u32 rIRER_pm;
static u32 rITER_pm;
static u32 rCER_pm;
static u32 rCCR_pm;
static u32 rCDR_pm;
static u32 rRER_pm;
static u32 rTER_pm;
static u32 rRCR_pm;
static u32 rTCR_pm;
static u32 rIMR_pm;
static u32 rRFCR_pm; 
static u32 rTFCR_pm;
/**********************************************************/
static struct imapx200_dma_client imapx200_dma_client_in = {
	.name = "I2S PCM Stereo in",
	.type =	IMAPX200_DMA_LLI, 
	.block_num = 2,
	.pch = &imapx200_pch_in,
	.is_loop = 1,
};

static struct imapx200_pcm_dma_params imapx200_i2s_pcm_stereo_out = {
	.client		= &imapx200_dma_client_out,
	/****************************************************************/
	.channel	= 0,

	.dma_addr	= IIS_BASE_REG_PA + rTXDMA,
	.dma_size	= 16,
	/****************************************************************/
};

static struct imapx200_pcm_dma_params imapx200_i2s_pcm_stereo_in = {
	.client		= &imapx200_dma_client_in,
	/***************************************************************/
	.channel	= 0,

	.dma_addr	= IIS_BASE_REG_PA + rRXDMA,
	.dma_size	= 16,
	/**************************************************************/
};

struct imapx200_i2s_info {
	void __iomem	*regs;
	struct clk	*iis_clk;
	unsigned int    iis_clock_rate;
	int master;
};
static struct imapx200_i2s_info imapx200_i2s;
/**********************************************************
 * in this funciton, if in on condition, the program active the iis interface, start the operation.
 * If in off condition, the program disable the iis interface.
*/
static void imapx200_snd_txctrl(int on)
{
	int rITER_value, rTER0_value, rIRER_value, rRER0_value, rCER_value, rIER_value;  
	int rIMR0_value, rTFF0_value ;
	imapx200("Entered %s : on = %d \n", __FUNCTION__, on);

	if (on) {
		rITER_value = 0x1;         	//iis transmitter block enable register
		rTER0_value = 0x1;		//iis transmitter enable register0	
		rCER_value = 0x1;		//clock enable register
		rIER_value = 0x1;		//iis enable register
	       	writel(rITER_value, imapx200_i2s.regs +  rITER);	
	       	writel(rTER0_value, imapx200_i2s.regs + rTER0);	
		writel(rCER_value, imapx200_i2s.regs + rCER);
		writel(rIER_value, imapx200_i2s.regs + rIER);
	} else {
		/*
		rIMR0_value = (0x1<<5) | (0x1<<4)| (0x1<<1) | 0x1;
		writel(rIMR0_value, imapx200_i2s.regs + rIMR0);
		rTFF0_value = 0x1; //flush the tx fifo
		rITER_value = 0x0;         	//iis transmitter block enable register
		rTER0_value = 0x0;		//iis transmitter enable register0	
		rCER_value = 0x0;		//clock enable register
		rIER_value = 0x0;		//iis enable register
	       	writel(rTFF0_value, imapx200_i2s.regs + rTFF0);	
	       	writel(rITER_value, imapx200_i2s.regs + rITER);	
	       	writel(rTER0_value, imapx200_i2s.regs + rTER0);	
		writel(rCER_value, imapx200_i2s.regs + rCER);
		writel(rIER_value, imapx200_i2s.regs + rIER);
*/
	}

}
/**********************************************************
 * in this funciton, if in on condition, the program active the iis interface, start the operation.
 * If in off condition, the program disable the iis interface.
*/
static void imapx200_snd_rxctrl(int on)
{
	int rITER_value, rTER0_value, rIRER_value, rRER0_value, rCER_value, rIER_value;  
	int rIMR0_value, rRFF0_value ;
	imapx200("Entered %s : on = %d \n", __FUNCTION__, on);

	if (on) {
		rIRER_value = 0x1;         	//iis transmitter block enable register
		rRER0_value = 0x1;		//iis transmitter enable register0	
		rCER_value = 0x1;		//clock enable register
		rIER_value = 0x1;		//iis enable register
	       	writel(rIRER_value, imapx200_i2s.regs + rIRER);	
	       	writel(rRER0_value, imapx200_i2s.regs + rRER0);	
		writel(rCER_value, imapx200_i2s.regs +rCER);
		writel(rIER_value, imapx200_i2s.regs +rIER);
	} else {
		rIMR0_value = (0x1<<5) | (0x1<<4)| (0x1<<1) | 0x1;
		writel(rIMR0_value, imapx200_i2s.regs + rIMR0);
		rRFF0_value = 0x1; //flush the tx fifo
		rIRER_value = 0x0;         	//iis transmitter block enable register
		rRER0_value = 0x0;		//iis transmitter enable register0	
		rCER_value = 0x0;		//clock enable register
		rIER_value = 0x0;		//iis enable register
	       	writel(rRFF0_value, imapx200_i2s.regs + rRFF0);	
	       	writel(rIRER_value, imapx200_i2s.regs + rIRER);	
	       	writel(rRER0_value, imapx200_i2s.regs + rRER0);	
		writel(rCER_value, imapx200_i2s.regs + rCER);
		writel(rIER_value, imapx200_i2s.regs + rIER);

	}

}


static int imapx200_i2s_set_fmt(struct snd_soc_cpu_dai *cpu_dai,
		unsigned int fmt)
{

	return 0;

}

static int imapx200_i2s_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	int rITER_value, rTER0_value, rIRER_value, rRER0_value, rCER_value, rIER_value;  
	int rIMR0_value, rTFF0_value ;
	unsigned long iiscon;
	unsigned long iismod;
	unsigned long iisfcon;
	unsigned long rI2S_PRESCALER_value;	
	unsigned int rCCR_value, rTCR0_value, rRCR0_value, rTFCR0_value, rRFCR0_value;
	imapx200("Entered %s\n", __FUNCTION__);

	

	imapx200("substream->stream : %d\n", substream->stream);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		rtd->dai->cpu_dai->dma_data = &imapx200_i2s_pcm_stereo_out;
	} else {
		rtd->dai->cpu_dai->dma_data = &imapx200_i2s_pcm_stereo_in;
	}
	rIMR0_value = (0x1<<5) | (0x1<<1);
	writel(rIMR0_value, imapx200_i2s.regs + rIMR0);
	/*
	rI2S_PRESCALER_value = readl(imapx200_i2s.regs + rI2S_PRESCALER);
	rI2S_PRESCALER_value |= (cdclkdiv<<16) | iisclkdiv;
	writel(rI2S_PRESCALER_value, rI2S_PRESCALER);
	*/
	rTFCR0_value = readl(imapx200_i2s.regs + rTFCR0);
	rTFCR0_value = 0x8;
	writel(rTFCR0_value, imapx200_i2s.regs + rTFCR0);
	rRFCR0_value = readl(imapx200_i2s.regs + rRFCR0);
	rRFCR0_value = 0x8;
	writel(rRFCR0_value, imapx200_i2s.regs + rRFCR0);
	rCCR_value = readl(imapx200_i2s.regs + rCCR);
	rTCR0_value = readl(imapx200_i2s.regs + rTCR0);
	rRCR0_value = readl(imapx200_i2s.regs + rRCR0);
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S8:
		rCCR_value = 0x3 <<3;
		rTCR0_value = 0x0;
		rRCR0_value = 0x0;
		break;
	case SNDRV_PCM_FORMAT_S16_LE:
		rCCR_value = 0x0 <<3;
		rTCR0_value = 0x02;
		rRCR0_value = 0x02;
		break;

	case SNDRV_PCM_FORMAT_S24_LE:
		rCCR_value = 0x1 <<3;
		rTCR0_value = 0x04;
		rRCR0_value = 0x04;
		break;
	case SNDRV_PCM_FORMAT_S32_LE:
		rCCR_value = 0x2 <<3;
		rTCR0_value = 0x05;
		rRCR0_value = 0x05;
		break;

	default:
		return -EINVAL;
	}

	writel(rCCR_value, imapx200_i2s.regs + rCCR);
	writel(rTCR0_value, imapx200_i2s.regs + rTCR0);
	writel(rRCR0_value, imapx200_i2s.regs + rRCR0);

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		rITER_value = 0x1;         	//iis transmitter block enable register
		rTER0_value = 0x1;		//iis transmitter enable register0	
		rIER_value = 0x1;		//iis enable register
	       	writel(rITER_value, imapx200_i2s.regs +  rITER);	
	       	writel(rTER0_value, imapx200_i2s.regs + rTER0);	
		writel(rIER_value, imapx200_i2s.regs + rIER);
	}else {
		rIRER_value = 0x1;         	//iis transmitter block enable register
		rRER0_value = 0x1;		//iis transmitter enable register0	
		rIER_value = 0x1;		//iis enable register
	       	writel(rIRER_value, imapx200_i2s.regs + rIRER);	
	       	writel(rRER0_value, imapx200_i2s.regs + rRER0);	
		writel(rIER_value, imapx200_i2s.regs +rIER);
	}
	return 0;

}

static int imapx200_i2s_trigger(struct snd_pcm_substream *substream, int cmd)
{
	int ret = 0;

	imapx200("Entered %s: cmd = %d\n", __FUNCTION__, cmd);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		printk(KERN_INFO "trigger command is %d\n", cmd);
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			imapx200_snd_rxctrl(1);
		else
			imapx200_snd_txctrl(1);
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		
		printk(KERN_INFO "trigger command is %d\n", cmd);
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			imapx200_snd_rxctrl(0);
		else
			imapx200_snd_txctrl(0);
		break;
	default:
		ret = -EINVAL;
		break;
	}

exit_err:
	return ret;
}

static void imapx20064xx_i2s_shutdown(struct snd_pcm_substream *substream)
{
	unsigned long iismod, iiscon;
	int rITER_value, rTER0_value, rIRER_value, rRER0_value, rCER_value, rIER_value;  
	int rIMR0_value, rRFF0_value, rTFF0_value ;
	imapx200("Entered %s\n", __FUNCTION__);
	rIMR0_value = (0x1<<5) | (0x1<<4)| (0x1<<1) | 0x1;
	writel(rIMR0_value, imapx200_i2s.regs + rIMR0);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
	rTFF0_value = 0x1; //flush the tx fifo
	rITER_value = 0x0;         	//iis transmitter block enable register
	rTER0_value = 0x0;		//iis transmitter enable register0	
	writel(rTFF0_value, imapx200_i2s.regs + rTFF0);	
	writel(rITER_value, imapx200_i2s.regs + rITER);	
	writel(rTER0_value, imapx200_i2s.regs + rTER0);	
	}
	else {
	rRFF0_value = 0x1; //flush the tx fifo
	rIRER_value = 0x0;         	//iis transmitter block enable register
	rRER0_value = 0x0;		//iis transmitter enable register0	
	writel(rRFF0_value, imapx200_i2s.regs + rRFF0);	
	writel(rIRER_value, imapx200_i2s.regs + rIRER);	
	writel(rRER0_value, imapx200_i2s.regs + rRER0);
	}
	rCER_value = 0x0;		//clock enable register
	rIER_value = 0x0;		//iis enable register
	writel(rCER_value, imapx200_i2s.regs + rCER);
	writel(rIER_value, imapx200_i2s.regs + rIER);
/******************************************************************/
	//clock disable
	//gate disable
	//pll disable
/******************************************************************/

}


/*
 * Set S3C24xx Clock source
 */
static int imapx200_i2s_set_sysclk(struct snd_soc_cpu_dai *cpu_dai,
	int clk_id, unsigned int freq, int format)
{
#if 0	

	int iisclkdiv, cdclkdiv, format_value;
	int rI2S_PRESCALER_value;
	imapx200("Entered %s : clk_id = %d, freq is %d, format is %d\n", __FUNCTION__, clk_id, freq, format);
	/***********************************************************/
	//here, the IIS_PCLK is not decided.
	switch(format){
		case 0:
		case 1:
			format_value = 8;
			break;
		case 2:
		case 3:
		case 4:
		case 5:
			format_value = 16;
			break;
		case 6:
		case 7:
		case 8:
		case 9:
			format_value = 24;
			break;
		case 10:
		case 11:
		case 12:
		case 13:
			format_value = 32;
			break;


	}

#ifdef FPGA_TEST
	iisclkdiv = (IIS_PCLK - (format_value<<1)*freq)/((format_value<<1)*freq);
	cdclkdiv  = (IIS_PCLK - (freq*clk_id))/(freq*clk_id);
#else
	iisclkdiv = (imapx200_i2s.iis_clock_rate - (format_value<<1)*freq)/((format_value<<1)*freq);
	cdclkdiv  = (imapx200_i2s.iis_clock_rate - (freq*clk_id))/(freq*clk_id);
	//printk("i2s clkdiv = %d, cdclkdiv = %d\n", iisclkdiv, cdclkdiv);
#endif
	/***********************************************************/
	iisclkdiv = iisclkdiv/2;
	rI2S_PRESCALER_value = readl(imapx200_i2s.regs + rI2S_PRESCALER);
	rI2S_PRESCALER_value = ((cdclkdiv<<16) | iisclkdiv);
	writel(rI2S_PRESCALER_value, imapx200_i2s.regs + rI2S_PRESCALER);
#else
    u32 base_clk;
    u32 div,div_m,div_b;
    
    //change to eclk,divide to 12Mhz    
    base_clk=imapx200_i2s.iis_clock_rate>>2;

    div = (44100*272);
    div_m = (base_clk-(div>>1))/div;
    div = ((16*2)*44100);
    div_b = (base_clk-(div>>1))/div;

    printk("base_clk %d MCLK %d (div %d) SCLK %d (div %d)\r\n",
        base_clk,base_clk/(div_m+1),div_m,base_clk/(div_b+1),div_b);

    writel((1<<26)|(1<<24)|  //select system clock
            (div_m<<16)|    
            (div_b<<0), imapx200_i2s.regs + rI2S_PRESCALER);
    
#endif

	return 0;
}

/*
 * Set S3C24xx Clock dividers
 */
static int imapx200_i2s_set_clkdiv(struct snd_soc_cpu_dai *cpu_dai,
	int div_id, int div)
{
	imapx200("Entered %s : div_id = %d, div = %d\n", __FUNCTION__, div_id, div);

	return 0;
}

static int imapx200_i2s_probe(struct platform_device *pdev)
{
	unsigned int div;
	imapx200("Entered %s\n", __FUNCTION__);

	imapx200_i2s.regs = ioremap(IIS_BASE_REG_PA, 0x200);
	if (imapx200_i2s.regs == NULL)
	{
		printk(KERN_ERR "regs is null, exit!\n");
		return -ENXIO;
	}
#ifndef FPGA_TEST
	imapx200_i2s.iis_clk = clk_get(&pdev->dev, "i2s");
	if (imapx200_i2s.iis_clk == NULL) {
		printk(KERN_ERR "failed to get iis_clock\n");
		return -ENODEV;
	}
	clk_enable(imapx200_i2s.iis_clk);
	imapx200_i2s.iis_clock_rate = clk_get_rate(imapx200_i2s.iis_clk);	

    div = __raw_readl(rDIV_CFG2);
    div &=~(0x1f<<24);
    div |= ((3<<2)|         //divide 4 to 120Mhz
            (2<<0))<<24;
    __raw_writel(div,rDIV_CFG2); 
	
//	printk("**********   iis_clock_rate is %x\n", imapx200_i2s.iis_clock_rate);
#endif
		return 0;
}

#ifdef CONFIG_PM
static int imapx200_i2s_suspend(struct platform_device *dev,
	struct snd_soc_cpu_dai *dai)
{
	imapx200("Entered %s\n", __FUNCTION__);

	rIER_pm = readl(imapx200_i2s.regs + rIER);;
	rIRER_pm = readl(imapx200_i2s.regs + rIRER);
	rITER_pm = readl(imapx200_i2s.regs + rITER);
	rCER_pm = readl(imapx200_i2s.regs + rCER);
	rCCR_pm = readl(imapx200_i2s.regs + rCCR);
	rCDR_pm = readl(imapx200_i2s.regs + rI2S_PRESCALER);
	rRER_pm = readl(imapx200_i2s.regs + rRER0);
	rTER_pm = readl(imapx200_i2s.regs + rTER0);
	rRCR_pm = readl(imapx200_i2s.regs + rRCR0);
	rTCR_pm = readl(imapx200_i2s.regs + rTCR0);
	rIMR_pm = readl(imapx200_i2s.regs + rIMR0);
	rRFCR_pm = readl(imapx200_i2s.regs + rRFCR0); 
	rTFCR_pm = readl(imapx200_i2s.regs + rTFCR0);
	return 0;
}

static int imapx200_i2s_resume(struct platform_device *pdev,
	struct snd_soc_cpu_dai *dai)
{
	imapx200("Entered %s\n", __FUNCTION__);
	writel(rIER_pm, imapx200_i2s.regs + rIER);;
	writel(rIRER_pm, imapx200_i2s.regs + rIRER);
	writel(rITER_pm, imapx200_i2s.regs + rITER);
	writel(rCER_pm, imapx200_i2s.regs + rCER);
	writel(rCCR_pm, imapx200_i2s.regs + rCCR);
	writel(rCDR_pm, imapx200_i2s.regs + rI2S_PRESCALER);
	writel(rRER_pm, imapx200_i2s.regs + rRER0);
	writel(rTER_pm, imapx200_i2s.regs + rTER0);
	writel(rRCR_pm, imapx200_i2s.regs + rRCR0);
	writel(rTCR_pm, imapx200_i2s.regs + rTCR0);
	writel(rIMR_pm, imapx200_i2s.regs + rIMR0);
	writel(rRFCR_pm, imapx200_i2s.regs + rRFCR0); 
	writel(rTFCR_pm, imapx200_i2s.regs + rTFCR0);
#ifndef FPGA_TEST
	imapx200_i2s.iis_clk = clk_get(&pdev->dev, "i2s");
	if (imapx200_i2s.iis_clk == NULL) {
		printk(KERN_ERR "failed to get iis_clock\n");
		return -ENODEV;
	}
	clk_enable(imapx200_i2s.iis_clk);
	imapx200_i2s.iis_clock_rate = clk_get_rate(imapx200_i2s.iis_clk);	
#endif

	return 0;
}

#else
#define imapx200_i2s_suspend	NULL
#define imapx200_i2s_resume	NULL
#endif
/* 
static int imapx200_i2s_prepare()
{

}
*/
#define imapx200_I2S_RATES \
	(SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 | SNDRV_PCM_RATE_16000 | \
	SNDRV_PCM_RATE_22050 | SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100 | \
	SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200 | SNDRV_PCM_RATE_96000)
static struct snd_soc_dai_ops imapx200_i2s_dai_ops = {
		.shutdown = imapx20064xx_i2s_shutdown,
		.trigger = imapx200_i2s_trigger,
		.hw_params = imapx200_i2s_hw_params,
		.set_fmt = imapx200_i2s_set_fmt,
		.set_clkdiv = imapx200_i2s_set_clkdiv,
		.set_sysclk = imapx200_i2s_set_sysclk,
//		.prepare = imapx200_i2s_prepare,
};


struct snd_soc_dai imapx200_i2s_dai[] = {
	{
	.name = "imapx200-i2s",
	.id = 0,
//	.type = SND_SOC_DAI_I2S,
	.probe = imapx200_i2s_probe,
	.suspend = imapx200_i2s_suspend,
	.resume = imapx200_i2s_resume,
	.playback = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = imapx200_I2S_RATES,
	//	.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE,},
	
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE},
	.capture = {
		.channels_min = 2,
		.channels_max = 2,
		.rates = imapx200_I2S_RATES,
	//	.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE | SNDRV_PCM_FORMAT_S24_LE,},
	
		.formats = SNDRV_PCM_FMTBIT_S8 | SNDRV_PCM_FMTBIT_S16_LE},
	.ops = &imapx200_i2s_dai_ops,
#if 0
	.ops = {
		.shutdown = imapx20064xx_i2s_shutdown,
		.trigger = imapx200_i2s_trigger,
		.hw_params = imapx200_i2s_hw_params,},
	.dai_ops = {
		.set_fmt = imapx200_i2s_set_fmt,
		.set_clkdiv = imapx200_i2s_set_clkdiv,
		.set_sysclk = imapx200_i2s_set_sysclk,
	},
#endif
	},
};
EXPORT_SYMBOL_GPL(imapx200_i2s_dai);


static int __init imapx200_i2s_init(void)
{
	return 0;
//		return snd_soc_register_dais(imapx200_i2s_dai,ARRAY_SIZE(imapx200_i2s_dai));
}

module_init(imapx200_i2s_init);
#if 0
static void __exit imapx200_i2s_exit(void)
{
	snd_soc_unregister_dais(imapx200_i2s_dai,
				ARRAY_SIZE(imapx200_i2s_dai));
}
module_exit(imapx200_i2s_exit);
#endif

/* Module information */
MODULE_AUTHOR("James xu, James Xu@infotmic.com.cn");
MODULE_DESCRIPTION("imapx200 I2S SoC Interface");
MODULE_LICENSE("GPL");
