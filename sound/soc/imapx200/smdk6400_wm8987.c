/*
 * smdk6400_wm8987.c  --  SoC audio for Neo1973
 *
 * Copyright 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    20th Jan 2007   Initial version.
 *    05th Feb 2007   Rename all to Neo1973
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h> //lzcx
//#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>
#include <mach/regs-iis.h>

#include <mach/gpio.h>
#include <mach/hardware.h>
#include <plat/audio.h>
#include <asm/io.h>
#include <plat/gpio-cfg.h>
#include <plat/gpio-bank-b.h>
//#include <mach/spi-gpio.h>
#include <plat/map-base.h>
#ifdef CONFIG_CPU_S3C6400
#include <asm/arch/regs-s3c6400-clock.h>
#elif defined CONFIG_CPU_S3C6410
#include <plat/regs-clock.h>
#else

#endif

#include "../codecs/wm8987.h"
#include "s3c-pcm.h"
#include "s3c-i2s.h"
//#include "s3c64xx-i2s.h"

/* define the scenarios */
#define SMDK6400_AUDIO_OFF		1
#define SMDK6400_CAPTURE_MIC1		3
#define SMDK6400_STEREO_TO_HEADPHONES	0
#define SMDK6400_CAPTURE_LINE_IN	2
#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define s3cdbg(x...) printk(x)
#else
#define s3cdbg(x...)
#endif

static struct snd_soc_card smdk6400;

static int smdk6400_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;

	unsigned int pll_out = 0, bclk = 0;
	int ret = 0;
	unsigned int iispsr, iismod;
	unsigned int prescaler = 4;
	s3cdbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));
	printk(KERN_INFO "CPU_DAI name is %s\n",cpu_dai->name);
	/*PCLK & SCLK gating enable*/
	writel(readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_IIS0, S3C_PCLK_GATE);
	writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0, S3C_SCLK_GATE);

	iismod = readl(S3C_IIS0MOD);
	iismod &=~(0x3<<3);

	/*Clear I2S prescaler value [13:8] and disable prescaler*/
	iispsr = readl(S3C_IIS0PSR);	
	iispsr &=~((0x3f<<8)|(1<<15)); 
	writel(iispsr, S3C_IIS0PSR);
	
	switch (params_rate(params)) {
	case 16000:
	case 32000:
	case 96000:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(128<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		break;
	case 8000:
		prescaler = 0xe; 
	case 48000:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(192<<16)|(25<<8)|(0<<0) ,S3C_EPLL_CON0);
		break;
	case 11025:
		prescaler = 9; 
	case 22050:
	case 44100:
		writel(0, S3C_EPLL_CON1);
		writel((1<<31)|(254<<16)|(9<<8)|(2<<0) ,S3C_EPLL_CON0);
		break;
	default:
		break;
	}

	s3cdbg("%s, IISCON: %x IISMOD: %x,IISFIC: %x,IISPSR: %x",
			__FUNCTION__ , readl(S3C_IIS0CON), readl(S3C_IIS0MOD), 
			readl(S3C_IIS0FIC), readl(S3C_IIS0PSR));
	
	while(!(__raw_readl(S3C_EPLL_CON0)&(1<<30)));

	/* MUXepll : FOUTepll */
	writel(readl(S3C_CLK_SRC)|S3C_CLKSRC_EPLL_CLKSEL, S3C_CLK_SRC);
	/* AUDIO0 sel : FOUTepll */
	//writel((readl(S3C_CLK_SRC)&~(0x7<<7))|(0<<7), S3C_CLK_SRC);
	writel((readl(S3C_CLK_SRC)&~(0x7<<7))|(0x0<<7), S3C_CLK_SRC); //lzcx 

	/* CLK_DIV2 setting */
	writel(0x0,S3C_CLK_DIV2);

	switch (params_rate(params)) {
	case 8000:
		iismod |= S3C_IIS0MOD_768FS;	
		pll_out = 12288000;
		//pll_out = 6144000; //lzcx
		break;
	case 11025:
		iismod |= S3C_IIS0MOD_768FS;	
		//bclk = WM8987_BCLK_DIV_16;
		bclk = WM8987_BCLK_DIV_8 | (1<<6); //lzcx
		pll_out = 16934400;
		break;
	case 16000:
		iismod |= S3C_IIS0MOD_768FS;	
		bclk = WM8987_BCLK_DIV_2;
		pll_out = 12288000;
		break;
	case 22050:
		iismod |= S3C_IIS0MOD_768FS;	
		bclk = WM8987_BCLK_DIV_8;
		pll_out = 16934400;
		break;
	case 96000: //lzcx add
	case 32000:
		iismod |= S3C_IIS0MOD_384FS;
		bclk = WM8987_BCLK_DIV_2;
		pll_out = 12288000;
		break;
	case 44100:
		iismod |= S3C_IIS0MOD_384FS;
		bclk = WM8987_BCLK_DIV_4;
		pll_out = 16934400;
		break;
	case 48000:
		iismod |= S3C_IIS0MOD_384FS;
		bclk = WM8987_BCLK_DIV_4;
		pll_out = 18432000;
		break;
	}

	writel(iismod , S3C_IIS0MOD);

	printk("\nIISMOD: %x\n",readl(S3C_IIS0MOD));
	
	/* set codec DAI configuration */
	ret = codec_dai->ops->set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0)
		{ printk("aaa\n");
		return ret;
		}

	/* set cpu DAI configuration */
	ret = cpu_dai->ops->set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0)
		{ printk("aaa1\n");
		return ret;
		}

	/* set the codec system clock for DAC and ADC */
	ret = codec_dai->ops->set_sysclk(codec_dai, WM8987_MCLK, pll_out,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		{ printk("aaa2 pll_out %ld \n",pll_out);
		return ret;
		}

	/* set MCLK division for sample rate */
#if 1
	ret = cpu_dai->ops->set_clkdiv(cpu_dai, S3C24XX_DIV_MCLK,
		S3C2410_IISMOD_32FS );
	if (ret < 0)
		{ printk("aaa3\n");
		return ret;
		}
#endif
	/* set codec BCLK division for sample rate */
	ret = codec_dai->ops->set_clkdiv(codec_dai, WM8987_BCLKDIV, bclk);
	if (ret < 0)
		{ printk("aaa4\n");
		return ret;
		}

	/* set prescaler division for sample rate */
#if 1
	ret = cpu_dai->ops->set_clkdiv(cpu_dai, S3C24XX_DIV_PRESCALER,
		(prescaler << 0x8));
	if (ret < 0)
		{ printk("aaa5\n");
		return ret;
		}
#endif
	return 0;
}

