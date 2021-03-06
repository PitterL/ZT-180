#ifndef __IMAPX_RTC_H__
#define __IMAPX_RTC_H__

/* MEMORY MAP */
#define IMAPX200_BASE 0x20dc0000

#define IMAPX200_RTCCON			0x40
#define IMAPX200_TICNT			0x44
#define IMAPX200_RTCALM			0x50
#define IMAPX200_ALMSEC			0x54
#define IMAPX200_ALMMIN			0x58
#define IMAPX200_ALMHOUR		0x5c
#define IMAPX200_ALMDATE		0x60
#define IMAPX200_ALMMON			0x64
#define IMAPX200_ALMYEAR		0x68
#define IMAPX200_RTCRST			0x6c
#define IMAPX200_BCDSEC			0x70
#define IMAPX200_BCDMIN			0x74
#define IMAPX200_BCDHOUR		0x78
#define IMAPX200_BCDDATE		0x7c
#define IMAPX200_BCDDAY			0x80
#define IMAPX200_BCDMON			0x84
#define IMAPX200_BCDYEAR		0x88
#define IMAPX200_ALMDAY			0x8c
#define IMAPX200_RTCSET			0x90

/* INDIVIDUAL REGISTER FUNCTIONS */
/*
 *
 * ---->RTCCON
 */
#define IMAPX200_RTCCON_RTCEN		(1<<0)
#define IMAPX200_RTCCON_CLKSEL		(1<<1)
#define IMAPX200_RTCCON_Reserved1	(1<<2)
#define IMAPX200_RTCCON_Reserved2	(1<<3)
/*
 *
 * ---->TICNT
 */
#define IMAPX200_TICNT_TICEN	(1<<7)
#define IMAPX200_MAX_CNT 128
/*
 *
 * ---->RTCALM
 */
#define IMAPX200_RTCALM_DAY		(1<<7)
#define IMAPX200_RTCALM_ALMEN	(1<<6)
#define IMAPX200_RTCALM_YEAR	(1<<5)
#define IMAPX200_RTCALM_MON		(1<<4)
#define IMAPX200_RTCALM_DATE	(1<<3)
#define IMAPX200_RTCALM_HOUR	(1<<2)
#define IMAPX200_RTCALM_MIN		(1<<1)
#define IMAPX200_RTCALM_SEC		(1<<0)

#define IMAPX200_RTCALM_ALL		(0xff)
/*
 *
 * ---->RTCSET
 */
#define IMAPX200_RTCSET_BCD		(1<<0)
#define IMAPX200_RTCSET_ALM		(1<<1)

#endif /* __IMAPX_RTC_H__ */
