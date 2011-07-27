/********************************************************************************
** sound/soc/imapx200/imapx200-pcm.c 
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
**     1.0  12/21/2009    James Xu
********************************************************************************/
#include <linux/module.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/platform_device.h>
#include <linux/slab.h>
#include <linux/dma-mapping.h>
#include <linux/bitmap.h>
#include <linux/dw_dmac.h>
#include <sound/atmel-ac97c.h>
#include <linux/dmaengine.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <asm/bitops.h>
#include <asm/dma.h>
#include <mach/hardware.h>
#include <plat/dma.h>
//#define imapx200_pcm_debug
#ifdef imapx200_pcm_debug
#define pr_debug(fmt...) printk(fmt)
#else
#define pr_debug(fmt...) 
#endif
#include "imapx200-pcm.h"
enum {
	DMA_TX_READY = 0,
	DMA_RX_READY,
	DMA_TX_CHAN_PRESENT,
	DMA_RX_CHAN_PRESENT,
};


struct imapx200_dma_channel {
	struct dma_chan			*rx_chan;
	struct dma_chan			*tx_chan;
};

struct imapx200_dma {
	struct clk			*pclk;
	struct platform_device		*pdev;
	struct imapx200_dma_channel		dma;

	struct snd_pcm_substream	*substream;
	struct snd_card			*card;
	struct snd_pcm			*pcm;

	u64				cur_format;
	unsigned int			cur_rate;
	unsigned long			flags;
	/* Serialize access to opened variable */
	spinlock_t			lock;
	void __iomem			*regs;
	int				irq;
	int				opened;
	int				reset_pin;
};


static const struct snd_pcm_hardware imapx200_pcm_hardware = {
	.info			= SNDRV_PCM_INFO_INTERLEAVED |
				    SNDRV_PCM_INFO_BLOCK_TRANSFER |
				    SNDRV_PCM_INFO_MMAP |
				    SNDRV_PCM_INFO_MMAP_VALID |
				    SNDRV_PCM_INFO_PAUSE |
				    SNDRV_PCM_INFO_RESUME,
	.formats		= SNDRV_PCM_FMTBIT_S16_LE |
				    SNDRV_PCM_FMTBIT_U16_LE |
				    SNDRV_PCM_FMTBIT_U8 |
				    SNDRV_PCM_FMTBIT_S8,
	.channels_min		= 2,
	.channels_max		= 2,
	.buffer_bytes_max	= 128*1024,
	.period_bytes_min	= PAGE_SIZE,
	.period_bytes_max	= PAGE_SIZE*4,
	.periods_min		= 2,
	.periods_max		= 128,
	.fifo_size		= 64,
};

struct imapx200_runtime_data {
	spinlock_t lock;
	int state;
	unsigned int dma_loaded;
	unsigned int dma_limit;
	unsigned int dma_period;
	dma_addr_t dma_start;
	dma_addr_t dma_pos;
	dma_addr_t dma_end;
	struct imapx200_pcm_dma_params *params;
};

/* This function is called by the DMA driver. */
static void imapx200_dma_period_done(void *arg)
{
	struct imapx200_dma *chip = arg;
	snd_pcm_period_elapsed(chip->substream);
}


static int imapx200_prepare_dma(struct imapx200_dma *chip,
		struct snd_pcm_substream *substream,
		enum dma_data_direction direction)
{
	struct dma_chan			*chan;
	struct dw_cyclic_desc		*cdesc;
	struct snd_pcm_runtime		*runtime = substream->runtime;
	unsigned long			buffer_len, period_len;

	/*
	 * We don't do DMA on "complex" transfers, i.e. with
	 * non-halfword-aligned buffers or lengths.
	 */
	if (runtime->dma_addr & 1 || runtime->buffer_size & 1) {
		dev_dbg(&chip->pdev->dev, "too complex transfer\n");
		return -EINVAL;
	}

	if (direction == DMA_TO_DEVICE)
		chan = chip->dma.tx_chan;
	else
		chan = chip->dma.rx_chan;

	buffer_len = frames_to_bytes(runtime, runtime->buffer_size);
	period_len = frames_to_bytes(runtime, runtime->period_size);

