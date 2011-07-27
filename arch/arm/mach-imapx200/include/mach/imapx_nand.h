/*****************************************************************************
** regs-nand.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description:	iMAP NAND Flash platform driver header file. 
**				Layout of the NAND Flash controller registers on iMAPx200.
**
** Author:
**     warits    <warits@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  02/10/2009  Initialized by warits.
*****************************************************************************/


#ifndef __ARM_IMAPX_REGS_NAND
#define __ARM_IMAPX_REGS_NAND

/* Register Layout */

#define iMAPX200_NAND_BASE

#define iMAPX200_NFCONF						0x00
#define iMAPX200_NFCONT						0x04
#define iMAPX200_NFCMD						0x08
#define iMAPX200_NFADDR						0x0c
#define iMAPX200_NFDATA						0x10
#define iMAPX200_NFMECCD0					0x14
#define iMAPX200_NFMECCD1					0x18
#define iMAPX200_NFMECCD2					0x1c
#define iMAPX200_NFSBLK						0x20
#define iMAPX200_NFEBLK						0x24
#define iMAPX200_NFSTAT						0x28
#define iMAPX200_NFESTAT0					0x2c
#define iMAPX200_NFESTAT1					0x30
#define iMAPX200_NFMECC0					0x34
#define iMAPX200_NFMECC1					0x38
#define iMAPX200_NFMECC2					0x3c
#define iMAPX200_NFESTAT2					0x40
#define iMAPX200_NFSECCD					0x44
#define iMAPX200_NFSECC						0x48
#define iMAPX200_NFDESPCFG					0x4c
#define iMAPX200_NFDESPADDR					0x50
#define iMAPX200_NFDESPCNT					0x54
#define iMAPX200_NFDECERRFLAG				0x58		/* if 1, spare ecc is error, else main ess is error */
#define iMAPX200_NFPAGECNT					0x5c		/* [0:3]: page count, [4:15]: unit count, means '6+1' counter */
#define iMAPX200_NFDMAADDR_A				0x80
#define iMAPX200_NFDMAC_A					0x84
#define iMAPX200_NFDMAADDR_B				0x88
#define iMAPX200_NFDMAC_B					0x8c
#define iMAPX200_NFDMAADDR_C				0x90
#define iMAPX200_NFDMAC_C					0x94
#define iMAPX200_NFDMAADDR_D				0x98
#define iMAPX200_NFDMAC_D					0x9c


/* Defination of NFCONF */

#define iMAPX200_NFCONF_TWRPH0_(x)			((x)<<8)
#define iMAPX200_NFCONF_TWRPH1_(x)			((x)<<4)
#define iMAPX200_NFCONF_TACLS_(x)			((x)<<12)
#define iMAPX200_NFCONF_TMSK				(0x07)
#define iMAPX200_NFCONF_ECCTYPE4			(1<<24)		/* 1 for MLC(4-bit), 0 for SLC(1-bit). */
#define iMAPX200_NFCONF_BusWidth16			(1<<0)		/* 1 for 16bit, 0 for 8bit. */

/* Defination of NFCONT */

#define iMAPX200_NFCONT_MODE				(1<<0)
#define iMAPX200_NFCONT_Reg_nCE0			(1<<1)
#define iMAPX200_NFCONT_Reg_nCE1			(1<<2)
#define iMAPX200_NFCONT_InitSECC			(1<<4)
#define iMAPX200_NFCONT_InitMECC			(1<<5)
#define iMAPX200_NFCONT_SpareECCLock		(1<<6)
#define iMAPX200_NFCONT_MainECCLock			(1<<7)
#define iMAPX200_NFCONT_RnB_TransMode		(1<<8)
#define iMAPX200_NFCONT_EnRnBINT			(1<<9)
#define iMAPX200_NFCONT_EnIllegalAccINT		(1<<10)
#define iMAPX200_NFCONT_EnECCDecINT			(1<<12)
#define iMAPX200_NFCONT_EnECCEncINT			(1<<13)
#define iMAPX200_NFCONT_DMACompleteINT		(1<<14)
#define iMAPX200_NFCONT_DMABlockEndINT		(1<<15)
#define iMAPX200_NFCONT_INTMSK				(0x3f << 9)
#define iMAPX200_NFCONT_SoftLock			(1<<16)
#define iMAPX200_NFCONT_LockTight			(1<<17)
#define iMAPX200_NFCONT_ECCDirectionEnc		(1<<18)

