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
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/wait.h>
#include <linux/delay.h>
#include <linux/clk.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/ac97_codec.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/pcm_params.h>
#include <asm/io.h>
#include <asm/dma.h>


#include "imapx200-ac97.h"
#include "../codecs/lm4550.h"
#include "imapx200-pcm.h"
#include <mach/imapx_ac97.h>
#include <mach/imapx_base_reg.h>
#include <plat/gpio-bank-d.h>
#include <mach/imapx_gpio.h>
#include <mach/imapx_mpdma.h>
#include <plat/dma.h>
#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define imapxdbg(x...) printk(x)
#else
#define imapxdbg(x...)
#endif

extern struct clk *clk_get(struct device *dev, const char *id);
extern int clk_enable(struct clk *clk);
extern void clk_disable(struct clk *clk);

struct imapx200_ac97_info {
	void __iomem	*regs;
	struct clk	*ac97_clk;
};
static struct imapx200_ac97_info imapx200_ac97;
//SAMPLETYPE imapx200_sampletype;

typedef enum{FIX=0,VSR} SAMPLETYPE;

static u32 codec_ready = AC_GLBSTAT_CODECREADY;
static DEFINE_MUTEX(ac97_mutex);
static DECLARE_WAIT_QUEUE_HEAD(gsr_wq);

static void SetControllerSampletype(SAMPLETYPE SampleType)
{
	u32 ac_glbctrl;
	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	if(SampleType == FIX)
		ac_glbctrl  &= ~(1<<AC_GLBCTRL_SAMPLERATE);
	else
		ac_glbctrl  |= (1<<AC_GLBCTRL_SAMPLERATE);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
}


static unsigned short imapx200_ac97_read(struct snd_ac97 *ac97,
	unsigned short reg)
{
	u32 ac_glbctrl;
	u32 ac_codec_cmd;
	u32 stat, addr, data;

	imapxdbg("Entered %s: reg=0x%x\n", __FUNCTION__, reg);

	mutex_lock(&ac97_mutex);

	ac_codec_cmd = (1<<AC_CODEC_CMD_READEN) | AC_CMD_ADDR(reg);
	writel(ac_codec_cmd, imapx200_ac97.regs + rAC_CODEC_CMD);

	udelay(1000);
	//enable code ready interrupt
	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	ac_glbctrl |= (1<<AC_GLBCTRL_CODECREADYEN);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	//read the return cmd address and the data 
	stat = readl(imapx200_ac97.regs + rAC_CODEC_STAT);
	addr = (stat >> 16) & 0x7f;
	data = (stat & 0xffff);

//	wait_event_timeout(gsr_wq,addr==reg,1);
	if(addr!=reg){
		printk(KERN_ERR"AC97: read error (ac97_reg=%x addr=%x)\n", reg, addr);
		printk(KERN_ERR"Check audio codec jumpper settings\n\n");
		goto out;
	}
	imapxdbg("the value of the addr %x is %x\n", addr, data);	
out:	mutex_unlock(&ac97_mutex);
	return (unsigned short)data;
}

static void imapx200_ac97_write(struct snd_ac97 *ac97, unsigned short reg,
	unsigned short val)
{
	u32 ac_glbctrl;
	u32 ac_codec_cmd;
	u32 stat, data;

//	printk(KERN_INFO "----------------imapx200_ac97_write\n");
	imapxdbg("Entered %s: reg=0x%x, val=0x%x\n", __FUNCTION__,reg,val);

	mutex_lock(&ac97_mutex);

//	printk(KERN_INFO "----------------imapx200_ac97_write1\n");
	ac_codec_cmd = (0<<AC_CODEC_CMD_READEN);
	ac_codec_cmd |= AC_CMD_ADDR(reg) | AC_CMD_DATA(val);
	writel(ac_codec_cmd, imapx200_ac97.regs + rAC_CODEC_CMD);

	udelay(1000);
	//enable codec ready interrupt


	ac_codec_cmd = (1<<AC_CODEC_CMD_READEN);
	ac_codec_cmd |= AC_CMD_ADDR(reg);
	writel(ac_codec_cmd, imapx200_ac97.regs + rAC_CODEC_CMD);
/*
	msleep(2);
	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	ac_glbctrl |= (1<<AC_GLBCTRL_CODECREADYEN);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	stat = readl(imapx200_ac97.regs + rAC_CODEC_STAT);
	data = (stat & 0xffff);

	if(data!=val){
		printk("%s: write error (ac97_val=%x data=%x)\n",
				__FUNCTION__, val, data);
	}
*/
	mutex_unlock(&ac97_mutex);
}