	cdesc = dw_dma_cyclic_prep(chan, runtime->dma_addr, buffer_len,
			period_len, direction);
	if (IS_ERR(cdesc)) {
		dev_dbg(&chip->pdev->dev, "could not prepare cyclic DMA\n");
		return PTR_ERR(cdesc);
	}

	if (direction == DMA_TO_DEVICE) {
		cdesc->period_callback = imapx200_dma_period_done;
		set_bit(DMA_TX_READY, &chip->flags);
	} else {
		cdesc->period_callback = imapx200_dma_period_done;
		set_bit(DMA_RX_READY, &chip->flags);
	}

	cdesc->period_callback_param = chip;

	return 0;
}


/* This function is called by the DMA driver. */
/*
static void imapx200_dma_period_done(void *arg)
{
	struct imapx200_dma *chip = arg;
	snd_pcm_period_elapsed(chip->substream);
}
*/
/* imapx200_pcm_enqueue
 *
 * place a dma buffer onto the queue for the dma system
 * to handle.
*/
static void imapx200_pcm_enqueue(struct snd_pcm_substream *substream)
{
	struct imapx200_runtime_data *prtd = substream->runtime->private_data;
	dma_addr_t pos = prtd->dma_pos;
	int ret;
	pr_debug("Entered %s\n", __func__);
	if (substream == NULL)
		pr_debug("%s: error parameter subsream\n", __func__);
	while (prtd->dma_loaded < prtd->dma_limit) {
		unsigned long len = prtd->dma_period;
		pr_debug("dma_loaded: %d\n", prtd->dma_loaded);
		if ((pos + len) > prtd->dma_end) {
			len  = prtd->dma_end - pos;
			pr_debug(KERN_DEBUG "%s: corrected dma len %ld\n",
			       __func__, len);
		}
		ret = imapx200_dma_enqueue(prtd->params->channel,
			substream, pos, len);
		if (ret == 0) {
			prtd->dma_loaded++;
			pos += prtd->dma_period;
			if (pos >= prtd->dma_end)
				pos = prtd->dma_start;
		} else
			break;
	}
	prtd->dma_pos = pos;
}

static void imapx200_audio_buffdone(struct imapx200_dma_chan *channel,
				void *dev_id, int size,
				enum imapx200_dma_buffresult result)
{
	struct snd_pcm_substream *substream = dev_id;
	struct imapx200_runtime_data *prtd;

	pr_debug("Entered %s\n", __func__);

	if (result == IMAPX200_RES_ABORT || result == IMAPX200_RES_ERR)
		return;

	prtd = substream->runtime->private_data;

	if (substream)
		snd_pcm_period_elapsed(substream);

	spin_lock(&prtd->lock);
	if (prtd->state & ST_RUNNING) {
		prtd->dma_loaded--;
		imapx200_pcm_enqueue(substream);
	}

	spin_unlock(&prtd->lock);
}

static int imapx200_pcm_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx200_runtime_data *prtd = runtime->private_data;
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct imapx200_pcm_dma_params *dma = rtd->dai->cpu_dai->dma_data;
	unsigned long totbytes = params_buffer_bytes(params);
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	/* return if this is a bufferless transfer e.g.
	 * codec <--> BT codec or GSM modem -- lg FIXME */
	if (!dma)
		return 0;

	/* this may get called several times by oss emulation
	 * with different params -HW */
	if (prtd->params == NULL) {
		/* prepare DMA */
		prtd->params = dma;

		pr_debug("params %p, client %p, channel %d\n", prtd->params,
			prtd->params->client, prtd->params->channel);

		ret = imapx200_dma_request(prtd->params->channel,
					  prtd->params->client, NULL);

		if (ret < 0) {
			printk(KERN_ERR "failed to get dma channel\n");
			return ret;
		}
	}

	imapx200_dma_set_buffdone_fn(prtd->params->channel,
				    imapx200_audio_buffdone);

	snd_pcm_set_runtime_buffer(substream, &substream->dma_buffer);

	runtime->dma_bytes = totbytes;

	spin_lock_irq(&prtd->lock);
	prtd->dma_loaded = 0;
	prtd->dma_limit = runtime->hw.periods_min;
	prtd->dma_period = params_period_bytes(params);
	prtd->dma_start = runtime->dma_addr;
	prtd->dma_pos = prtd->dma_start;
	prtd->dma_end = prtd->dma_start + totbytes;
	spin_unlock_irq(&prtd->lock);

	return 0;
}

