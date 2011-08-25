/*
 * linux/sound/soc/tcc/tcc_board_es8388.c
 *
 * Author:  <linux@telechips.com>
 * Created: Nov 30, 2007
 * Description: SoC audio for TCCxx
 *
 * Copyright (C) 2008-2009 Telechips 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see the file COPYING, or write
 * to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/init.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/gpio.h>
#include <asm/mach-types.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h>
#include <asm/hardware/scoop.h>
#include <asm/io.h>

#include <mach/hardware.h>
#include <mach/gpio.h>
#include <mach/imapx_iis.h>
#include <mach/imapx_base_reg.h>
#include <mach/imapx_gpio.h>
#include <plat/gpio-bank-c.h>
#include <plat/gpio-bank-d.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>
#include <sound/pcm_params.h>

#include <mach/imapx_sysmgr.h>
#include "../codecs/es8388.h"
#include "imapx200-pcm.h"
#include "imapx200-i2s.h"

#define TCC_HP        0
#define TCC_SPK       1

#define TCC_SPK_ON    0
#define TCC_SPK_OFF   1

//#undef alsa_dbg
#if 1
#define alsa_dbg(fmt,arg...)    printk("== alsa-debug == "fmt,##arg)
#else
#define alsa_dbg(fmt,arg...)
#endif

// /* audio clock in Hz - rounded from 12.235MHz */
//#define TCC83X_AUDIO_CLOCK 12288000

/* default: Speaker output */
//static int tcc_jack_func = TCC_SPK;
//static int tcc_spk_func;


#if 0
static void spk_mute(void)
{
		gpio_set_value(IMAPX200_GPE(4), 0);
}

static void spk_un_mute(void)
{
		gpio_set_value(IMAPX200_GPE(4), 1);
}

static void hp_mute(void)
{
		gpio_set_value(IMAPX200_GPG(2), 0);
}

static void hp_un_mute(void)
{
		gpio_set_value(IMAPX200_GPG(2), 1);
}

int tcc_hp_is_valid(void)
{
        // gpio_get_value is ==> 0: disconnect, 1: connect
        return (gpio_get_value(IMAPX200_GPG(2))?0:1);
}

void tcc_hp_hw_mute(int flag)
{
    if(flag)
        hp_mute();
    else
        hp_un_mute();

}

void tcc_spk_hw_mute(int flag)
{
    alsa_dbg("%s()  mute[%d]\n", __func__, flag);

    if(flag)
        spk_mute();
    else
        spk_un_mute();
	
}

static void tcc_ext_control(struct snd_soc_codec *codec)
{
	int spk = 0;

    alsa_dbg("%s() tcc_jack_func=%d, bias_level[%d]\n", __func__, tcc_jack_func, codec->bias_level);

	/* set up jack connection */
    if(codec->bias_level == SND_SOC_BIAS_ON) {
    	switch (tcc_jack_func) {
    	case TCC_HP:
            tcc_hp_hw_mute(false);
            tcc_spk_hw_mute(true);
    		break;
    	case TCC_SPK:
            tcc_hp_hw_mute(true);
            tcc_spk_hw_mute(false);
    		break;
    	}
    }
	if (tcc_spk_func == TCC_SPK_ON)
	{
		spk = 1;
		snd_soc_dapm_enable_pin(codec, "Ext Spk");		
	}

	/* signal a DAPM event */
	snd_soc_dapm_sync(codec);
}
#endif

static int tcc_hw_params(struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;
	unsigned int clk = 0;
	int ret = 0;

    	alsa_dbg("%s()\n", __func__);

	switch (params_rate(params)) {
	case 8000:
	case 16000:
	case 48000:
	case 96000:
		clk = 12288000;
		break;
	case 11025:
	case 22050:
    	case 32000:
	case 44100:
		clk = 11289600;
		break;
	}
 
	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, SND_SOC_DAIFMT_I2S |
		SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, SND_SOC_DAIFMT_I2S |
		SND_SOC_DAIFMT_NB_NF | SND_SOC_DAIFMT_CBS_CFS);
	if (ret < 0)
		return ret;

	/* set the codec system clock for DAC and ADC */
	ret = snd_soc_dai_set_sysclk(codec_dai, ES8388_SYSCLK, clk,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;

	/* set the I2S system clock as input (unused) */
   	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, 0, SND_SOC_CLOCK_IN);
	if (ret < 0)
		return ret;
 
	return 0;
}