static int smdk6400_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;

	/* disable the PLL */
	return 0;//lzcx codec_dai->dai_ops.set_pll(codec_dai, WM8987_PLL1, 0, 0);
	//return codec_dai->dai_ops.set_pll(codec_dai, WM8987_PLL1, 0, 0);
}

/*
 * Neo1973 WM8987 HiFi DAI opserations.
 */
static struct snd_soc_ops smdk6400_hifi_ops = {
	.hw_params = smdk6400_hifi_hw_params,
	.hw_free = smdk6400_hifi_hw_free,
};

static int smdk6400_scenario = 0;

static int smdk6400_get_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = smdk6400_scenario;
	return 0;
}

static int set_scenario_endpoints(struct snd_soc_codec *codec, int scenario)
{
	switch(smdk6400_scenario) {
	printk("smdk6400_scenario is %d\n",smdk6400_scenario);//lzcx
	case SMDK6400_AUDIO_OFF:
		printk(KERN_INFO "----------SMDK6400_AUDIO_OFF!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic Bias");
		snd_soc_dapm_disable_pin(codec, "Line In Jack");
		break;
	case SMDK6400_STEREO_TO_HEADPHONES:
		printk(KERN_INFO "-----------SMDK6400_STEREO_TO_HEADPHONE!\n");
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;
#if 0
	case SMDK6400_CAPTURE_MIC1:
		printk(KERN_INFO "-----------SMDK6400_CAPTURE_MIC1!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_disable_pin(codec, "Line In Jack");
		break;
	case SMDK6400_CAPTURE_LINE_IN:
		printk(KERN_INFO "------------SMDK6400_CAPTURE_LINE_IN!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;
#endif
	default:
		printk(KERN_INFO "-------------default!\n");
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;
	}

	snd_soc_dapm_sync(codec);

	return 0;
}

static int smdk6400_set_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (smdk6400_scenario == ucontrol->value.integer.value[0])
		return 0;

	smdk6400_scenario = ucontrol->value.integer.value[0];
	set_scenario_endpoints(codec, smdk6400_scenario);
	return 1;
}

static const struct snd_soc_dapm_widget wm8987_dapm_widgets[] = {
	SND_SOC_DAPM_HP("Headphone Jack", NULL),
	SND_SOC_DAPM_MIC("Mic Bias", NULL),
	SND_SOC_DAPM_LINE("Line In Jack", NULL),
};


/* example machine audio_mapnections */
static const char* audio_map[][3] = {

	{"Headphone Jack", NULL, "LOUT2"},
	{"Headphone Jack", NULL, "ROUT2"}, //lzcx 1

//	mic is connected to line2  //lzcx
	{ "LINPUT2", NULL, "Mic Bias" }, 
	{ "Mic Bias", NULL, "Mic Jack" }, //lzcxv

	{"LINPUT1", NULL, "Line In Jack"},
	{"RINPUT1", NULL, "Line In Jack"},

#if 0
	/* Connect the ALC pins */
	{"ACIN", NULL, "ACOP"},
#endif
		
	{NULL, NULL, NULL},
};

static const char *smdk_scenarios[] = {
	"Speaker",
	"Mute",
#if 0
	"Capture Line In",
	"Headphones",
	"Capture Mic1",
#endif
};

static const struct soc_enum smdk_scenario_enum[] = {
	SOC_ENUM_SINGLE_EXT(ARRAY_SIZE(smdk_scenarios),smdk_scenarios),
};

static const struct snd_kcontrol_new wm8987_smdk6400_controls[] = {
#if 0
	SOC_ENUM_EXT("SMDK Mode", smdk_scenario_enum[0],
		smdk6400_get_scenario, smdk6400_set_scenario),
#endif
};

/*
 * This is an example machine initialisation for a wm8987 connected to a
 * smdk6400. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int smdk6400_wm8987_init(struct snd_soc_codec *codec)
{
	int i, err;

	/* set endpoints to default mode */
//	set_scenario_endpoints(codec, SMDK6400_AUDIO_OFF);

	/* Add smdk6400 specific widgets */
	for (i = 0; i < ARRAY_SIZE(wm8987_dapm_widgets); i++)
		snd_soc_dapm_new_control(codec, &wm8987_dapm_widgets[i]);

	/* add smdk6400 specific controls */
	for (i = 0; i < ARRAY_SIZE(wm8987_smdk6400_controls); i++) {
		err = snd_ctl_add(codec->card,
				snd_soc_cnew(&wm8987_smdk6400_controls[i],
				codec, NULL));
		if (err < 0)
			return err;
	}

	/* set up smdk6400 specific audio path audio_mapnects */
#if 1
	for (i = 0; audio_map[i][0] != NULL; i++) {
		snd_soc_dapm_connect_input(codec, audio_map[i][0],
			audio_map[i][1], audio_map[i][2]);
	}
#endif

//	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));
	/* always connected */
	snd_soc_dapm_enable_pin(codec, "Mic Bias");
	printk(KERN_INFO "Mic Bias!\n");
	snd_soc_dapm_enable_pin(codec, "Headphone Jack");
	printk(KERN_INFO "Headphone Jack!\n");
	snd_soc_dapm_enable_pin(codec, "Line In Jack");
	printk(KERN_INFO "Line In Jack!\n");
	snd_soc_dapm_sync(codec);
	return 0;
}

