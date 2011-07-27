#ifndef _AC97_REG_H_
#define  _AC97_REG_H_

#include "def.h"
#include "imap_addr.h"
#include "imap_cfg.h"

//ac97 model configure
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

typedef enum{COLDSTART=0,WARMSTART} RESTARTTYPE;
typedef enum{FIX=0,VSR} SAMPLETYPE;
typedef enum{HALF_INTMODE=0,FULL_INTMODE} INTMODE;
typedef enum{SAMPLE8000=0,SAMPLE11025,SAMPLE16000,SAMPLE22050,SAMPLE44100,SAMPLE48000} SAMPLERATE;
typedef enum{OFF_MODE=0,POLL_MODE,INT_MODE,DMA_MODE} OPERATORMODE;

#define SETDAC_MODE    1
#define SETADC_MODE    2
#define LEFT_VOLUME    1
#define RIGHT_VOLUME  2

//LM4550 CODEC COMMAND SERIAL NUM
#define LM4550_RESET                      (0x0)
#define LM4550_MASTERVOL             (0x2)
#define LM4550_HEADPHONEVOL      (0x4)
#define LM4550_MONOVOL                (0x6)
#define LM4550_PCBEEPVOL              (0xA)
#define LM4550_MICVOL                    (0xC)
#define LM4550_LINEINVOL              (0x10)
#define LM4550_CDVOL                     (0x12)
#define LM4550_VIDEOVOL               (0x14)
#define LM4550_VUXVOL                   (0x16)
#define LM4550_PCMOUTVOL            (0x18)
#define LM4550_RECORDSEL             (0x1A)
#define LM4550_RECORDGAIN          (0x1C)
#define LM4550_GENERALPURPOSE  (0x20)
#define LM4550_3DCONTROL            (0x22)
#define LM4550_POWERDOWNCTRL  (0x26)
#define LM4550_EXAUDIOID            (0x28)
#define LM4550_EXAUDIOCTRL        (0x2A)
#define LM4550_PCMDACRATE         (0x2C)
#define LM4550_PCMADCRATE         (0x32)
#define LM4550_CHAININCTRL         (0x74)
#define LM4550_VENDORID1            (0x7C)
#define LM4550_VENDORID2            (0x7E)

typedef struct
{
	BYTE* pBuffer;
	DWORD dwToLen;
	DWORD dwCbLen;
}DATAHEAD;

typedef struct
{
	OPERATORMODE uPlayMode;
	OPERATORMODE uRecordMode;
	U8 uPlayIntType;  //play intrrupt type:half or full
	U8 uRecordIntType; //record intrrupt type:half or full
	U8 VolNum;
	SAMPLETYPE uSampleType;
	SAMPLERATE dwISampleRate;
	SAMPLERATE dwOSampleRate;
	U32  dwSampleSize;
	U32 dwPlayVol;
	U32 dwRecordVol;
	U32 dwCodecDelayTime;
	DATAHEAD DataHead[2];
	U32 dwPlayCount;
	U32 dwRecordCount;
}AC97REGCTRL,*PAC97REGCTRL;






void   InitAc97Codec();
U16   Ac97CodecCmd(U8 CMD_Read, U8 CMD_Offset, U16 CMD_Data);
void   Ac97ControllerState(void);

DWORD	Ac97PlayWav(BYTE* pBuff,DWORD dwLen,BYTE fBlk);
DWORD	Ac97RecordWav(BYTE* pBuff,DWORD dwLen,BYTE fBlk);
void Ac97DeinitCodec();
void SetPlayOperatorMode(OPERATORMODE value,BYTE IsOperator);
void SetRecordOperatorMode(OPERATORMODE value,BYTE IsOperator);


#endif