static struct snd_soc_ops tcc_ops = {
	.hw_params = tcc_hw_params,
};

#if 0
static int tcc_get_jack(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = tcc_jack_func;
	return 0;
}

static int tcc_set_jack(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

    alsa_dbg("%s() tcc_jack_func=%d, ucontrol->value.integer.value[0]=%ld\n", __func__, tcc_jack_func, ucontrol->value.integer.value[0]);

	if (tcc_jack_func == ucontrol->value.integer.value[0])
		return 0;

	tcc_jack_func = ucontrol->value.integer.value[0];
	tcc_ext_control(codec);
	return 1;
}

static int tcc_get_spk(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = tcc_spk_func;
	return 0;
}

static int tcc_set_spk(struct snd_kcontrol *kcontrol, struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec =  snd_kcontrol_chip(kcontrol);

	if (tcc_spk_func == ucontrol->value.integer.value[0])
		return 0;

	tcc_spk_func = ucontrol->value.integer.value[0];
	tcc_ext_control(codec);
	return 1;
}
#endif

static int tcc_hp_event(struct snd_soc_dapm_widget *w,
    struct snd_kcontrol *k, int event)
{
    alsa_dbg("%s() in... event[%d]\n", __func__, event);
    return 0;
}
static int tcc_amp_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *k, int event)
{
	return 0;
}

static int tcc_mic_event(struct snd_soc_dapm_widget *w,
	struct snd_kcontrol *k, int event)
{
	return 0;
}

/* tcc machine dapm widgets */
static const struct snd_soc_dapm_widget tcc_es8388_dapm_widgets[] = {
    SND_SOC_DAPM_HP("Headphone Jack", tcc_hp_event),
    SND_SOC_DAPM_MIC("Mic Jack", tcc_mic_event),
    SND_SOC_DAPM_SPK("Ext Spk", tcc_amp_event),
    SND_SOC_DAPM_LINE("Line Jack", NULL),
    SND_SOC_DAPM_HP("Headset Jack", NULL),
};

/* tcc machine audio map (connections to the codec pins) */
static const struct snd_soc_dapm_route tcc_audio_map[] = {

	/* mic is connected to MICIN (via right channel of headphone jack) */
	{"MICIN", NULL, "Mic Jack"},

	/* headset Jack  - in = micin, out = LHPOUT*/
	{"Headset Jack", NULL, "LOUT1"},
    	{"Headset Jack", NULL, "ROUT1"},

	/* headphone connected to LHPOUT1, RHPOUT1 */
	{"Headphone Jack", NULL, "LOUT1"},
	{"Headphone Jack", NULL, "ROUT1"},

	/* speaker connected to LOUT, ROUT */
	{"Ext Spk", NULL, "ROUT1"},
	{"Ext Spk", NULL, "LOUT1"},

};

static const char *jack_function[] = {"HeadPhone", "Speaker"};
static const char *spk_function[]  = {"On", "Off"};

static const struct soc_enum tcc_enum[] = {
	SOC_ENUM_SINGLE_EXT(2, jack_function),
	SOC_ENUM_SINGLE_EXT(2, spk_function),
};

static const struct snd_kcontrol_new es8388_tcc_controls[] = {
#if 0
	SOC_ENUM_EXT("Jack Function"   , tcc_enum[0], tcc_get_jack, tcc_set_jack),
	SOC_ENUM_EXT("Speaker Function", tcc_enum[1], tcc_get_spk , tcc_set_spk),
#endif
};

/*
 * Logic for a es8388 as connected on a Sharp SL-C7x0 Device
 */
static int tcc_es8388_init(struct snd_soc_codec *codec)
{
	int i, err;

    alsa_dbg("%s() in...\n", __func__);

    snd_soc_dapm_enable_pin(codec, "MICIN");

	/* Add tcc specific controls */
	for (i = 0; i < ARRAY_SIZE(es8388_tcc_controls); i++) {
		err = snd_ctl_add(codec->card,
			snd_soc_cnew(&es8388_tcc_controls[i],codec, NULL));
		if (err < 0)
			return err;
	}

	/* Add tcc specific widgets */
	snd_soc_dapm_new_controls(codec, tcc_es8388_dapm_widgets,
				  ARRAY_SIZE(tcc_es8388_dapm_widgets));

	/* Set up Telechips specific audio path telechips audio_map */
	snd_soc_dapm_add_routes(codec, tcc_audio_map, ARRAY_SIZE(tcc_audio_map));

	snd_soc_dapm_sync(codec);

    alsa_dbg("%s() call snd_soc_jack_new()\n", __func__);
	return 0;
}