static int imapx200_dma_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *hw_params)
{
	struct imapx200_dma *chip = snd_pcm_substream_chip(substream);
	int retval;

	retval = snd_pcm_lib_malloc_pages(substream,
					params_buffer_bytes(hw_params));
	if (retval < 0)
		return retval;
	/* snd_pcm_lib_malloc_pages returns 1 if buffer is changed. */
	if (retval == 1) {
		if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
			if (test_and_clear_bit(DMA_TX_READY, &chip->flags))
				dw_dma_cyclic_free(chip->dma.tx_chan);
		}
		else {
			if (test_and_clear_bit(DMA_RX_READY, &chip->flags))
				dw_dma_cyclic_free(chip->dma.rx_chan);
		}
			
	}
	return retval;
}
static int imapx200_dma_hw_free(struct snd_pcm_substream *substream)
{
	struct imapx200_dma *chip = snd_pcm_substream_chip(substream);
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (test_and_clear_bit(DMA_TX_READY, &chip->flags))
			dw_dma_cyclic_free(chip->dma.tx_chan);
	}
	else {
		if (test_and_clear_bit(DMA_RX_READY, &chip->flags))
			dw_dma_cyclic_free(chip->dma.rx_chan);
	}
	return snd_pcm_lib_free_pages(substream);
}

static int imapx200_dma_prepare(struct snd_pcm_substream *substream)
{
	struct imapx200_dma *chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime *runtime = substream->runtime;
	int retval;


	/* assign channels to AC97C channel A */
	switch (runtime->channels) {
	case 1:
		break;
	case 2:
		break;
	default:
		/* TODO: support more than two channels */
		return -EINVAL;
	}

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		if (!test_bit(DMA_TX_READY, &chip->flags))
			retval = imapx200_prepare_dma(chip, substream,
					DMA_TO_DEVICE);
	}
	else {
		if (!test_bit(DMA_RX_READY, &chip->flags))
			retval = imapx200_prepare_dma(chip, substream,
					DMA_FROM_DEVICE);


	}

	return retval;
}

static int
imapx200_dma_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct imapx200_dma *chip = snd_pcm_substream_chip(substream);
	unsigned long camr;
	int retval = 0;


	switch (cmd) {
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE: /* fall through */
	case SNDRV_PCM_TRIGGER_RESUME: /* fall through */
	case SNDRV_PCM_TRIGGER_START:
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {

		retval = dw_dma_cyclic_start(chip->dma.tx_chan);
		if (retval)
			goto out;
	}
	else {
		retval = dw_dma_cyclic_start(chip->dma.rx_chan);
		if (retval)
			goto out;
	}
		break;
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH: /* fall through */
	case SNDRV_PCM_TRIGGER_SUSPEND: /* fall through */
	case SNDRV_PCM_TRIGGER_STOP:
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		dw_dma_cyclic_stop(chip->dma.tx_chan);
	}
	else {

		dw_dma_cyclic_stop(chip->dma.rx_chan);
	}
		break;
	default:
		retval = -EINVAL;
		goto out;
	}

out:
	return retval;
}

static snd_pcm_uframes_t
imapx200_dma_pointer(struct snd_pcm_substream *substream)
{
	struct imapx200_dma	*chip = snd_pcm_substream_chip(substream);
	struct snd_pcm_runtime	*runtime = substream->runtime;
	snd_pcm_uframes_t	frames;
	unsigned long		bytes;

	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		bytes = dw_dma_get_src_addr(chip->dma.tx_chan);
	}
	else {

		bytes = dw_dma_get_dst_addr(chip->dma.rx_chan);
	}
	bytes -= runtime->dma_addr;

	frames = bytes_to_frames(runtime, bytes);
	if (frames >= runtime->buffer_size)
		frames -= runtime->buffer_size;
	return frames;
}



