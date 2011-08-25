/********************************************************************************
** linux-2.6.28.5/sound/soc/imapx200/imapx200_wm8987.c
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


#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/proc_fs.h>
#include <linux/ctype.h> 
//#include <sound/driver.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <sound/soc-dapm.h>

#include <asm/mach-types.h>
#include <asm/hardware/scoop.h>
#include <mach/imapx_iis.h>

#include <mach/gpio.h>
#include <mach/hardware.h>
#include <asm/io.h>
#include <mach/imapx_base_reg.h>
#include <mach/imapx_gpio.h>
#include <plat/gpio-bank-c.h>
#include <plat/gpio-bank-d.h>
#include <mach/imapx_sysmgr.h>
#include <linux/gpio.h>
//#include <mach/spi-gpio.h>
/*******************************************************/
#ifdef CONFIG_CPU_S3C6400
#include <asm/arch/regs-s3c6400-clock.h>
#elif defined CONFIG_CPU_S3C6410
#include <plat/regs-clock.h>
#else

#endif
/********************************************************/
#include "../codecs/wm8987.h"
#include "imapx200-pcm.h"
#include "imapx200-i2s.h"
//#include "s3c64xx-i2s.h"

#define IMAPX200_CONFIG_AUDIO_POWER_ON

#if defined(CONFIG_BOARD_B0)

#define GPIO_AUDIO_PWR_EN  IMAPX200_GPO(2)
#define GPIO_AUDIO_ON_VAL  1

#else

#define GPIO_AUDIO_PWR_EN  IMAPX200_GPO(12)
#define GPIO_AUDIO_ON_VAL  1

#endif


/* define the scenarios */
#define IMAPX200_AUDIO_OFF		1
#define IMAPX200_CAPTURE_MIC1		3
#define IMAPX200_STEREO_TO_HEADPHONES	0
#define IMAPX200_CAPTURE_LINE_IN	2
#define CONFIG_SND_DEBUG
#ifdef CONFIG_SND_DEBUG
#define imapx200dbg(x...) printk(x)
#else
#define imapx200dbg(x...)
#endif

static struct snd_soc_card imapx200;