static int imapx200_probe(struct platform_device * pdev)
{
	u32 rGPDCON_value;

	rGPDCON_value = readl(rGPDCON);
	rGPDCON_value |= IMAP_GPD0_I2S_SCLK;
	rGPDCON_value |= IMAP_GPD1_I2S_CDCLK;
	rGPDCON_value |= IMAP_GPD2_I2S_LRCLK;
	rGPDCON_value |= IMAP_GPD3_I2S_SDI;
	rGPDCON_value |= IMAP_GPD4_I2S_SDO;
	writel(rGPDCON_value, rGPDCON);

	return 0;
}

extern struct snd_soc_platform tcc_soc_platform;

/* tcc digital audio interface glue - connects codec <--> CPU */
static struct snd_soc_dai_link imapx200_dai[] = {
	{
		.name = "ES8388",
		.stream_name = "ES8388",
		.cpu_dai = &imapx200_i2s_dai[0],
		.codec_dai = &es8388_dai,
		.init = tcc_es8388_init,
		.ops = &tcc_ops,
	},
};


/* tcc audio machine driver */
static struct snd_soc_card imapx200 = {
	.platform = &imapx200_soc_platform,
	.name = "imapx200",
	.probe = imapx200_probe,
	.dai_link = imapx200_dai,
	.num_links = ARRAY_SIZE(imapx200_dai),
};

/* 
 * FIXME: This is a temporary bodge to avoid cross-tree merge issues. 
 * New drivers should register the es8388 I2C device in the machine 
 * setup code (under arch/arm for ARM systems). 
 */
 
static int es8388_i2c_register(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;

    alsa_dbg("%s()\n", __func__);

    memset(&info, 0, sizeof(struct i2c_board_info));

    // The Bit 0 value of I2C subaddress is AD0 pin value.
    info.addr = 0x10;
	printk("###es8388_i2c_register info.addr = 0x%x\n", info.addr);
    strlcpy(info.type, "es8388", I2C_NAME_SIZE);

    adapter = i2c_get_adapter(1);
    if (!adapter) 
    {
        printk(KERN_ERR "can't get i2c adapter \n");
        return -ENODEV;
    }

    client = i2c_new_device(adapter, &info);
    i2c_put_adapter(adapter);
    if (!client) 
    {
        printk(KERN_ERR "can't add i2c device at 0x%x\n", (unsigned int)info.addr);
        return -ENODEV;
    }
    return 0;
}


/* tcc audio subsystem */
static struct snd_soc_device imapx200_snd_devdata = {
	.card = &imapx200,
	.codec_dev = &soc_codec_dev_es8388,
};

static struct platform_device *imapx200_snd_device;


extern int es8388_device_exist;

static int __init imapx200_init_es8388(void)
{

	int ret;

    printk("%s() ++\n", __func__);

#ifdef IMAPX200_CONFIG_AUDIO_POWER_ON
	if (gpio_request(IMAPX200_GPO(12), "power on"))
		return 0;

	gpio_direction_output(IMAPX200_GPO(12), 1);
#endif
	printk("###es8388!\n");

    ret = es8388_i2c_register();

    if(!es8388_device_exist){
        printk("es8388 device not exist\n");
        return 0;
    }

	imapx200_snd_device = platform_device_alloc("soc-audio", -1);
	if (!imapx200_snd_device)
		return -ENOMEM;

	platform_set_drvdata(imapx200_snd_device, &imapx200_snd_devdata);
	imapx200_snd_devdata.dev = &imapx200_snd_device->dev;
	ret = platform_device_add(imapx200_snd_device);

	if (ret)
		platform_device_put(imapx200_snd_device);


    printk("%s() --\n", __func__);

	return ret;
}

static void __exit imapx200_exit_es8388(void)
{
    printk("%s() \n", __func__);

	platform_device_unregister(imapx200_snd_device);
}

module_init(imapx200_init_es8388);
module_exit(imapx200_exit_es8388);

/* Module information */
MODULE_AUTHOR("linux <linux@telechips.com>");
MODULE_DESCRIPTION("ALSA SoC TCCxx Board (WM8988)");
MODULE_LICENSE("GPL");