static int smdk6400_probe(struct platform_device *pdev)
{
	printk(KERN_INFO "smdk6400_probe!\n");
	/*Set I2C port to controll WM8987 codec*/
	s3c_gpio_setpull(S3C64XX_GPB(5), S3C_GPIO_PULL_DOWN);
	s3c_gpio_setpull(S3C64XX_GPB(6), S3C_GPIO_PULL_DOWN);
	s3c_gpio_cfgpin(S3C64XX_GPB(5), S3C64XX_GPB5_I2C_SCL0);
	s3c_gpio_cfgpin(S3C64XX_GPB(6), S3C64XX_GPB6_I2C_SDA0);
	//s3c_gpio_cfgpin(S3C_GPL1, S3C_GPL1_OUTP);
	//s3c_gpio_setpin(S3C_GPL1, 1);
	return 0;
}

static struct snd_soc_dai_link smdk6400_dai[] = {
{ /* Hifi Playback - for similatious use with voice below */
	.name = "WM8987",
	.stream_name = "WM8987 HiFi",
	.cpu_dai = &s3c_i2s_dai[0],
	//.codec_dai = &wm8987_dai[WM8987_DAI_HIFI],
	.codec_dai = &wm8987_dai,
	.init = smdk6400_wm8987_init,
	.ops = &smdk6400_hifi_ops,
},
};