/* Defination of NFSTAT */

#define iMAPX200_NFSTAT_RnB_ro				(1<<0)		/* 0 busy, 1 ready */
#define iMAPX200_NFSTAT_nFCE_ro				(1<<2)
#define iMAPX200_NFSTAT_nCE_ro				(1<<3)
#define iMAPX200_NFSTAT_RnB_TransDetect		(1<<4)		/* write 1 to clear */
#define iMAPX200_NFSTAT_IllegalAccess		(1<<5)		/* write 1 to clear(to be confirmed) */
#define iMAPX200_NFSTAT_ECCDecDone			(1<<6)		/* write 1 to clear */
#define iMAPX200_NFSTAT_ECCEncDone			(1<<7)		/* write 1 to clear */
#define iMAPX200_NFSTAT_DMA_COMPLETE		(1<<8)
#define iMAPX200_NFSTAT_DMA_BLOCKEND		(1<<9)
#define iMAPX200_NFSTAT_ProgramErr			(1<<10)		/* Err=1, OK=0, write 1 to clear */
#define iMAPX200_NFSTAT_DespEnd				(1<<11)		/* OK=1 */
#define iMAPX200_NFSTAT_DespECCErr			(1<<12)		/* if intr en in description, Err=1, OK=0, write 1 to clear */
#define iMAPX200_NFSTAT_ECCErrResult		(1<<13)

/* Defination of SLC NFESTAT0 */

#define iMAPX200_NFESTAT0_SLC_MErrType_ro_		(0)
#define iMAPX200_NFESTAT0_SLC_SErrType_ro_		(2)
#define iMAPX200_NFESTAT0_SLC_Err_MSK			(0x03)
#define iMAPX200_NFESTAT0_SLC_MByte_Loc_ro_		(7)
#define iMAPX200_NFESTAT0_SLC_SByte_Loc_ro_		(21)
#define iMAPX200_NFESTAT0_SLC_MByte_Loc_MSK		(0x7ff)
#define iMAPX200_NFESTAT0_SLC_SByte_Loc_MSK		(0xf)
#define iMAPX200_NFESTAT0_SLC_MBit_Loc_ro_		(4)
#define iMAPX200_NFESTAT0_SLC_SBit_Loc_ro_		(18)
#define iMAPX200_NFESTAT0_SLC_Bit_MSK			(0x7)

/* Defination of MLC NFESTAT0 & NFESTAT1 & NFESTAT2*/

#define iMAPX200_NFESTAT0_Busy_ro				(1<<31)
#define iMAPX200_NFESTAT0_Ready_ro				(1<<30)
#define iMAPX200_NFESTAT0_MLC_MErrType_ro_		(27)
#define iMAPX200_NFESTAT0_MLC_MErrType_MSK		(0x7)
#define	iMAPX200_NFESTAT0_MLC_Loc1_ro_			(0)
#define	iMAPX200_NFESTAT1_MLC_Loc2_ro_			(0)
#define	iMAPX200_NFESTAT1_MLC_Loc3_ro_			(9)
#define	iMAPX200_NFESTAT1_MLC_Loc4_ro_			(18)
#define iMAPX200_NFESTAT0_MLC_PT1_ro_			(16)
#define iMAPX200_NFESTAT2_MLC_PT2_ro_			(0)
#define iMAPX200_NFESTAT2_MLC_PT3_ro_			(9)
#define iMAPX200_NFESTAT2_MLC_PT4_ro_			(18)
#define iMAPX200_NFESTATX_MLC_PT_MSK			(0x1ff)
#define iMAPX200_NFESTATX_MLC_Loc_MSK			(0x1ff)

/* Defination of DMAC */
#define iMAPX200_NFDMAC_DMADIROut						(1<<25)
#define iMAPX200_NFDMAC_DMAAUTO							(1<<26)
#define iMAPX200_NFDMAC_DMAALT							(1<<27)
#define iMAPX200_NFDMAC_DMARPT							(1<<28)
#define iMAPX200_NFDMAC_DMAWIND							(1<<29)
#define iMAPX200_NFDMAC_DMARST							(1<<30)
#define iMAPX200_NFDMAC_DMAEN							(1<<31)

#endif