static void imapx200_ac97_warm_reset(struct snd_ac97 *ac97)
{
	u32 ac_glbctrl;

	imapxdbg("Entered %s\n", __FUNCTION__);

	//enable codec_ready interrupt	
	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	ac_glbctrl |= (1<<AC_GLBCTRL_CODECREADYEN);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	ac_glbctrl |= (1<<AC_GLBCTRL_WARMRESET);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);
/*
	ac_glbctrl &= ~rAC_GLBCTRL_WARMRESET;
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);
	
	ac_glbctrl = rAC_GLBCTRL_ACLINKON;
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	ac_glbctrl = rAC_GLBCTRL_TRANSFERDATAENABLE;
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	ac_glbctrl |= rAC_GLBCTRL_PCMOUTTM_DMA |
		rAC_GLBCTRL_PCMINTM_DMA | rAC_GLBCTRL_MICINTM_DMA;
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);

	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	ac_glbctrl |= rAC_GLBCTRL_ACLINKON;
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	udelay(1000);
	*/
}

static void imapx200_ac97_cold_reset(struct snd_ac97 *ac97)
{
	u32 ac_glbctrl;
	SAMPLETYPE imapx200_sampletype;
	imapx200_sampletype = VSR;
	imapxdbg("Entered %s\n", __FUNCTION__);

	//enable codec_ready interrupt	
	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	ac_glbctrl |= (1<<AC_GLBCTRL_CODECREADYEN);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	//here, try second times to reset the controller.
	ac_glbctrl = (1<<AC_GLBCTRL_COLDRESET);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	ac_glbctrl &= ~(1<<AC_GLBCTRL_COLDRESET);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	ac_glbctrl = (1<<AC_GLBCTRL_COLDRESET);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);

	ac_glbctrl &= ~(1<<AC_GLBCTRL_COLDRESET);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);
	
	ac_glbctrl |= (1<<AC_GLBCTRL_LINKON);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);	

	ac_glbctrl |= (1<<AC_GLBCTRL_TRANSFEN);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);
	//SetControllerSampletype(imapx200_sampletype);
#if 0
#ifdef FIXED_RATE
	ac_glbctrl &= ~(1<<AC_GLBCTRL_SAMPLERATE);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
#else
	//here, we select sample rate on demand
	ac_glbctrl |= (1<<AC_GLBCTRL_SAMPLERATE);
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
#endif	
	msleep(1);
#endif
}

static irqreturn_t imapx200_ac97_irq(int irq, void *dev_id)
{
	int status;
	u32 ac_glbctrl, ac_glbstat;

	ac_glbstat = readl(imapx200_ac97.regs + rAC_GLBSTAT);

//	imapxdbg("Entered %s: AC_GLBSTAT = 0x%x\n", __FUNCTION__, ac_glbstat);

	status = ac_glbstat & codec_ready;


	if (status) {
		ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
		ac_glbctrl &= ~(1<<AC_GLBCTRL_CODECREADYEN);
		writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	//	wake_up(&gsr_wq);
	}
	return IRQ_HANDLED;
}

struct snd_ac97_bus_ops soc_ac97_ops = {
	.read	= imapx200_ac97_read,
	.write	= imapx200_ac97_write,
	.warm_reset	= imapx200_ac97_warm_reset,
	.reset	= imapx200_ac97_cold_reset,
};
struct imapx200_dma_ch imapx200_pch[2] ={
	{
		
