#ifndef IMAPX200AC97_H_
#define IMAPX200AC97_H_

#define AC_CMD_ADDR(x) (x << 16)
#define AC_CMD_DATA(x) (x & 0xffff)

extern struct snd_soc_dai imapx200_ac97_dai[];
#undef FIXED_RATE
#if 0
#define AC97_RECORD_GAIN         0x001c
#define AC97_POWER_CONTROL      0x0026
#define	AC97_EXTENDED_MODEM_ID    0x003C
#define AC97_EXTEND_MODEM_STAT    0x003E
#define AC97_PHONE_VOL           0x000c      // TAD Input (mono)
#define AC97_PCBEEP_VOL         0x000a      // none
#endif
#endif /*S3C6400AC97_H_*/