static struct snd_soc_card smdk6400 = {
	.name = "smdk6400",
	.probe = smdk6400_probe,
	.platform = &s3c24xx_soc_platform,
	.dai_link = smdk6400_dai,
	.num_links = ARRAY_SIZE(smdk6400_dai),
};

static struct wm8987_setup_data smdk6400_wm8987_setup = {
	.i2c_address = 0x1a,
};

static struct snd_soc_device smdk6400_snd_devdata = {
	.card = &smdk6400,
//	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm8987,
	.codec_data = &smdk6400_wm8987_setup,
};

static struct platform_device *smdk6400_snd_device;

#ifdef	CONFIG_AUDIO_CODEC_PROCFS

// This function forces any delayed work to be queued and run.
static int proc_run_delayed_work(struct delayed_work *dwork)
{
	int ret;

	// cancel any work waiting to be queued.
	ret = cancel_delayed_work(dwork);

	// if there was any work waiting then we run it now and
	//  wait for it's completion.
	if (ret) {
		schedule_delayed_work(dwork, 0);
		flush_scheduled_work();
	}

	return ret;
}

static int aud_proc_read(char* page, char** start, off_t off, int count,
	int* eof, void* data)
{
    struct snd_soc_codec* codec = (smdk6400_snd_devdata.card)->codec;

    if (!codec || !codec->read) return count;

    if (off) return 0;			data = (void*)page;

    page += sprintf(page, "%s registers cached settings: ", codec->name);
    for (count=0; count < codec->reg_cache_size; ++count) {
	if (!(count % 16)) page += sprintf(page, "\n   R%02x:  ", count);
	page += sprintf(page, "%03x ", codec->read(codec, count));
	if ((count % 8) == 7) page += sprintf(page, "  ");
    }
    
    return ((page += sprintf(page, "\n")) - (char*)data);
}


static int aud_proc_write(struct file* file, const char* buffer,
	unsigned long count, void* data)
{
#define	MAX_BUFLEN	16
    u8 reg;
    u16 val = MAX_BUFLEN - 1;
    char *ptr, tmp_buf[MAX_BUFLEN];
    struct snd_soc_codec* codec = (smdk6400_snd_devdata.card)->codec;

    if (!codec || !codec->write) return count;

    if (count < MAX_BUFLEN) val = count - 1;  tmp_buf[val] = 0;
    if (copy_from_user(tmp_buf, buffer, val)) return -EFAULT;

    for (ptr = tmp_buf; isspace(*ptr); ++ptr) ;

#if 0
	if (strncasecmp(ptr, "FmOn", val) == 0) {	// enter fmmode
		ptr += 4;
		proc_run_delayed_work(&smdk6400_snd_devdata.delayed_work);
		cancel_delayed_work(&codec->delayed_work);
		codec->write(codec, 0x1A, 0x018);
		codec->write(codec, 0x22, 0x0D0);
		codec->write(codec, 0x25, 0x0D0);
		return count;
	} else
	if (strncasecmp(ptr, "FmOff", val) == 0) {	// leave fmmode
		ptr += 5;
		codec->write(codec, 0x1A, 0x000);
		codec->write(codec, 0x22, 0x150);
		codec->write(codec, 0x25, 0x150);
		return count;
	} else
	if (strncasecmp(ptr, "FmRecOn", val) == 0) {	// record fm
#if 0
		struct snd_soc_dapm_path *p;
		list_for_each_entry(p, &codec->dapm_paths, list) {
			if(strncmp(p->sink->name, "Left PGA Mux", 12)) continue;
			if(!p->connect && !strcmp(p->source->name, "LINPUT1"))
				p->connect = 1;
			if(p->connect && !strnmp(p->source->name, "LINPUT2"))
				p->connect = 0;
		}
#endif
		proc_run_delayed_work(&smdk6400_snd_devdata.delayed_work);
		cancel_delayed_work_sync(&codec->delayed_work);
	  	codec->write(codec, 0x20, 0x000);
		//smdk6400_snd_devdata.s_rec = 1;
		ptr += 7;
		return count;
	} else
	if (strncasecmp(ptr, "FmRecOff", val) == 0) {	// leave record fm
	  	codec->write(codec, 0x20, 0x060);
		//smdk6400_snd_devdata.s_rec = 0;
		ptr += 8;
		return count;
	}
#endif //lzcx

    reg = simple_strtoul(ptr, &ptr, 16);

    if (!(reg < codec->reg_cache_size)) {
	printk(KERN_DEBUG "wrong register no %d, max %d\n",
		reg, codec->reg_cache_size);
	return count;
    }

    while (isspace(*ptr)) ++ptr;
    val = simple_strtoul(ptr, &ptr, 16);

    if (codec->write(codec, reg, val)) ;

    return count;
}

