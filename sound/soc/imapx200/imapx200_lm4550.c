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
**     1.0  12/15/2009    James Xu
********************************************************************************/


#include <linux/module.h>
#include <linux/device.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include "../codecs/lm4550.h"
#include "imapx200-pcm.h"
#include "imapx200-ac97.h"
#define imapx200_lm4550_debug
#ifdef imapx200_lm4550_debug 
#define imapx200_debug(fmt...) printk(fmt)
#else
#define imapx200_debug(fmt...) 
#endif
static struct snd_soc_card imapx200;

static int imapx200_probe(struct platform_device *pdev)
{
	imapx200_debug("imapx200_lm4550_probe!\n");
	return 0;
}
static int imapx200_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	imapx200_debug("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));

	return 0;
}

static int imapx200_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;

	imapx200_debug("Entered %s\n", __FUNCTION__);
	return 0;
}

static struct snd_soc_ops imapx200_hifi_ops = {
	.hw_params = imapx200_hifi_hw_params,
	.hw_free = imapx200_hifi_hw_free,
};


static int imapx200_lm4550_init(struct snd_soc_codec *codec)
{
	return 0;
}
static struct snd_soc_dai_link imapx200_dai[] = {
{
	.name = "AC97",
	.stream_name = "AC97 HiFi",
	.cpu_dai = &imapx200_ac97_dai[0],
	/**********************************/
	.codec_dai = &lm4550_dai,
	.init = imapx200_lm4550_init,
	.ops = &imapx200_hifi_ops,

	/*********************************/
},
};

static struct snd_soc_card imapx200 = {
	.name = "IMAPX200",
	.probe = imapx200_probe,
	.platform = &imapx200_soc_platform,
	.dai_link = imapx200_dai,
	.num_links = ARRAY_SIZE(imapx200_dai),
};

static struct snd_soc_device imapx200_snd_ac97_devdata = {
	.card = &imapx200, 
	/*****************************************/
	.codec_dev = &soc_codec_dev_lm4550,
	/*****************************************/
};

static struct platform_device *imapx200_snd_ac97_device;

static int __init imapx200_init(void)
{
	int ret;

	imapx200_snd_ac97_device = platform_device_alloc("soc-audio", -1);
	if (!imapx200_snd_ac97_device)
		return -ENOMEM;

	platform_set_drvdata(imapx200_snd_ac97_device,
				&imapx200_snd_ac97_devdata);
	imapx200_snd_ac97_devdata.dev = &imapx200_snd_ac97_device->dev;
	ret = platform_device_add(imapx200_snd_ac97_device);

	if (ret)
		platform_device_put(imapx200_snd_ac97_device);

	return ret;
}

static void __exit imapx200_exit(void)
{
	platform_device_unregister(imapx200_snd_ac97_device);
}

module_init(imapx200_init);
module_exit(imapx200_exit);

/* Module information */
MODULE_AUTHOR("James xu, james_xu@infotm.com.cn");
MODULE_DESCRIPTION("ALSA SoC LM4550 IMAPX200");
MODULE_LICENSE("GPL");
