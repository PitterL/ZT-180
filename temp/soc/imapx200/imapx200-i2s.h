#ifndef IMAPX200I2S_H_
#define IMAPX200I2S_H_

/* clock sources */
//#define IMAPX200_CLKSRC_PCLK 0
//#define IMAPX200_CLKSRC_MPLL 1

/* Clock dividers */
//#define IMAPX200_DIV_MCLK	0
//#define IMAPX200_DIV_BCLK	1
//#define IMAPX200_DIV_PRESCALER	2

/* prescaler */
//u32 imapx200_i2s_get_clockrate(void);

extern struct snd_soc_dai imapx200_i2s_dai[];
#endif /*IMAPX200I2S_H_*/
