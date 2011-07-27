#ifndef __IMAPX_AC97__
#define __IMAPX_AC97__

#define rAC_GLBCTRL	    (0x00 )
#define rAC_GLBSTAT	    (0x04 )
#define rAC_CODEC_CMD	    (0x08 )
#define rAC_CODEC_STAT	    (0x0C )
#define rAC_OFS             (0x10 )
#define rAC_IFS             (0x14 )
#define rAC_R_FRONT         (0x20 )
#define rAC_LFE             (0x30 )
#define rAC_RESETTIME       (0x34 )
#define DATA_16		1
#define DATA_32		2
#define DATA_LEN	DATA_16

#define TEST_OIFSLEN		0
#define TEST_SAMPLESIZE		0
#define TEST_POLLMODE		0
#define TEST_INTMODE		0
#define TEST_DMAMODE		0
#define TEST_PLAYSONG		1
#define TEST_WARMSRART		0

//AC97 CONTROLLER
//AC_GLBCTRL
#define AC_GLBCTRL_COLDRESET          (0)
#define AC_GLBCTRL_WARMRESET        (1)
#define AC_GLBCTRL_LINKON                (2)
#define AC_GLBCTRL_TRANSFEN           (3)
#define AC_GLBCTRL_SAMPLESIZE        (4)    //00: 16 bit    01: 18 bit   10: 20 bit
#define AC_GLBCTRL_SAMPLERATE        (6)   //0: fixed sample rate(48Khz)  1: variable sample rate(on demand)
#define AC_GLBCTRL_PCMINMODE         (10)  //PCM in channel transfer mode
#define AC_GLBCTRL_PCMOUTMODE      (12)  //PCM out channel transfer mode
#define AC_GLBCTRL_PCMINHFIFOEN   (17)  //PCM in channel threshold interrupt enable
#define AC_GLBCTRL_PCMOUTHFIFOEN (18)  //PCM out channel threshold interrupt enable
#define AC_GLBCTRL_PCMINOFIFOEN   (20)  //PCM in channel overrun interrupt Enable
#define AC_GLBCTRL_PCMOUTOFIFOEN (21)  //PCM out channel underrun interrupt enable
#define AC_GLBCTRL_CODECREADYEN   (22)  //Codec ready interrupt enable

//AC_GLBSTAT
#define AC_GLBSTAT_CONTRLSTATE         (0)
#define AC_GLBSTAT_PCMRIGHTINHALF   (8)
#define AC_GLBSTAT_PCMRIGHTOUTHALF (9)
#define AC_GLBSTAT_PCMRIGHTINOVER   (10)
#define AC_GLBSTAT_PCMRIGHTOUTUNDER  (11)
#define AC_GLBSTAT_PCMLEFTINHALF      (17)
#define AC_GLBSTAT_PCMLEFTOUTHALF   (18)
#define AC_GLBSTAT_PCMLEFTINOVER      (20)
#define AC_GLBSTAT_PCMLEFTOUTUNDER  (21)
#define AC_GLBSTAT_CODECREADY            (22)

//AC_CODEC_CMD
#define AC_CODEC_CMD_DATA       (0)
#define AC_CODEC_CMD_ADDR       (16)
#define AC_CODEC_CMD_READEN   (23)

//AC_CODEC_STAT
#define AC_CODEC_STAT_DATA     (0)
#define AC_CODEC_STAT_ADDR     (16)

//AC_OFS
#define AC_OFS_PCMLEFT             (0)
#define AC_OFS_PCMRIGHT           (5)

//AC_IFS
#define AC_IFS_PCMLEFT              (0)
#define AC_IFS_PCMRIGHT            (5)

//AC_TIMER
#define AC_TIMER_NANOSECOND   (0) 
#define AC_TIMER_SUSPENDED      (8)
/*
typedef enum{COLDSTART=0,WARMSTART} RESTARTTYPE;
typedef enum{FIX=0,VSR} SAMPLETYPE;
typedef enum{HALF_INTMODE=0,FULL_INTMODE} INTMODE;
typedef enum{SAMPLE8000=0,SAMPLE11025,SAMPLE16000,SAMPLE22050,SAMPLE44100,SAMPLE48000} SAMPLERATE;
typedef enum{OFF_MODE=0,POLL_MODE,INT_MODE,DMA_MODE} OPERATORMODE;
*/
#define SETDAC_MODE    1
#define SETADC_MODE    2
#define LEFT_VOLUME    1
#define RIGHT_VOLUME  2


#endif