static int imapx200_pcm_hw_free(struct snd_pcm_substream *substream)
{
	struct imapx200_runtime_data *prtd = substream->runtime->private_data;

	pr_debug("Entered %s\n", __func__);

	/* TODO - do we need to ensure DMA flushed */
	snd_pcm_set_runtime_buffer(substream, NULL);

	if (prtd->params) {
		imapx200_dma_free(prtd->params->channel, prtd->params->client);
		prtd->params = NULL;
	}

	return 0;
}

static int imapx200_pcm_prepare(struct snd_pcm_substream *substream)
{
	struct imapx200_runtime_data *prtd = substream->runtime->private_data;
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	/* return if this is a bufferless transfer e.g.
	 * codec <--> BT codec or GSM modem -- lg FIXME */
	if (!prtd->params)
		return 0;

	/* channel needs configuring for mem=>device, increment memory addr,
	 * sync to pclk, half-word transfers to the IIS-FIFO. */
	if (substream->stream == SNDRV_PCM_STREAM_PLAYBACK) {
		imapx200_dma_devconfig(prtd->params->channel,
				      IMAPX200_DMASRC_MEM,
				      prtd->params->dma_addr);
	} else {
		imapx200_dma_devconfig(prtd->params->channel,
				      IMAPX200_DMASRC_HW,
				      prtd->params->dma_addr);
	}

	imapx200_dma_config(prtd->params->channel,
			   prtd->params->dma_size);

	/* flush the DMA channel */
	imapx200_dma_ctrl(prtd->params->channel, IMAPX200_DMAOP_FLUSH);
	prtd->dma_loaded = 0;
	prtd->dma_pos = prtd->dma_start;

	/* enqueue dma buffers */
	imapx200_pcm_enqueue(substream);

	return ret;
}

static int imapx200_pcm_trigger(struct snd_pcm_substream *substream, int cmd)
{
	struct imapx200_runtime_data *prtd = substream->runtime->private_data;
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	spin_lock(&prtd->lock);

	switch (cmd) {
	case SNDRV_PCM_TRIGGER_START:
	case SNDRV_PCM_TRIGGER_RESUME:
	case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
		prtd->state |= ST_RUNNING;
		imapx200_dma_ctrl(prtd->params->channel, IMAPX200_DMAOP_START);
		imapx200_dma_ctrl(prtd->params->channel, IMAPX200_DMAOP_STARTED);
		break;

	case SNDRV_PCM_TRIGGER_STOP:
	case SNDRV_PCM_TRIGGER_SUSPEND:
	case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
		prtd->state &= ~ST_RUNNING;
		imapx200_dma_ctrl(prtd->params->channel, IMAPX200_DMAOP_STOP);
		break;

	default:
		ret = -EINVAL;
		break;
	}

	spin_unlock(&prtd->lock);

	return ret;
}

static snd_pcm_uframes_t
imapx200_pcm_pointer(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx200_runtime_data *prtd = runtime->private_data;
	unsigned long res;
	dma_addr_t src, dst;

	pr_debug("Entered %s\n", __func__);

	spin_lock(&prtd->lock);
	imapx200_dma_getposition(prtd->params->channel, &src, &dst);

	if (substream->stream == SNDRV_PCM_STREAM_CAPTURE)
		res = dst - prtd->dma_start;
	else
		res = src - prtd->dma_start;

	spin_unlock(&prtd->lock);

	pr_debug("Pointer %x %x\n", src, dst);

	/* we seem to be getting the odd error from the pcm library due
	 * to out-of-bounds pointers. this is maybe due to the dma engine
	 * not having loaded the new values for the channel before being
	 * callled... (todo - fix )
	 */

	if (res >= snd_pcm_lib_buffer_bytes(substream)) {
		if (res == snd_pcm_lib_buffer_bytes(substream))
			res = 0;
	}

	return bytes_to_frames(substream->runtime, res);
}

static int imapx200_pcm_open(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx200_runtime_data *prtd;

	pr_debug("Entered %s\n", __func__);

	snd_soc_set_runtime_hwparams(substream, &imapx200_pcm_hardware);

	prtd = kzalloc(sizeof(struct imapx200_runtime_data), GFP_KERNEL);
	if (prtd == NULL)
		return -ENOMEM;

	spin_lock_init(&prtd->lock);

	runtime->private_data = prtd;
	return 0;
}

