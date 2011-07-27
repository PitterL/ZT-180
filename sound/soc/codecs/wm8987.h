/*
 * Copyright 2005 Openedhand Ltd.
 *
 * Author: Richard Purdie <richard@openedhand.com>
 *
 * Based on WM8987.h
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 */

#ifndef _WM8987_H
#define _WM8987_H

#define CONFIG_HHTECH_MINIPMP	1

/* WM8987 register space */

#define WM8987_LINVOL    0x00
#define WM8987_RINVOL    0x01
#define WM8987_LOUT1V    0x02
#define WM8987_ROUT1V    0x03
#define WM8987_ADCDAC    0x05
#define WM8987_IFACE     0x07
#define WM8987_SRATE     0x08
#define WM8987_LDAC      0x0a
#define WM8987_RDAC      0x0b
#define WM8987_BASS      0x0c
#define WM8987_TREBLE    0x0d
#define WM8987_RESET     0x0f
#define WM8987_3D        0x10
#define WM8987_ALC1      0x11
#define WM8987_ALC2      0x12
#define WM8987_ALC3      0x13
#define WM8987_NGATE     0x14
#define WM8987_LADC      0x15
#define WM8987_RADC      0x16
#define WM8987_ADCTL1    0x17
#define WM8987_ADCTL2    0x18
#define WM8987_PWR1      0x19
#define WM8987_PWR2      0x1a
#define WM8987_ADCTL3    0x1b
#define WM8987_ADCIN     0x1f
#define WM8987_LADCIN    0x20
#define WM8987_RADCIN    0x21
#define WM8987_LOUTM1    0x22
#define WM8987_LOUTM2    0x23
#define WM8987_ROUTM1    0x24
#define WM8987_ROUTM2    0x25
#define WM8987_MOUTM1    0x26
#define WM8987_MOUTM2    0x27
#define WM8987_LOUT2V    0x28
#define WM8987_ROUT2V    0x29
#define WM8987_MOUTV     0x2a

#define WM8987_CACHE_REGNUM 0x2a

#define WM8987_SYSCLK	0

struct wm8987_setup_data {
	int i2c_bus;	
	unsigned short i2c_address;
};

extern struct snd_soc_dai wm8987_dai;
extern struct snd_soc_codec_device soc_codec_dev_wm8987;

#if 1 //lzcx
#define WM8987_PLL1			0
#define WM8987_PLL2			1

/* clock inputs */
#define WM8987_MCLK		0
#define WM8987_PCMCLK		1

/* clock divider id's */
#define WM8987_PCMDIV		0
#define WM8987_BCLKDIV		1
#define WM8987_VXCLKDIV		2

/* PCM clock dividers */
#define WM8987_PCM_DIV_1	(0 << 6)
#define WM8987_PCM_DIV_3	(2 << 6)
#define WM8987_PCM_DIV_5_5	(3 << 6)
#define WM8987_PCM_DIV_2	(4 << 6)
#define WM8987_PCM_DIV_4	(5 << 6)
#define WM8987_PCM_DIV_6	(6 << 6)
#define WM8987_PCM_DIV_8	(7 << 6)

/* BCLK clock dividers */
#define WM8987_BCLK_DIV_1	(0 << 7)
#define WM8987_BCLK_DIV_2	(1 << 7)
#define WM8987_BCLK_DIV_4	(2 << 7)
#define WM8987_BCLK_DIV_8	(3 << 7)

/* VXCLK clock dividers */
#define WM8987_VXCLK_DIV_1	(0 << 6)
#define WM8987_VXCLK_DIV_2	(1 << 6)
#define WM8987_VXCLK_DIV_4	(2 << 6)
#define WM8987_VXCLK_DIV_8	(3 << 6)
#define WM8987_VXCLK_DIV_16	(4 << 6)

#define WM8987_DAI_HIFI		0
#define WM8987_DAI_VOICE		1

#define WM8987_1536FS 1536
#define WM8987_1024FS	1024
#define WM8987_768FS	768
#define WM8987_512FS	512
#define WM8987_384FS	384
#define WM8987_256FS	256
#define WM8987_128FS	128
#endif

#endif
