/*
 * s3c24xx-ac97.c  --  ALSA Soc Audio Layer
 *
 * (c) 2007 Wolfson Microelectronics PLC.
 * Author: Graeme Gregory
 *         graeme.gregory@wolfsonmicro.com or linux@wolfsonmicro.com
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 *  Revision history
 *    10th Nov 2006   Initial version.
 */

#ifndef S3C6400AC97_H_
#define S3C6400AC97_H_

//#define AC_CMD_ADDR(x) (x << 16)
//#define AC_CMD_DATA(x) (x & 0xffff)

extern struct snd_soc_cpu_dai s3c6400_ac97_dai[];

#if 0
#define AC97_RECORD_GAIN         0x001c
#define AC97_POWER_CONTROL      0x0026
#define	AC97_EXTENDED_MODEM_ID    0x003C
#define AC97_EXTEND_MODEM_STAT    0x003E
#define AC97_PHONE_VOL           0x000c      // TAD Input (mono)
#define AC97_PCBEEP_VOL         0x000a      // none
#endif
#endif /*S3C6400AC97_H_*/