			.block_size = 4096,
			.src_addr = 0,
			.dst_addr = AC97_BASE_REG_PA + rAC_R_FRONT,
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
			.dst_addr = AC97_BASE_REG_PA + rAC_R_FRONT,
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

	.name = "AC97 PCM Stereo out",
	.type =	IMAPX200_DMA_LLI, 
	.block_num = 2,
	.pch = &imapx200_pch,
	.is_loop = 1,
};

static struct imapx200_pcm_dma_params imapx200_ac97_pcm_stereo_out = {
	.client		= &imapx200_dma_client_out,
	.channel	= 0,
	.dma_addr	= AC97_BASE_REG_PA + rAC_OFS,
	.dma_size	= 16,
};


static struct imapx200_dma_client imapx200_dma_client_in = {

	.name = "AC97 PCM Stereo Line in",
	.type =	IMAPX200_DMA_LLI, 
	.block_num = 2,
	.trans_type = 1,
	.pch = &imapx200_pch_in,
	.reload_type = 0,
};

static struct imapx200_pcm_dma_params imapx200_ac97_pcm_stereo_in = {
	.client		= &imapx200_dma_client_in,
	//.channel	= ,//todo,
	.channel	= 1,
	.dma_addr	= AC97_BASE_REG_PA + rAC_IFS,
	.dma_size	= 16,
};

static int imapx200_ac97_probe(struct platform_device *pdev)
{
	int ret;
	u32 rGPDCON_value, rGPDPUD_value;
	imapxdbg("Entered %s\n", __FUNCTION__);
	imapx200_ac97.regs = ioremap(AC97_BASE_REG_PA, 0x100);
	if (imapx200_ac97.regs == NULL)
		return -ENXIO;
/*******************************************************************/
//because the codec chip provide the clock, so the following sentenses should be delete.
/*
	imapx200_ac97.ac97_clk = clk_get(&pdev->dev, "ac97");
	if (imapx200_ac97.ac97_clk == NULL) {
		printk(KERN_ERR "imapx200-ac97 failed to get ac97_clock\n");
		iounmap(imapx200_ac97.regs);
		return -ENODEV;
	}
	clk_enable(imapx200_ac97.ac97_clk);
*/
/********************************************************************/
/*
*	s3c_gpio_cfgpin(S3C_GPD0,S3C_GPD0_AC97_BITCLK);
*       s3c_gpio_cfgpin(S3C_GPD1,S3C_GPD1_AC97_RESET);
*        s3c_gpio_cfgpin(S3C_GPD2,S3C_GPD2_AC97_SYNC);
*        s3c_gpio_cfgpin(S3C_GPD3,S3C_GPD3_AC97_SDI);
*        s3c_gpio_cfgpin(S3C_GPD4,S3C_GPD4_AC97_SDO);
*
*        s3c_gpio_pullup(S3C_GPD0,0);
*        s3c_gpio_pullup(S3C_GPD1,0);
*        s3c_gpio_pullup(S3C_GPD2,0);
*        s3c_gpio_pullup(S3C_GPD3,0);
*        s3c_gpio_pullup(S3C_GPD4,0);
*/
	//configure the AC97 GPIO
	rGPDCON_value = readl(rGPDCON);
	rGPDCON_value |= IMAP_GPD0_AC97_BITCLK;
	rGPDCON_value |= IMAP_GPD1_AC97_nRESET;
	rGPDCON_value |= IMAP_GPD2_AC97_SYNC;
	rGPDCON_value |= IMAP_GPD3_AC97_SDIN;
	rGPDCON_value |= IMAP_GPD4_AC97_SDOUT;
	writel(rGPDCON_value, rGPDCON);	

	rGPDPUD_value = 0;
	writel(rGPDPUD_value, rGPDPUD);

	ret = request_irq(IRQ_AC97, imapx200_ac97_irq,
		IRQF_DISABLED, "AC97", NULL);
	if (ret < 0) {
		printk(KERN_ERR "s3c24xx-ac97: interrupt request failed.\n");
	//	clk_disable(imapx200_ac97.ac97_clk);
	//	clk_put(imapx200_ac97.ac97_clk);
		iounmap(imapx200_ac97.regs);
	}

	return ret;
}

static void imapx200_ac97_remove(struct platform_device *pdev)
{
	imapxdbg("Entered %s\n", __FUNCTION__);

	free_irq(IRQ_AC97, NULL);
//	clk_disable(imapx200_ac97.ac97_clk);
//	clk_put(imapx200_ac97.ac97_clk);
	iounmap(imapx200_ac97.regs);
}

static int imapx200_ac97_hw_params(struct snd_pcm_substream *substream,
				struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	u32 ac_glbctrl;
	imapxdbg("Entered %s\n", __FUNCTION__);

	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK){
		cpu_dai->dma_data = &imapx200_ac97_pcm_stereo_out;
		ac_glbctrl |= (1<<AC_GLBCTRL_PCMOUTHFIFOEN);// half fifo
	}
	else {
		cpu_dai->dma_data = &imapx200_ac97_pcm_stereo_in;
		ac_glbctrl |= (1<<AC_GLBCTRL_PCMINHFIFOEN); // half fifo
	}
	switch (params_format(params)) {
	case SNDRV_PCM_FORMAT_S16_LE:
		ac_glbctrl &= ~(3<<AC_GLBCTRL_SAMPLESIZE);
		break;
	case SNDRV_PCM_FORMAT_S18_3LE:
		ac_glbctrl &= ~(3<<AC_GLBCTRL_SAMPLESIZE);
		ac_glbctrl |= (1<<AC_GLBCTRL_SAMPLESIZE);
		break;
	case SNDRV_PCM_FORMAT_S20_3LE:
		ac_glbctrl &= ~(3<<AC_GLBCTRL_SAMPLESIZE);
		ac_glbctrl |= (2<<AC_GLBCTRL_SAMPLESIZE);
		break;
	default:
		return -EINVAL;
	}

	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	
	return 0;
}