static int imapx200_pcm_close(struct snd_pcm_substream *substream)
{
	struct snd_pcm_runtime *runtime = substream->runtime;
	struct imapx200_runtime_data *prtd = runtime->private_data;

	pr_debug("Entered %s\n", __func__);

	if (!prtd)
		pr_debug("imapx200_pcm_close called with prtd == NULL\n");

	kfree(prtd);

	return 0;
}

static int imapx200_pcm_mmap(struct snd_pcm_substream *substream,
	struct vm_area_struct *vma)
{
	struct snd_pcm_runtime *runtime = substream->runtime;

	pr_debug("Entered %s\n", __func__);

	return dma_mmap_writecombine(substream->pcm->card->dev, vma,
				     runtime->dma_area,
				     runtime->dma_addr,
				     runtime->dma_bytes);
}

static struct snd_pcm_ops imapx200_pcm_ops = {
	.open		= imapx200_pcm_open,
	.close		= imapx200_pcm_close,
	.ioctl		= snd_pcm_lib_ioctl,
	.hw_params	= imapx200_dma_hw_params,
	.hw_free	= imapx200_dma_hw_free,
	.prepare	= imapx200_dma_prepare,
	.trigger	= imapx200_dma_trigger,
	.pointer	= imapx200_dma_pointer,
	.mmap		= imapx200_pcm_mmap,
};

static int imapx200_pcm_preallocate_dma_buffer(struct snd_pcm *pcm, int stream)
{
	struct snd_pcm_substream *substream = pcm->streams[stream].substream;
	struct snd_dma_buffer *buf = &substream->dma_buffer;
	size_t size = imapx200_pcm_hardware.buffer_bytes_max;

	pr_debug("Entered %s\n", __func__);

	buf->dev.type = SNDRV_DMA_TYPE_DEV;
	buf->dev.dev = pcm->card->dev;
	buf->private_data = NULL;
	buf->area = dma_alloc_writecombine(pcm->card->dev, size,
					   &buf->addr, GFP_KERNEL);
	if (!buf->area)
		return -ENOMEM;
	buf->bytes = size;
	return 0;
}

static void imapx200_pcm_free_dma_buffers(struct snd_pcm *pcm)
{
	struct snd_pcm_substream *substream;
	struct snd_dma_buffer *buf;
	int stream;

	pr_debug("Entered %s\n", __func__);

	for (stream = 0; stream < 2; stream++) {
		substream = pcm->streams[stream].substream;
		if (!substream)
			continue;

		buf = &substream->dma_buffer;
		if (!buf->area)
			continue;

		dma_free_writecombine(pcm->card->dev, buf->bytes,
				      buf->area, buf->addr);
		buf->area = NULL;
	}
}

static u64 imapx200_pcm_dmamask = DMA_BIT_MASK(32);

static int imapx200_pcm_new(struct snd_card *card,
	struct snd_soc_dai *dai, struct snd_pcm *pcm)
{
	int ret = 0;

	pr_debug("Entered %s\n", __func__);

	if (!card->dev->dma_mask)
		card->dev->dma_mask = &imapx200_pcm_dmamask;
	if (!card->dev->coherent_dma_mask)
		card->dev->coherent_dma_mask = 0xffffffff;

	if (dai->playback.channels_min) {
		ret = imapx200_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_PLAYBACK);
		if (ret)
			goto out;
	}

	if (dai->capture.channels_min) {
		ret = imapx200_pcm_preallocate_dma_buffer(pcm,
			SNDRV_PCM_STREAM_CAPTURE);
		if (ret)
			goto out;
	}
 out:
	return ret;
}