static int imapx200_hifi_hw_params(struct snd_pcm_substream *substream,
	struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->dai->cpu_dai;

	unsigned int pll_out = 0, bclk = 0;
	int ret = 0;
	unsigned int iispsr, iisdiv, iiscdclk_div;
	unsigned int prescaler = 4;
	unsigned long pclkmask;

	imapx200dbg("Entered %s, rate = %d\n", __FUNCTION__, params_rate(params));
	/********************************************************************/
	/*PCLK & SCLK gating enable*/
	//writel(readl(S3C_PCLK_GATE)|S3C_CLKCON_PCLK_IIS0, S3C_PCLK_GATE);
	//writel(readl(S3C_SCLK_GATE)|S3C_CLKCON_SCLK_AUDIO0, S3C_SCLK_GATE);
	pclkmask = readl(rPCLK_MASK);
	pclkmask &= ~IMAP_CLKCON_PCLK_IIS;
	writel(pclkmask, rPCLK_MASK);
//	writel(readl(rPCLK_MASK)& ~IMAP_CLKCON_PCLK_IIS, rPCLK_MASK);
//	writel(readl(rSCLK_MASK)& ~IMAP_CLKCON_SCLK_IIS, rSCLK_MASK);
	/********************************************************************/
	switch (params_rate(params)) {
	/*
	case 8000:
		iisdiv = WM8987_1536FS;	
		pll_out = 12288000;
		break;
	case 11025:
		iisdiv = WM8987_1536FS;	
		pll_out = 16934400;
		break;
	case 16000:
		iisdiv = WM8987_768FS;	
		pll_out = 12288000;
		break;
	case 22050:
		//iisdiv = WM8987_768FS;	
		iisdiv = WM8987_512FS;	
		//pll_out = 16934400;
		pll_out = 11289600;
		break;
	case 32000:
		iisdiv = WM8987_384FS;
		pll_out = 12288000;
		break;
	case 44100:
		iisdiv = WM8987_384FS;
		//iisdiv = WM8987_256FS;
		pll_out = 16934400;
		//pll_out = 11289600;
		break;
	case 48000:
		iisdiv = WM8987_384FS;
		pll_out = 18432000;
		break;
	case 96000:
		iisdiv = WM8987_128FS;
		pll_out = 12288000;
		break;
	*/
	case 44100:
	    iisdiv = 272;
	    pll_out = 12000000;
	    break;
	default:
	    printk("imapx200_hifi_hw_params failed sample rate =%d\n",params_rate(params));
	}
//	iiscdclk_div = (IMAPX200_IIS_CLK - pll_out)/pll_out;
//	iisclk_dev = (IMAPX200_IIS_CLK - );

	/* set codec DAI configuration */
	ret = codec_dai->ops->set_fmt(codec_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0)
		{ printk("aaa\n");
		return ret;
		}
	/* set cpu DAI configuration */
	/*******************************************************************/
	//in s3c6410, this function does nothing.
	ret = cpu_dai->ops->set_fmt(cpu_dai,
		SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
		SND_SOC_DAIFMT_CBS_CFS ); 
	if (ret < 0)
		{ printk("aaa1\n");
		return ret;
		}

	/*******************************************************************/
	/* set the codec system clock for DAC and ADC */
	ret = codec_dai->ops->set_sysclk(codec_dai, WM8987_MCLK, pll_out,
		SND_SOC_CLOCK_IN);
	if (ret < 0)
		{ printk("aaa2 pll_out %ld \n",pll_out);
		return ret;
		}

	/* set MCLK division for sample rate */
	/**********************************************************************/
	ret = cpu_dai->ops->set_sysclk(cpu_dai, iisdiv,
		params_rate(params), params_format(params) );
	if (ret < 0)
		{ printk("aaa3\n");
		return ret;
		}

	/***********************************************************************/
	/* set codec BCLK division for sample rate */
	ret = codec_dai->ops->set_clkdiv(codec_dai, WM8987_BCLKDIV, bclk);
	if (ret < 0)
		{ printk("aaa4\n");
		return ret;
		}

	/* set prescaler division for sample rate */
	/***********************************************************************/
#if 1
	ret = cpu_dai->ops->set_clkdiv(cpu_dai, 0,0);
	if (ret < 0)
		{ printk("aaa5\n");
		return ret;
		}

#endif
	return 0;
	/************************************************************************/
}

static int imapx200_hifi_hw_free(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->dai->codec_dai;

	return 0;
}

/*
 * Neo1973 WM8987 HiFi DAI opserations.
 */
static struct snd_soc_ops imapx200_hifi_ops = {
	.hw_params = imapx200_hifi_hw_params,
	.hw_free = imapx200_hifi_hw_free,
};

static int imapx200_scenario = 0;

static int imapx200_get_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	ucontrol->value.integer.value[0] = imapx200_scenario;
	return 0;
}