static int imapx200_ac97_hifi_prepare(struct snd_pcm_substream *substream)
{

	u32 ac_glbctrl;
	imapxdbg("Entered %s: \n", __FUNCTION__);
	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
#ifdef FIXED_RATE
	ac_glbctrl &= ~(1<<AC_GLBCTRL_SAMPLERATE);
//	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
#else
	//here, we select sample rate on demand
	ac_glbctrl |= (1<<AC_GLBCTRL_SAMPLERATE);
//	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
#endif
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);
/*	
	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			ac_glbctrl |= (2<<AC_GLBCTRL_PCMINMODE); //dma mode
			ac_glbctrl |= (1<<AC_GLBCTRL_PCMINHFIFOEN); // half fifo
		else
			ac_glbctrl |= (2<<AC_GLBCTRL_PCMOUTMODE); // dma mode
			ac_glbctrl |= (1<<AC_GLBCTRL_PCMOUTHFIFOEN);// half fifo
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			ac_glbctrl &= ~(3<<AC_GLBCTRL_PCMINMODE);
		else
			ac_glbctrl &= ~(3<<AC_GLBCTRL_PCMOUTMODE);
		break;
	}
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);
	msleep(1);
*/
	/*********************************************************/
	//these will be refine later
	/*
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		s3c6400_ac97_write(0,0x02, 0x8080);	
		s3c6400_ac97_write(0, 0x04, 0x0606);	
		s3c6400_ac97_write(0,0x1c, 0x00aa);
	}
	else
	{
		s3c6400_ac97_write(0, 0x12, 0x0f0f);

		s3c6400_ac97_write(0, 0x14, 0xd612);
	}
	*/
	/************************************************************/
	return 0;
}