struct snd_soc_platform imapx200_soc_platform = {
	.name		= "imapx200-audio",
	.pcm_ops 	= &imapx200_pcm_ops,
	.pcm_new	= imapx200_pcm_new,
	.pcm_free	= imapx200_pcm_free_dma_buffers,
};
EXPORT_SYMBOL_GPL(imapx200_soc_platform);
static int __devexit imapx200_ac97c_remove(struct platform_device *pdev)
{
	struct snd_card *card = platform_get_drvdata(pdev);
	struct imapx200_dma *chip = get_chip(card);

	if (test_bit(DMA_RX_CHAN_PRESENT, &chip->flags))
		dma_release_channel(chip->dma.rx_chan);
	if (test_bit(DMA_TX_CHAN_PRESENT, &chip->flags))
		dma_release_channel(chip->dma.tx_chan);
	clear_bit(DMA_RX_CHAN_PRESENT, &chip->flags);
	clear_bit(DMA_TX_CHAN_PRESENT, &chip->flags);
	chip->dma.rx_chan = NULL;
	chip->dma.tx_chan = NULL;

	platform_set_drvdata(pdev, NULL);

	return 0;
}


static int __devinit imapx200_ac97c_probe(struct platform_device *pdev)
{
	struct snd_card			*card;
	struct imapx200_dma		*chip;
	struct resource			*regs;
	struct ac97c_platform_data	*pdata;


	pdata = pdev->dev.platform_data;
	if (!pdata) {
		dev_dbg(&pdev->dev, "no platform data\n");
		return -ENXIO;
	}
	chip = get_chip(card);
	if (pdata->rx_dws.dma_dev) {
		struct dw_dma_slave *dws = &pdata->rx_dws;
		dma_cap_mask_t mask;


		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);

		chip->dma.rx_chan = dma_request_channel(mask, filter, dws);

		dev_info(&chip->pdev->dev, "using %s for DMA RX\n",
				dev_name(&chip->dma.rx_chan->dev->device));
		set_bit(DMA_RX_CHAN_PRESENT, &chip->flags);
	}

	if (pdata->tx_dws.dma_dev) {
		struct dw_dma_slave *dws = &pdata->tx_dws;
		dma_cap_mask_t mask;


		dma_cap_zero(mask);
		dma_cap_set(DMA_SLAVE, mask);

		chip->dma.tx_chan = dma_request_channel(mask, filter, dws);

		dev_info(&chip->pdev->dev, "using %s for DMA TX\n",
				dev_name(&chip->dma.tx_chan->dev->device));
		set_bit(DMA_TX_CHAN_PRESENT, &chip->flags);
	}

	if (!test_bit(DMA_RX_CHAN_PRESENT, &chip->flags) &&
			!test_bit(DMA_TX_CHAN_PRESENT, &chip->flags)) {
		dev_dbg(&pdev->dev, "DMA not available\n");
		retval = -ENODEV;
		goto err_dma;
	}
	return 0;

err_dma:
	if (test_bit(DMA_RX_CHAN_PRESENT, &chip->flags))
		dma_release_channel(chip->dma.rx_chan);
	if (test_bit(DMA_TX_CHAN_PRESENT, &chip->flags))
		dma_release_channel(chip->dma.tx_chan);
	clear_bit(DMA_RX_CHAN_PRESENT, &chip->flags);
	clear_bit(DMA_TX_CHAN_PRESENT, &chip->flags);
	chip->dma.rx_chan = NULL;
	chip->dma.tx_chan = NULL;
	return -1;
}


static struct platform_driver impax200_ac97c_driver = {
	.remove		= __devexit_p(imapx200_ac97c_remove),
	.driver		= {
		.name	= "imapx200_ac97c",
	},
};


static int __init imapx200_soc_platform_init(void)
{
	platform_driver_probe(&imapx200_ac97c_driver,
			imapx200_ac97c_probe);
	return snd_soc_register_platform(&imapx200_soc_platform);
}
module_init(imapx200_soc_platform_init);

static void __exit imapx200_soc_platform_exit(void)
{
	platform_driver_unregister(&imapx200_ac97c_driver);
	snd_soc_unregister_platform(&imapx200_soc_platform);
}
module_exit(imapx200_soc_platform_exit);

MODULE_AUTHOR("James xu, <james_xu@infotmic.com.cn>");
MODULE_DESCRIPTION("INFOTM IMAPX200 PCM DMA module");
MODULE_LICENSE("GPL");