#if 0 	/* comment by mhfan */
/*
 * Initial:
 * R00:  157 157 1f9 1f9 000 008 000 00a   000 000 1ff 1ff 00f 00f 000 000   
 * R10:  000 07b 000 032 000 1c3 1c3 0c5   050 141 000 000 000 000 000 000   
 * R20:  0a0 000 150 050 050 150 050 050   1f9 1f9 079 
 *
 * Playback:
 * R00:  157 157 1f9 1f9 000 000 000 042   023 000 1ff 1ff 00f 00f 000 000   
 * R10:  000 07b 000 032 000 1c3 1c3 0c5   050 0c0 1fa 000 000 000 000 000   
 * R20:  0a0 000 150 050 050 150 050 050   1f9 1f9 079 (44.1 KHz)
 *
 * R00:  157 157 1f9 1f9 000 000 000 042   037 000 1ff 1ff 00f 00f 000 000   
 * R10:  000 07b 000 032 000 1c3 1c3 0c5   050 0c0 1fa 000 000 000 000 000   
 * R20:  0a0 000 150 050 050 150 050 050   1f9 1f9 079 (22050 Hz)
 *
 * Capture:
 */
#endif	/* comment by mhfan */

#define	AUD_PROC_ENTRY		"driver/audregs"

static int __init aud_proc_init(void)
{
    struct proc_dir_entry* aud_entry;

    if (!(aud_entry = create_proc_entry(AUD_PROC_ENTRY,
	    S_IRUGO | S_IWUSR, NULL))) return -ENOMEM;
	
    printk(KERN_INFO "Proc-FS interface for audio codec\n");

//    aud_entry->owner	  = THIS_MODULE;
    aud_entry->write_proc = aud_proc_write;
    aud_entry->read_proc  = aud_proc_read;
    aud_entry->data	  = NULL;

    return 0;
}

static void __exit aud_proc_exit(void)
{
    remove_proc_entry(AUD_PROC_ENTRY, NULL);
}
#endif//CONFIG_AUDIO_CODEC_PROCFS


static int __init smdk6400_init(void)
{
	int ret;
	printk(KERN_INFO "-------------smdk6400_init!\n");
	smdk6400_snd_device = platform_device_alloc("soc-audio", -1);
	if (!smdk6400_snd_device)
		return -ENOMEM;
	platform_set_drvdata(smdk6400_snd_device, &smdk6400_snd_devdata);
	smdk6400_snd_devdata.dev = &smdk6400_snd_device->dev;
	ret = platform_device_add(smdk6400_snd_device);

	if (ret)
		platform_device_put(smdk6400_snd_device);
#ifdef	CONFIG_AUDIO_CODEC_PROCFS
    if (aud_proc_init()) ;
#endif//CONFIG_AUDIO_CODEC_PROCFS


	return ret;
}

static void __exit smdk6400_exit(void)
{
	platform_device_unregister(smdk6400_snd_device);
}

module_init(smdk6400_init);
module_exit(smdk6400_exit);

/* Module information */
MODULE_AUTHOR("Graeme Gregory, graeme.gregory@wolfsonmicro.com, www.wolfsonmicro.com");
MODULE_DESCRIPTION("ALSA SoC WM8987 Neo1973");
MODULE_LICENSE("GPL");