static int imapx200_ac97_trigger(struct snd_pcm_substream *substream, int cmd)
{
	u32 ac_glbctrl;

	imapxdbg("Entered %s: cmd = %d\n", __FUNCTION__, cmd);

	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE){
			ac_glbctrl |= (2<<AC_GLBCTRL_PCMINMODE); //dma mode, enable this bit means to start operation 
//			ac_glbctrl |= (1<<AC_GLBCTRL_PCMINHFIFOEN);  half fifo
		}
		else {
			ac_glbctrl |= (2<<AC_GLBCTRL_PCMOUTMODE); // dma mode
//			ac_glbctrl |= (1<<AC_GLBCTRL_PCMOUTHFIFOEN); half fifo
		}
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
			ac_glbctrl &= ~(3<<AC_GLBCTRL_PCMINMODE);
		else
			ac_glbctrl &= ~(3<<AC_GLBCTRL_PCMOUTMODE);
		break;
	}
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);

	return 0;
}

#if 0
static int s3c6400_ac97_hw_mic_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_cpu_dai *cpu_dai = rtd->dai->cpu_dai;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK)
		return -ENODEV;
	else
		cpu_dai->dma_data = &s3c6400_ac97_mic_mono_in;

	return 0;
}

static int s3c6400_ac97_mic_trigger(struct snd_pcm_substream *substream,
	int cmd)
{
	u32 ac_glbctrl;

	imapxdbg("Entered %s\n", __FUNCTION__);

	ac_glbctrl = readl(imapx200_ac97.regs + rAC_GLBCTRL);
	switch(cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		ac_glbctrl |= rAC_GLBCTRL_PCMINTM_DMA;
		break;
	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		ac_glbctrl &= ~rAC_GLBCTRL_PCMINTM_MASK;
	}
	writel(ac_glbctrl, imapx200_ac97.regs + rAC_GLBCTRL);

	return 0;
}
#endif

#define imapx200_AC97_RATES (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_11025 |\
		SNDRV_PCM_RATE_16000 | SNDRV_PCM_RATE_22050 | \
		SNDRV_PCM_RATE_44100 | SNDRV_PCM_RATE_48000)

static struct snd_soc_dai_ops imapx200_ac97_dai_ops = {
	.hw_params = imapx200_ac97_hw_params,
	.prepare = imapx200_ac97_hifi_prepare,
	.trigger = imapx200_ac97_trigger,
};
struct snd_soc_dai imapx200_ac97_dai[] = {
{
	.name = "imapx200-ac97",
	.id = 0,
	.ac97_control = 1,
//	.type = SND_SOC_DAI_AC97,
	.probe = imapx200_ac97_probe,
	.remove = imapx200_ac97_remove,
	.playback = {
		.stream_name = "AC97 Playback",
		.channels_min = 2,
		.channels_max = 2,
		.rates = imapx200_AC97_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.capture = {
		.stream_name = "AC97 Capture",
		.channels_min = 2,
		.channels_max = 2,
		.rates = imapx200_AC97_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops =	&imapx200_ac97_dai_ops, 
},
#if 0
{
	.name = "s3c6400-ac97-mic",
	.id = 1,
	.type = SND_SOC_DAI_AC97,
	.capture = {
		.stream_name = "AC97 Mic Capture",
		.channels_min = 1,
		.channels_max = 1,
		.rates = s3c6400_AC97_RATES,
		.formats = SNDRV_PCM_FMTBIT_S16_LE,},
	.ops = {
		.hw_params = s3c6400_ac97_hw_mic_params,
		.trigger = s3c6400_ac97_mic_trigger,},
},
#endif
};

EXPORT_SYMBOL_GPL(imapx200_ac97_dai);
EXPORT_SYMBOL_GPL(soc_ac97_ops);

static int __init imapx200_ac97_init(void)
{
	return snd_soc_register_dais(imapx200_ac97_dai,
				     ARRAY_SIZE(imapx200_ac97_dai));
}
module_init(imapx200_ac97_init);

static void __exit imapx200_ac97_exit(void)
{
	snd_soc_unregister_dais(imapx200_ac97_dai,
				ARRAY_SIZE(imapx200_ac97_dai));
}
module_exit(imapx200_ac97_exit);


MODULE_AUTHOR("James xu, James Xu@infotmic.com.cn");
MODULE_DESCRIPTION("AC97 driver for the imapx200 ");
MODULE_LICENSE("GPL");