static int set_scenario_endpoints(struct snd_soc_codec *codec, int scenario)
{
	switch(imapx200_scenario) {
	printk("imapx200_scenario is %d\n",imapx200_scenario);
	case IMAPX200_AUDIO_OFF:
		printk(KERN_INFO "----------IMAPX200_AUDIO_OFF!\n");
		snd_soc_dapm_disable_pin(codec, "Headphone Jack");
		snd_soc_dapm_disable_pin(codec, "Mic Bias");
		snd_soc_dapm_disable_pin(codec, "Line In Jack");
		break;
	case IMAPX200_STEREO_TO_HEADPHONES:
		printk(KERN_INFO "-----------IMAPX200_STEREO_TO_HEADPHONE!\n");
		snd_soc_dapm_enable_pin(codec, "Headphone Jack");
		snd_soc_dapm_enable_pin(codec, "Mic Bias");
		snd_soc_dapm_enable_pin(codec, "Line In Jack");
		break;

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

static int imapx200_set_scenario(struct snd_kcontrol *kcontrol,
	struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_kcontrol_chip(kcontrol);

	if (imapx200_scenario == ucontrol->value.integer.value[0])
		return 0;

	imapx200_scenario = ucontrol->value.integer.value[0];
	set_scenario_endpoints(codec, imapx200_scenario);
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
	{"Headphone Jack", NULL, "ROUT2"}, 

//	mic is connected to line2  
	{ "LINPUT2", NULL, "Mic Bias" }, 
	{ "Mic Bias", NULL, "Mic Jack" },

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

static const struct snd_kcontrol_new wm8987_imapx200_controls[] = {
#if 0
	SOC_ENUM_EXT("SMDK Mode", smdk_scenario_enum[0],
		imapx200_get_scenario, imapx200_set_scenario),
#endif
};

/*
 * This is an example machine initialisation for a wm8987 connected to a
 * imapx200. It is missing logic to detect hp/mic insertions and logic
 * to re-route the audio in such an event.
 */
static int imapx200_wm8987_init(struct snd_soc_codec *codec)
{
	int i, err;

	/* set endpoints to default mode */
//	set_scenario_endpoints(codec, IMAPX200_AUDIO_OFF);

	/* Add imapx200 specific widgets */
	for (i = 0; i < ARRAY_SIZE(wm8987_dapm_widgets); i++)
		snd_soc_dapm_new_control(codec, &wm8987_dapm_widgets[i]);

	/* add imapx200 specific controls */
	for (i = 0; i < ARRAY_SIZE(wm8987_imapx200_controls); i++) {
		err = snd_ctl_add(codec->card,
				snd_soc_cnew(&wm8987_imapx200_controls[i],
				codec, NULL));
		if (err < 0)
			return err;
	}

	/* set up imapx200 specific audio path audio_mapnects */
#if 1
	for (i = 0; audio_map[i][0] != NULL; i++) {
		snd_soc_dapm_connect_input(codec, audio_map[i][0],
			audio_map[i][1], audio_map[i][2]);
	}
#endif

//	snd_soc_dapm_add_routes(codec, audio_map, ARRAY_SIZE(audio_map));
	/* always connected */
	snd_soc_dapm_enable_pin(codec, "Mic Bias");
	snd_soc_dapm_enable_pin(codec, "Headphone Jack");
	snd_soc_dapm_enable_pin(codec, "Line In Jack");
	snd_soc_dapm_sync(codec);
	return 0;
}

static int imapx200_probe(struct platform_device *pdev)
{

	u32 rGPDCON_value, rGPDPUD_value, rGPCCON_value;
//	printk(KERN_INFO "imapx200_probe!\n");
	//configure GPD and GPC for IIS and IIC
	rGPDCON_value = readl(rGPDCON);
	rGPDCON_value |= IMAP_GPD0_I2S_SCLK;
	rGPDCON_value |= IMAP_GPD1_I2S_CDCLK;
	rGPDCON_value |= IMAP_GPD2_I2S_LRCLK;
	rGPDCON_value |= IMAP_GPD3_I2S_SDI;
	rGPDCON_value |= IMAP_GPD4_I2S_SDO;
	writel(rGPDCON_value, rGPDCON);	

	/*pull up GPO2 for wm8987 power supply*/
	//writel((readl(rGPOCON)& ~(0x3<<4)) | 0x1<<4, rGPOCON);
	//writel(readl(rGPODAT) | 0x1<<2, rGPODAT);
	

	return 0;
}

static struct snd_soc_dai_link imapx200_dai[] = {
{ /* Hifi Playback - for similatious use with voice below */
	.name = "WM8987",
	.stream_name = "WM8987 HiFi",
	.cpu_dai = &imapx200_i2s_dai[0],
	//.codec_dai = &wm8987_dai[WM8987_DAI_HIFI],
	.codec_dai = &wm8987_dai,
	.init = imapx200_wm8987_init,
	.ops = &imapx200_hifi_ops,
},
};

static struct snd_soc_card imapx200 = {
	.name = "imapx200",
	.probe = imapx200_probe,
	.platform = &imapx200_soc_platform,
	.dai_link = imapx200_dai,
	.num_links = ARRAY_SIZE(imapx200_dai),
};

static struct wm8987_setup_data imapx200_wm8987_setup = {
	.i2c_address = 0x1a, // 0x1a
	.i2c_bus = 0x1,
};

static struct snd_soc_device imapx200_snd_devdata = {
	.card = &imapx200,
//	.platform = &s3c24xx_soc_platform,
	.codec_dev = &soc_codec_dev_wm8987,
	.codec_data = &imapx200_wm8987_setup,
};

static struct platform_device *imapx200_snd_device;

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
    struct snd_soc_codec* codec = (imapx200_snd_devdata.card)->codec;

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
    struct snd_soc_codec* codec = (imapx200_snd_devdata.card)->codec;

    if (!codec || !codec->write) return count;

    if (count < MAX_BUFLEN) val = count - 1;  tmp_buf[val] = 0;
    if (copy_from_user(tmp_buf, buffer, val)) return -EFAULT;

    for (ptr = tmp_buf; isspace(*ptr); ++ptr) ;


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

static int wm8987_i2c_register(void)
{
    struct i2c_board_info info;
    struct i2c_adapter *adapter;
    struct i2c_client *client;

    printk("%s()\n", __func__);

    memset(&info, 0, sizeof(struct i2c_board_info));

    // The Bit 0 value of I2C subaddress is AD0 pin value.
    info.addr = 0x1a;
	printk("###wm8987_i2c_register info.addr = 0x%x\n", info.addr);
    strlcpy(info.type, "wm8987", I2C_NAME_SIZE);

    adapter = i2c_get_adapter(1);
    if (!adapter) 
    {
        printk(KERN_ERR "can't get i2c adapter \n");
        return -ENODEV;
    }
    printk("adapter 0x%x\n", adapter);

    client = i2c_new_device(adapter, &info);
    printk("client 0x%x\n", client);
    i2c_put_adapter(adapter);
    if (!client) 
    {
        printk(KERN_ERR "can't add i2c device at 0x%x\n", (unsigned int)info.addr);
        return -ENODEV;
    }
    return 0;
}


//extern int wm8987_device_exist;
static int __init imapx200_init(void)
{
	int ret;
	printk(KERN_INFO "-------------imapx200_init!\n");

#ifdef IMAPX200_CONFIG_AUDIO_POWER_ON
	if (gpio_request(GPIO_AUDIO_PWR_EN, "power on"))
		return 0;	
	gpio_direction_output(GPIO_AUDIO_PWR_EN,GPIO_AUDIO_ON_VAL);
	
#endif
	printk(KERN_INFO "wm8988!\n");

    /*
    ret = wm8987_i2c_register();

    if(!wm8987_device_exist){
        printk("wm8987 device not exist\n");
        return 0;
    }
    */

	imapx200_snd_device = platform_device_alloc("soc-audio", -1);
	if (!imapx200_snd_device)
		return -ENOMEM;
	platform_set_drvdata(imapx200_snd_device, &imapx200_snd_devdata);
	imapx200_snd_devdata.dev = &imapx200_snd_device->dev;
	ret = platform_device_add(imapx200_snd_device);

	if (ret)
		platform_device_put(imapx200_snd_device);
#ifdef	CONFIG_AUDIO_CODEC_PROCFS
    if (aud_proc_init()) ;
#endif//CONFIG_AUDIO_CODEC_PROCFS


	return ret;
}

static void __exit imapx200_exit(void)
{
	platform_device_unregister(imapx200_snd_device);
}

module_init(imapx200_init);
module_exit(imapx200_exit);

/* Module information */
MODULE_AUTHOR("James xu, James Xu@infotmic.com.cn");
MODULE_DESCRIPTION("ALSA SoC WM8987 IMAPX200");
MODULE_LICENSE("GPL");
