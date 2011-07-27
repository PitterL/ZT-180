/***************************************************************************** 
** XXX driver/mtd/nand_imapx200.c XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: iMAP NAND Flash platform driver.
**				TODO: Science spare ECC is used, read_oob, wirte_oob
**					should be applied to ensure the validity of OOB.
**
** Author:
**     Warits   <warits.wang@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 09/16/2009 XXX	Initialized by warits
*****************************************************************************/

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/sched.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <linux/highmem.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>

#include "bch_ecc.h"

#define __nand_msg(args...) printk(KERN_ERR "iMAPx200 NAND: " args)
//#define __nand_dbg(args...) printk(KERN_ERR "iMAPx200 NAND: " args)
#define __nand_dbg(args...)
//#define __nand_msg(args...)


#undef FPGA_TEST

#undef  pr_fmt
#define pr_fmt(fmt) "NAND DBG: " fmt

#if defined(CONFIG_MTD_NAND_IMAPX200_SDMA)
#define NAND_USE_IRQ
#else
#undef NAND_USE_IRQ
#endif


static struct mtd_partition imap_nand_parts[] = {
#if defined(CONFIG_MTD_NAND_IMAPX200_MUTI_OS)
	{
		.name       = "system",
		.offset     = 0x270  * SZ_1M,
		.size       = 0x100 * SZ_1M,
	}, {
		.name       = "userdata",
		.offset     = 0x370*SZ_1M,
		.size       = 0x220 * SZ_1M,
	}, {
		.name       = "cache",
		.offset     = 0x590*SZ_1M,
		.size       = 0x60 * SZ_1M,	
	}, {			
		.name       = "Local-disk",
		.offset     = 0x5F0*SZ_1M,
		.size       = MTDPART_SIZ_FULL,
	}
#else
   {
       .name       = "system",
       .offset     = 0x90  * SZ_1M,
       .size       = 0x100 * SZ_1M,
	}, { /* factory reset : 288M-350M */
       .name       = "userdata",
       .offset     = 0x190*SZ_1M,
       .size       = 0x400 * SZ_1M,
   }, { 
       .name       = "cache",
       .offset     = 0x590*SZ_1M,
       .size       = 0x60 * SZ_1M, 
   }, { 
       .name       = "Local-disk",
       .offset     = 0x5F0*SZ_1M,
       .size       = MTDPART_SIZ_FULL,
   }
#endif
};

/* The structions used by NAND driver */
struct imap_nand_device {

	/* basic */
	struct	mtd_info	*mtd;
	struct	nand_chip	*chip;

	/* resource */
	struct	device		*dev;
    struct  resource    *area;
    struct  clk         *clk;
    void    __iomem     *regs;
	int		irqno;
	wait_queue_head_t	wq;

	/* device flags */
	int		cur_ecc_mode;
};


static struct imap_nand_device imap_nand;


#if defined(CONFIG_MTD_NAND_IMAPX200_SDMA)
struct imap_nand_dma {
	uint32_t dma_addr;
	uint32_t len;
	struct page *page;
};

struct imap_nand_dma imap_nand_dma_list[4];
#endif


/*
 * We do not use ECC layout in this driver, but directly write
 * ECC codes to the end of OOB
 */

/* Tools Functions */
#if 0
/* Backup function */
static struct imap_nand_device *imap_getdev(struct mtd_info *mtd)
{
	return container_of(&mtd, struct imap_nand_device, mtd);
}
#endif


/*!
 ***********************************************************************
 * -Function:
 *    imap_bis(uint32_t offset, uint32_t val)
 *    imap_bic(uint32_t offset, uint32_t val)
 *
 * -Description:
 *    imap_bis set coreponding bits in offset according to the value
 *    of val
 *    imap_bic clear coresponding bits in offset according to the value
 *    of val
 *
 * -Input Param
 *    offset     register address offset to nand_base
 *    val        bits to set 1 or clear 0
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_bis(uint32_t offset, uint32_t val)
{
	void __iomem *reg = imap_nand.regs + offset;
	writel(readl(reg) | val, reg);
}

#if defined(CONFIG_MTD_NAND_IMAPX200_OOBECC)
/* Currently not used */
static void imap_bic(uint32_t offset, uint32_t val)
{
	void __iomem *reg = imap_nand.regs + offset;
	writel(readl(reg) & ~val, reg);
}
#endif

void flash_parse_id5(struct nand_chip *chip,char *id5)
{
    unsigned long val;

    val=id5[2];
    chip->cellinfo=val;

    val=id5[3];
	chip->writesize= 1024 << (val & 0x3);
	val>>= 2;
	/* Calc oobsize */
	chip->oobsize = (8 << (val & 0x01)) * (chip->writesize >> 9);
	val >>= 2;
	/* Calc blocksize. Blocksize is multiples of 64KiB */
	chip->erasesize = (64 <<10) << (val & 0x03);

	//val >>= 2;
	/* Get buswidth information */
	//chip->busw = (extid & 0x01) ? NAND_BUSWIDTH_16 : 0;

	chip->ecclevel=0;
}

void flash_parse_id6(struct nand_chip *chip,char *id6)
{
    unsigned long val;
    unsigned long redundant,blocksize,ecclevel;

    val=id6[2];
    chip->cellinfo=val;

    val=id6[3];
    chip->writesize = 2048 << (val & 0x3);
    val>>= 2;
    /* Calc oobsize */
    redundant=(val & 0x3)|((val>>2)&0x4);
    switch(redundant){
        case 1:
            chip->oobsize=128;
        break;
        case 2:
            chip->oobsize=218;
        break;
        case 3:
            chip->oobsize=256;
        break;
        case 4:
            chip->oobsize=368;
        break;
        case 5:
            chip->oobsize=400;
        break;
        case 6:
            chip->oobsize=436;
        break;
        default:
            chip->oobsize=0;
    }
    val >>= 2;
    /* Calc blocksize. Blocksize is multiples of 64KiB */
    blocksize=(val & 0x3)|((val>>1)&0x4);
    switch(blocksize){
        case 0:
            chip->erasesize=128<<10;
        break;  
        case 1:
            chip->erasesize=256<<10;
        break;
        case 2:
            chip->erasesize=512<<10;
        break;        
        case 3:
            chip->erasesize=1024<<10;
        break;
        case 4:
            chip->erasesize=1536<<10;
        break;
        default:
            chip->erasesize=0;
    }
    val = id6[4];
    ecclevel=(val>>4)&0x7;
    switch(ecclevel){
        case 0:
            chip->ecclevel=1;
        break;
        case 1:
            chip->ecclevel=2;
        break;
        case 2:
            chip->ecclevel=4;
        break;
        case 3:
            chip->ecclevel=8;
        break;        
        case 4:
            chip->ecclevel=16;
        break;
        case 5:
            chip->ecclevel=24;
        break;
        default:
            chip->ecclevel=-1;
    }
    DEBUG (MTD_DEBUG_LEVEL0,"ecc level %d\n", chip->ecclevel);
}

int flash_parse_id(struct mtd_info *mtd,char *id,int len)
{
    struct nand_chip *chip = mtd->priv;
    if(len==5){
        flash_parse_id5(chip,id);
    }else{
        flash_parse_id6(chip,id);
    }

    if(chip->erasesize&&
        chip->writesize&&
            chip->oobsize){
        return 0;
    }
    return 1;
}


/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_mlc_bitfix(uint8_t * dat, uint16_t pt, uint16_t loc)
 *    imap_nand_slc_bitfix(uint8_t *dat, uint16_t byteid, ushort bitid)
 *
 * -Description:
 *    SLC/MLC data correct function, using pt, loc, byteid , bitid
 *    value read from NFESTATX
 *
 * -Input Param
 *    dat   data to be corrected.
 *                
 * -Output Param
 *    dat   data that has been corrected.
 *
 * -Return
 *    none
 ***********************************************************************
 */
#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
static int imap_nand_mlc_bitfix(uint8_t * dat, uint16_t loc, uint16_t pt)
{
	int index = 0;
    int errsum = 0;
	if( (loc > 0x1c7) || (( loc == 0x1c7 )&& ( pt >= 0x2 )) ){
		//__nand_msg("Open one eye on a fly.\n");
		return 0;
	}
	
	while(1){
		if (index > 8) break;
		if (pt & 0x01){
		  dat[(index + loc * 9) / 8] ^= (1 << ((index + loc * 9) % 8));
          errsum++;
		}
		pt >>= 1;
		index++;
	}
    if(errsum > 1)
        __nand_msg ("errsum=%d\r\n",
                  errsum);
	return 0;
}
#endif

#if defined(CONFIG_MTD_NAND_IMAPX200_SLC) || defined(CONFIG_MTD_NAND_IMAPX200_OOBECC)
static int imap_nand_slc_bitfix(uint8_t *dat, uint16_t byteid, ushort bitid)
{
	__nand_msg("1bit@%d dancing successful.\n", byteid);
	dat[byteid] ^= (1 << bitid);
	return 0;
}
#endif

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_freepage(uint8_t *oob)
 *
 * -Description:
 *    iMAPx200 treat free page as ECC error when using 1 bit ECC,
 *    so a function is needed to check free page and get rid of ECC
 *    warning.
 *
 *    Note: the 4 bytes before seccloc is used for free page mark.
 *    as iMAPx have the feature that a free page is reported as 
 *    uncorrectalbe error in SLC, and 1 bit error in MLC.
 *    whenever a page is wrote, this 4 bytes is set to zero.
 *    when reading a page, if all the 4 bytes is 0xff, then this
 *    is a free page and driver will not report any error.
 *
 * -Input Param
 *    oob   oob buffer
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static int is_freepage(u_char *oob)
{
    if(*oob==(u_char)-1){
        return 1;
    }
	/* Not all 0xff values ,this is not a free page */
	return 0;
}
static void dirty_mark(u_char *oob)
{
    *oob=(u_char)0;
}


/* Board specific functions */
/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_select_chip(struct mtd_info *mtd, int chip)
 *
 * -Description:
 *    NAND chip selection, iMAPx200 NAND controller support 2 chips,
 *    if 2 chips exists the same time, this function is usful to select
 *    one of them to operate.
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   chip number to select, deselect all chips if -1.
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_nand_select_chip(struct mtd_info *mtd, int chip)
{
	void __iomem *regs = imap_nand.regs;

	uint32_t ctrl = readl(regs + iMAPX200_NFCONT);

	if ((chip * chip) > 1) /* We only support -1, 0, 1 three options */
	  return;

	ctrl |= 6;
	if (chip != -1)
	  ctrl &= ~(1 << (chip + 1));

	writel(ctrl, regs + iMAPX200_NFCONT);
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
 *
 * -Description:
 *    Hardware specific function to control ALE/CLE/nCE
 *    also used to write command and address.
 *
 * -Input Param
 *    mtd    mtd device
 *    cmd    Command or address to send, NAND_CMD_NONE is passed if no cmd.
 *    ctrl   NAND control options.
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_nand_cmd_ctrl(struct mtd_info *mtd, int cmd, unsigned int ctrl)
{
	void __iomem *regs = imap_nand.regs;
	struct nand_chip *chip = mtd->priv;
	/*
	if (ctrl & NAND_CTRL_CHANGE)
	{
		if (ctrl & NAND_NCE)
		  chip->select_chip(mtd, 0);
		else
		  chip->select_chip(mtd, -1);
	}*/

	if (cmd != NAND_CMD_NONE)
	{
		//while(!chip->dev_ready(mtd));
		int timeout = 1000000;
		while(timeout--){
			if(chip->dev_ready(mtd))
				break;
			udelay(1);	
		}
		if(timeout <= 0) {
			printk(KERN_ERR "imap_nand_cmd_ctrl : timeout\r\n");	
		}
		
		if (ctrl & NAND_CLE)
		  writeb(cmd, regs + iMAPX200_NFCMD);
		else
		  writeb(cmd, regs + iMAPX200_NFADDR); 
	}
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_ready(struct mtd_info *mtd)
 *
 * -Description:
 *    Check if NAND chip is busy, according to value of RnB bit in
 *    NFSTAT
 *
 * -Input Param
 *    mtd    mtd device
 *                
 * -Return
 *    1      chip is ready
 *    0      chip is busy
 ***********************************************************************
 */
static int imap_nand_ready(struct mtd_info *mtd)
{
	void __iomem *regs = imap_nand.regs; 

	return (readl(regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_RnB_ro);
}

/* Backup function */
static void imap_nand_ecc_busy(void)
{
	void __iomem *regs = imap_nand.regs;
	int timeout = 1000000;
	/* wait dec busy indicating bit change to 0 */
//	while(readl(regs + iMAPX200_NFESTAT0) & iMAPX200_NFESTAT0_Busy_ro);
	//while(!(readl(regs + iMAPX200_NFESTAT0) & iMAPX200_NFESTAT0_Ready_ro));

	while(timeout--){
		if(readl(regs + iMAPX200_NFESTAT0) & iMAPX200_NFESTAT0_Ready_ro)
			break;
		udelay(1);	
	}
	if(timeout <= 0) {
		printk(KERN_ERR "imap_nand_ecc_busy : timeout\r\n");	
	}	
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_wait(int dir)
 *
 * -Description:
 *    wait for encoding or decoding finish when using 4bit ECC mode.
 *
 * -Input Param
 *    dir    coding direction to wait
 *                
 * -Return
 *    0
 ***********************************************************************
 */
#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
/* This function is affen used to wait for a decoding operation done. */
static int imap_nand_wait(int dir)
{
	int timeout = 1000000;
	if (dir == NAND_ECC_WRITE)
	{
#if 0
		DEFINE_WAIT(enc_wait);
		prepare_to_wait(&imap_nand.wq, &enc_wait, TASK_INTERRUPTIBLE);
		if(!(readl(imap_nand.regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_ECCEncDone))
		  schedule();
		finish_wait(&imap_nand.wq, &enc_wait);
#else
		//while(!(readl(imap_nand.regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_ECCEncDone));
		while(timeout--){
			if(readl(imap_nand.regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_ECCEncDone)
				break;
			udelay(1);	
		}
		if(timeout <= 0) {
			printk(KERN_ERR "imap_nand_wait : write timeout\r\n");	
		}		
		
		/* Clear Enc flag */
		writel(iMAPX200_NFSTAT_ECCEncDone, imap_nand.regs + iMAPX200_NFSTAT);

		/* EncDone is something unbelivable */
		udelay(1);
#endif
	}
	else
	{
#if 0
		DEFINE_WAIT(dec_wait);
		prepare_to_wait(&imap_nand.wq, &dec_wait, TASK_INTERRUPTIBLE);
		if(!(readl(imap_nand.regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_ECCDecDone))
		  schedule();
		finish_wait(&imap_nand.wq, &dec_wait);
#else
		//while(!(readl(imap_nand.regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_ECCDecDone));
		while(timeout--){
			if(readl(imap_nand.regs + iMAPX200_NFSTAT) & iMAPX200_NFSTAT_ECCDecDone)
				break;
			udelay(1);	
		}
		if(timeout <= 0) {
			printk(KERN_ERR "imap_nand_wait : read timeout\r\n");	
		}


		/* Wait until ECC module is ready */
		imap_nand_ecc_busy();
		/* Clear Dec flag */
		writel(iMAPX200_NFSTAT_ECCDecDone, imap_nand.regs + iMAPX200_NFSTAT);
#endif
	}

	return 0;
}
#endif

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
 *
 * -Description:
 *    MECC module initialization.
 *
 * -Input Param
 *    mtd    mtd device
 *    mode   ECC direction to set
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_nand_ecc_hwctl(struct mtd_info *mtd, int mode)
{
	uint32_t nfcont, nfconf;
	void __iomem * regs = (&imap_nand)->regs;

	nfconf = readl(regs + iMAPX200_NFCONF);
	/* nand_type == 0 indicates that this is a SLC Flash */
	/* We use 1bit ECC method for SLC Flash, 4bit for MLC */
#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
	  nfconf |= iMAPX200_NFCONF_ECCTYPE4;
#else
	  nfconf &= ~iMAPX200_NFCONF_ECCTYPE4;
#endif
	writel(nfconf, regs + iMAPX200_NFCONF);

	/* Init main ECC and unlcok */
	nfcont = readl(regs + iMAPX200_NFCONT);
	nfcont |= iMAPX200_NFCONT_InitMECC;
	nfcont &= ~iMAPX200_NFCONT_MainECCLock;

#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
	if (mode == NAND_ECC_WRITE)
	  nfcont |= iMAPX200_NFCONT_ECCDirectionEnc;
	else if (mode == NAND_ECC_READ)
	  nfcont &= ~iMAPX200_NFCONT_ECCDirectionEnc;
#endif

	writel(nfcont, regs + iMAPX200_NFCONT);
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_secc_hwctl(struct mtd_info *mtd, int mode)
 *
 * -Description:
 *    SECC module initialization.
 *
 * -Input Param
 *    mtd    mtd device
 *    mode   1bit ECC do not need direction setting, this arg is useless
 *                
 * -Return
 *    none
 ***********************************************************************
 */
#if defined(CONFIG_MTD_NAND_IMAPX200_OOBECC)
static void imap_nand_secc_hwctl(struct mtd_info *mtd, int mode)
{
	uint32_t nfcont;
	void __iomem * regs = (&imap_nand)->regs;

	/* iMAPx200 only support 1bit ECC for Spare area */
	imap_bic(iMAPX200_NFCONF, iMAPX200_NFCONF_ECCTYPE4);

	/* Init main ECC and unlcok */
	nfcont = readl(regs + iMAPX200_NFCONT);
	nfcont |= iMAPX200_NFCONT_InitSECC;
	nfcont &= ~iMAPX200_NFCONT_SpareECCLock;
	writel(nfcont, regs + iMAPX200_NFCONT);
}
#endif

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_ecc_calculate(struct mtd_info *mtd, const uint8_t *dat,
 *                     uint8_t *ecc_code)
 *
 * -Description:
 *    ECC calculate function:
 *    when decoding, this function does nothing.
 *    when encoding, this function copy ECC code from NFMECC0/1/2 to ecc_code
 *
 * -Input Param
 *    mtd        mtd device
 *    dat        the original software ecc use dat to calculate ECC,
 *               but useless here
 *    ecc_code   to store ECC code. 
 *                
 * -Return
 *    0 on success;
 ***********************************************************************
 */
static int imap_nand_ecc_calculate(struct mtd_info *mtd, const uint8_t *dat,
   uint8_t *ecc_code)
{
	struct nand_chip *chip = mtd->priv;
	void __iomem *regs = imap_nand.regs;

	/* We do "lock engine" and "wait ecc decoding done" in .correct but not here */
	if (chip->state != FL_WRITING && chip->state != FL_CACHEDPRG)
	  return 0;

	/* Lock the ECC engine */
	imap_bis(iMAPX200_NFCONT, iMAPX200_NFCONT_MainECCLock);

	/* FIXME: GUESSING ... */
	/* We get 4/8bytes ECC code for x8/16 SLC chip, and 9 bytes for MLC */
#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
		/* Wait encoding done */
		imap_nand_wait(NAND_ECC_WRITE);

		/*
		 * iMAP use 8 parity codes per 512 Bytes in 4 bit ECC mode, each 9 bits
		 */

		*(uint32_t *)(ecc_code)		= readl(regs + iMAPX200_NFMECC0);
		*(uint32_t *)(ecc_code + 4)	= readl(regs + iMAPX200_NFMECC1);
		*(ecc_code + 8)	= readl(regs + iMAPX200_NFMECC2) & 0xff;
#else
		/* SLC */

		*(uint32_t *)(ecc_code)		= readl(regs + iMAPX200_NFMECC0);
#if defined(CONFIG_MTD_NAND_IMAPX200_BUSW16)
		*(uint32_t *)(ecc_code + 4)	= readl(regs + iMAPX200_NFMECC1);
#endif
#endif
	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_secc_calc(struct mtd_info *mtd, const uint8_t *dat,
 *                     uint8_t *ecc_code)
 *
 * -Description:
 *    ECC calculate function:
 *
 * -Input Param
 *    mtd        mtd device
 *    dat        the original software ecc use dat to calculate ECC,
 *               but useless here
 *    ecc_code   to store ECC code. 
 *                
 * -Return
 *    0 on success;
 ***********************************************************************
 */
#if defined(CONFIG_MTD_NAND_IMAPX200_OOBECC)
static int imap_nand_secc_calc(struct mtd_info *mtd, const uint8_t *dat,
   uint8_t *ecc_code)
{
	struct nand_chip *chip = mtd->priv;
	void __iomem *regs = imap_nand.regs;

	/* We do "lock engine" and "wait ecc decoding done" in .correct but not here */
	if (chip->state != FL_WRITING && chip->state != FL_CACHEDPRG)
	  return 0;

	*(uint32_t *)(ecc_code) = readl(regs + iMAPX200_NFSECC);
	__nand_dbg("Write SECC 0x%08x\n", (*(uint32_t *)ecc_code));
	return 0;
}
#endif


/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_ecc_correct(struct mtd_info *mtd, uint8_t *dat,
 *              uint8_t *read_ecc, uint8_t *calc_ecc)
 *
 * -Description:
 *    Error correct function
 *    correct a maximum of 4bits error per 512 when using 4bit ECC mode
 *    correct a maximum of 1bit error per 2048 when using 1bit ECC mode
 *    return err corrected, if uncorrectable err occured, return -1
 *
 * -Input Param
 *    mtd         mtd device
 *    dat         data to correct
 *    read_ecc    ECC generated from dat
 *    calc_ecc    ECC read from OOB
 *                
 * -Return
 *    number of corrected bits
 *    -1 if uncorrectable error occured
 ***********************************************************************
 */
static int imap_nand_ecc_correct(struct mtd_info *mtd, uint8_t *dat,
   uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint32_t nfestat0;
#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
	uint32_t nfestat1, nfestat2;
#endif
	uint8_t err_type;
	void __iomem *regs = imap_nand.regs;

	/* Lock the ECC engine */
	imap_bis(iMAPX200_NFCONT, iMAPX200_NFCONT_MainECCLock);

#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
		/*FIXME: write ECC into nfmeccd* if needed */
#if 0 
		writel((*(uint32_t *)(calc_ecc)), NFMECCD0);
		writel((*(uint32_t *)(calc_ecc + 4)), NFMECCD1);
		writel((*(uint32_t *)(calc_ecc + 8)), NFMECCD2);
#endif

		imap_nand_wait(NAND_ECC_READ);

		/* FIXME: Do I need to clear the dec done bit AND
		 * wait the ECC busy bit & check the ECC ready bit?
		 */
		nfestat0 = readl(regs + iMAPX200_NFESTAT0);
		nfestat1 = readl(regs + iMAPX200_NFESTAT1);
		nfestat2 = readl(regs + iMAPX200_NFESTAT2);

		err_type = (nfestat0 >> iMAPX200_NFESTAT0_MLC_MErrType_ro_)
							& iMAPX200_NFESTAT0_MLC_MErrType_MSK;

		switch(err_type)
		{   
			case 4: /* 4 bits error, correctable */
				imap_nand_mlc_bitfix(dat,
				   (nfestat1 >> iMAPX200_NFESTAT1_MLC_Loc4_ro_)
							& iMAPX200_NFESTATX_MLC_Loc_MSK,
				   (nfestat2 >> iMAPX200_NFESTAT2_MLC_PT4_ro_)
							& iMAPX200_NFESTATX_MLC_PT_MSK);
			case 3: /* 3 bits error, correctable */
				imap_nand_mlc_bitfix(dat,
				   (nfestat1 >> iMAPX200_NFESTAT1_MLC_Loc3_ro_)
							& iMAPX200_NFESTATX_MLC_Loc_MSK,
				   (nfestat2 >> iMAPX200_NFESTAT2_MLC_PT3_ro_)
							& iMAPX200_NFESTATX_MLC_PT_MSK);
			case 2: /* 2 bits error, correctalbe */
				imap_nand_mlc_bitfix(dat,
				   (nfestat1 >> iMAPX200_NFESTAT1_MLC_Loc2_ro_)
							& iMAPX200_NFESTATX_MLC_Loc_MSK,
				   (nfestat2 >> iMAPX200_NFESTAT2_MLC_PT2_ro_)
							& iMAPX200_NFESTATX_MLC_PT_MSK);
			case 1: /* 1 bits error, correctable */
				imap_nand_mlc_bitfix(dat,
				   (nfestat0 >> iMAPX200_NFESTAT0_MLC_Loc1_ro_)
							& iMAPX200_NFESTATX_MLC_Loc_MSK,
				   (nfestat0 >> iMAPX200_NFESTAT0_MLC_PT1_ro_)
							& iMAPX200_NFESTATX_MLC_PT_MSK);
				__nand_dbg("mlc_bitfix: nfestat0=0x%08x, 1=0x%08x, 2=0x%08x\n",
				   nfestat0, nfestat1, nfestat2);
				if(err_type > 2)   
				__nand_msg("Dance %d successful.\n", err_type);
			case 0:
				return (int)err_type;
			default:
				__nand_msg("Uncorrectable ECC error. name : %s\n",mtd->name);
				return -1; 
		}   
#else
		/* CONFIG_SLC */
		/*FIXME: write ECC into nfmeccd* if needed */
#if 0
		writel((*(uint32_t *)(calc_ecc)), NFMECCD0);
#if defined(CONFIG_SYS_NAND_BUSW16)
		writel((*(uint32_t *)(calc_ecc + 4)), NFMECCD1);
#endif
#endif
		nfestat0 = readl(regs + iMAPX200_NFESTAT0);
		err_type = (nfestat0 >> iMAPX200_NFESTAT0_SLC_MErrType_ro_)
							& iMAPX200_NFESTAT0_SLC_Err_MSK;

		switch(err_type)
		{
			case 0: /* No error detected */
				return 0;
			case 1: /* 1 bit error detected */
				imap_nand_slc_bitfix(dat,
				   (nfestat0 >> iMAPX200_NFESTAT0_SLC_MByte_Loc_ro_)
							& iMAPX200_NFESTAT0_SLC_MByte_Loc_MSK,
				   (nfestat0 >> iMAPX200_NFESTAT0_SLC_MBit_Loc_ro_)
							& iMAPX200_NFESTAT0_SLC_Bit_MSK);
				return 1;
			case 2: /* More than one err */
			case 3: /* ECC error */
				__nand_msg("Uncorrectable ECC error.\n");
				return -1; 
		}
#endif /* CONFIG_MLC */
	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_secc_crct(struct mtd_info *mtd, uint8_t *dat,
 *              uint8_t *read_ecc, uint8_t *calc_ecc)
 *
 * -Description:
 *    Error correct function
 *    correct a maximum of 1bit error per 16 when using 1bit SECC mode
 *    return err corrected, if uncorrectable err occured, return -1
 *
 * -Input Param
 *    mtd         mtd device
 *    dat         data to correct
 *    read_ecc    ECC generated from dat
 *    calc_ecc    ECC read from OOB
 *                
 * -Return
 *    number of corrected bits
 *    -1 if uncorrectable error occured
 ***********************************************************************
 */
#if defined(CONFIG_MTD_NAND_IMAPX200_OOBECC)
static int imap_nand_secc_crct(struct mtd_info *mtd, uint8_t *dat,
   uint8_t *read_ecc, uint8_t *calc_ecc)
{
	uint32_t nfestat0;
	uint8_t err_type;
	void __iomem *regs = imap_nand.regs;

	/* Lock the ECC engine */
	imap_bis(iMAPX200_NFCONT, iMAPX200_NFCONT_SpareECCLock);

	/* Write ECC to SECCD */
	writel((*(uint32_t *)calc_ecc), regs + iMAPX200_NFSECCD);
	__nand_dbg("Read SECC 0x%08x\n", (*(uint32_t *)calc_ecc));

	nfestat0 = readl(regs + iMAPX200_NFESTAT0);
	err_type = (nfestat0 >> iMAPX200_NFESTAT0_SLC_SErrType_ro_)
					& iMAPX200_NFESTAT0_SLC_Err_MSK;

	switch(err_type)
	{
		case 0: /* No error detected */
			return 0;
		case 1: /* 1 bit error detected */
			imap_nand_slc_bitfix(dat,
			   (nfestat0 >> iMAPX200_NFESTAT0_SLC_SByte_Loc_ro_)
			   & iMAPX200_NFESTAT0_SLC_SByte_Loc_MSK,
			   (nfestat0 >> iMAPX200_NFESTAT0_SLC_SBit_Loc_ro_)
			   & iMAPX200_NFESTAT0_SLC_Bit_MSK);
			return 1;
		case 2: /* More than one err */
		case 3: /* ECC error */
			__nand_msg("Uncorrectable SECC error.\n");
			return -1; 
	}
	return 0;
}
#endif

#if defined(NAND_USE_IRQ)
static irqreturn_t imap_nand_irq(int irq, void *id)
{
	uint32_t nfstat;
	/* wake up buffer trans */
	nfstat = readl(imap_nand.regs + iMAPX200_NFSTAT);
	
	/* IRQ handler */
	if(nfstat & iMAPX200_NFSTAT_DMA_COMPLETE)
	{
		wake_up_interruptible(&imap_nand.wq);
		/* Clear STAT */
		writel(iMAPX200_NFSTAT_DMA_COMPLETE, imap_nand.regs + iMAPX200_NFSTAT);
	}
	/*
	else if(nfstat & iMAPX200_NFSTAT_ECCEncDone)
	{
		wake_up_interruptible(&imap_nand.wq);
		writel(iMAPX200_NFSTAT_ECCEncDone, imap_nand.regs + iMAPX200_NFSTAT);
		printk("imap_nand_irq : iMAPX200_NFSTAT_ECCEncDone irq = 0x%x\r\n",irq);
	}
	else if(nfstat & iMAPX200_NFSTAT_ECCDecDone)
	{
		wake_up_interruptible(&imap_nand.wq);
		writel(iMAPX200_NFSTAT_ECCDecDone, imap_nand.regs + iMAPX200_NFSTAT);
		printk("imap_nand_irq : iMAPX200_NFSTAT_ECCDecDone irq = 0x%x\r\n",irq);
		
	}
	*/
	else
	{
		//printk("imap_nand_irq : else irq = 0x%x\r\n",irq);
		/* Nothing ^_^ */
	}

	return IRQ_HANDLED;
}
#endif

#if defined(CONFIG_MTD_NAND_IMAPX200_POLL)
/* Apply a 32bit read_buf here to speed up */
static void imap_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{       
	struct nand_chip *chip = mtd->priv;

	readsl(chip->IO_ADDR_R, (uint32_t *)buf, len >> 2);
}

/* Apply a 32bit write_buf here to speed up */
static void imap_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;

	writesl(chip->IO_ADDR_W, (uint32_t *)buf, len >> 2);
}
#elif defined(CONFIG_MTD_NAND_IMAPX200_SDMA)
static int imap_nand_map_vm(uint32_t vm_addr, int len,
   enum dma_data_direction dir)
{
	if(virt_addr_valid(vm_addr) && virt_addr_valid(vm_addr + len - 1 ))
	{
		//printk("phy : vm_addr=0x%x, len = 0x%x\r\n",vm_addr,len);
		/* This is a valid kernel logical address */
		imap_nand_dma_list[0].dma_addr = dma_map_single(NULL,
		   (void *)vm_addr, len, dir);
		imap_nand_dma_list[0].len = len;
	}
	else if(is_vmalloc_addr((void *)vm_addr))
	{
		int i;
		/* This is a vmalloc address */
		/* FIXME: I can't handle longer then 12KB, and this
		   will never happen */
		BUG_ON(len > (12 << 10));
		i = 0;
		while(len)
		{
			int page_tail = PAGE_SIZE - (vm_addr & (PAGE_SIZE - 1));
			uint32_t logic_addr;

			imap_nand_dma_list[i].page = vmalloc_to_page((void *)vm_addr);
			if(len > page_tail)
			  imap_nand_dma_list[i].len = PAGE_SIZE - (vm_addr & (PAGE_SIZE - 1));
			else
			  imap_nand_dma_list[i].len = len;

			/* Map page to kernel */
			logic_addr = (uint32_t)kmap(imap_nand_dma_list[i].page);
			/* add offset */
			logic_addr += vm_addr & (PAGE_SIZE - 1);
			
			imap_nand_dma_list[i].dma_addr = dma_map_single(NULL,
			   (void *)logic_addr, imap_nand_dma_list[i].len, dir);

			vm_addr += imap_nand_dma_list[i].len;
			len		-= imap_nand_dma_list[i].len;
			++i;
		}
	}
	else
	{
		return 0;
		/*FIXME: I suppose User-Space buffer will not get here. */
		BUG();
	}

	/* Map successed */
	return 1;
}

static void imap_nand_unmap_vm(enum dma_data_direction dir)
{
	int i;
	for (i = 0; i < 4; i++)
	{
		if(imap_nand_dma_list[i].dma_addr)
		  dma_unmap_single(NULL, imap_nand_dma_list[i].dma_addr,
			 imap_nand_dma_list[i].len, dir);
		if(imap_nand_dma_list[i].page)
		  kunmap(imap_nand_dma_list[i].page);

		imap_nand_dma_list[i].dma_addr = 0;
		imap_nand_dma_list[i].len = 0;
		imap_nand_dma_list[i].page = NULL;
	}
}

/* read_buf function using simple DMA method */
static void imap_nand_read_buf(struct mtd_info *mtd, uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	void __iomem *regs = imap_nand.regs;

	if((uint32_t)buf & 0x3) {
		__nand_dbg("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)buf);
	} else if((uint32_t)buf & 0x1f) {
		__nand_dbg("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)buf);
	}

	/* using poll method if len is short */
	if(len < 512 || !imap_nand_map_vm((uint32_t)buf, len, DMA_FROM_DEVICE)) {
	  readsl(chip->IO_ADDR_R, (uint32_t *)buf, len >> 2);
	} else {

		/* Now we assume address in imap_nand_dma_list is valid */
		if(unlikely(imap_nand_dma_list[1].dma_addr))
		{
			writel(imap_nand_dma_list[1].dma_addr,
			   regs + iMAPX200_NFDMAADDR_B);
			writel(imap_nand_dma_list[1].len | iMAPX200_NFDMAC_DMADIROut,
			   regs + iMAPX200_NFDMAC_B);
			writel(imap_nand_dma_list[0].dma_addr,
			   regs + iMAPX200_NFDMAADDR_A);
			writel(imap_nand_dma_list[0].len | iMAPX200_NFDMAC_DMADIROut
			   | iMAPX200_NFDMAC_DMAALT | iMAPX200_NFDMAC_DMAAUTO
			   | iMAPX200_NFDMAC_DMAEN,
			   regs + iMAPX200_NFDMAC_A);
		} else {
			/* address is consecutive */
			writel(imap_nand_dma_list[0].dma_addr,
			   regs + iMAPX200_NFDMAADDR_A);
			writel(imap_nand_dma_list[0].len | iMAPX200_NFDMAC_DMADIROut
			   | iMAPX200_NFDMAC_DMAEN,
			   regs + iMAPX200_NFDMAC_A);
		}

		/*
		 * TODO: Wait DMA tranportation finish, this should be changed to
		 *		IRQ mode
		 */
#if defined(NAND_USE_IRQ)
		wait_event_interruptible(imap_nand.wq,
				!(readl(regs + iMAPX200_NFDMAC_A) & iMAPX200_NFDMAC_DMAEN));
#else
		while(readl(regs + iMAPX200_NFDMAC_A) & iMAPX200_NFDMAC_DMAEN);
#endif
		imap_nand_unmap_vm(DMA_FROM_DEVICE);
	}

	return ;
}

/* write_buf function using simple DMA method */
static void imap_nand_write_buf(struct mtd_info *mtd, const uint8_t *buf, int len)
{
	struct nand_chip *chip = mtd->priv;
	void __iomem *regs = imap_nand.regs;

	if((uint32_t)buf & 0x3) {
		__nand_dbg("Buffer is not 0x4 aligned! 0x%08x\n", (uint32_t)buf);
	}else if((uint32_t)buf & 0x1f){
		__nand_dbg("Buffer is not 0x20 aligned! 0x%08x\n", (uint32_t)buf);
	}
	if(len < 512 || !imap_nand_map_vm((uint32_t)buf, len, DMA_TO_DEVICE)) /* using poll method if len is short */
	  writesl(chip->IO_ADDR_W, (uint32_t *)buf, len >> 2);
	else {

		/* Now we assume address in imap_nand_dma_list is valid */
		if(unlikely(imap_nand_dma_list[1].dma_addr))
		{
			writel(imap_nand_dma_list[1].dma_addr,
			   regs + iMAPX200_NFDMAADDR_B);
			writel(imap_nand_dma_list[1].len,
			   regs + iMAPX200_NFDMAC_B);
			writel(imap_nand_dma_list[0].dma_addr,
			   regs + iMAPX200_NFDMAADDR_A);
			writel(imap_nand_dma_list[0].len | iMAPX200_NFDMAC_DMAALT
			   | iMAPX200_NFDMAC_DMAAUTO | iMAPX200_NFDMAC_DMAEN,
			   regs + iMAPX200_NFDMAC_A);
		} else {
			/* address is consecutive */
			writel(imap_nand_dma_list[0].dma_addr,
			   regs + iMAPX200_NFDMAADDR_A);
			writel(imap_nand_dma_list[0].len | iMAPX200_NFDMAC_DMAEN,
			   regs + iMAPX200_NFDMAC_A);
		}

		/*
		 * TODO: Wait DMA tranportation finish, this should be changed to
		 *		IRQ mode
		 */
#if defined(NAND_USE_IRQ)
		wait_event_interruptible(imap_nand.wq,
				!(readl(regs + iMAPX200_NFDMAC_A) & iMAPX200_NFDMAC_DMAEN));
#else
		while(readl(regs + iMAPX200_NFDMAC_A) & iMAPX200_NFDMAC_DMAEN);
#endif
		imap_nand_unmap_vm(DMA_TO_DEVICE);
	}

	return ;
}
#endif
/**
 * imap_nand_read_oob_std - [REPLACABLE] the most common OOB data read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
static int imap_nand_read_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			     int page, int sndcmd)
{
	int seccloc = mtd->oobsize - chip->ecc.total - CONFIG_SYS_NAND_SECC_LEN;
    int secc_correct=0;
    
	if (sndcmd) {
		chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);
		sndcmd = 0;
	}
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	if(is_freepage(chip->oob_poi + seccloc - CONFIG_SYS_NAND_SELF_DATA_LEN)){  //self data is before ecc code 2
		memset(chip->oob_poi, 0xff, mtd->oobavail);
	}else{
    	/* Check if error occured in the first 32 bytes */
      #if defined(CONFIG_NAND_SECC)	
        secc_correct=nand_correct_data_bch(mtd, chip->oob_poi + chip->ecc.layout->oobfree[0].offset, 0, chip->oob_poi + seccloc);
        if(secc_correct)
            printk("secc err_correct* =%d\n",secc_correct);

        if (secc_correct < 0)
            mtd->ecc_stats.failed++;
        else if (secc_correct > 0)
            mtd->ecc_stats.corrected++;
      #endif
    }
	
	return sndcmd;
}

/**
 * imap_nand_write_oob_std - [REPLACABLE] the most common OOB data write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to write
 */
static int imap_nand_write_oob_std(struct mtd_info *mtd, struct nand_chip *chip,
			      int page)
{
	int status = 0;
	const uint8_t *buf = chip->oob_poi;
	int length = mtd->oobsize;
	int seccloc = mtd->oobsize - chip->ecc.total - CONFIG_SYS_NAND_SECC_LEN;

    dirty_mark(chip->oob_poi + seccloc - CONFIG_SYS_NAND_SELF_DATA_LEN);
	
	/* protect the first 32 byte */
  #if defined(CONFIG_NAND_SECC)	
	nand_calculate_ecc_bch(mtd, chip->oob_poi + chip->ecc.layout->oobfree[0].offset, chip->oob_poi + seccloc);
  #endif
	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);
	chip->write_buf(mtd, buf, length);
	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);

	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}


/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
 *                    uint8_t *buf)
 *
 * -Description:
 *    Read all data in current page in to buf, and all data in current
 *    oob in to chip->oop_poi.
 *    Data in main area of page is protected by MECC in oob area.
 *    MECC(if this is a SLC flash) and the first seccsteps * seccsize
 *    bytes in oob is protected by SECC.
 *    The rest of OOB and SECC has no protection.
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   nand_chip structure
 *    buf    buffer to store read data
 *                
 * -Return
 *    0
 *
 * -Others
 *    Science we now need to read the ECC codes out of OOB before any
 *    page data, so none of the default read_page method(HW, SW, SYND)
 *    can be used. This special odd function is applied here.
 ***********************************************************************
 */
static int imap_nand_read_page(struct mtd_info *mtd, struct nand_chip *chip,
   uint8_t *buf, int page)
{
	int stat;
	int eccsize  = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int meccloc = mtd->oobsize - /*eccbytes * eccsteps*/chip->ecc.total;
	uint8_t *p = buf;
	void __iomem *regs = imap_nand.regs;
	int seccloc = meccloc - CONFIG_SYS_NAND_SECC_LEN;
	int secc_correct = 0;

	/* No spare ECC availiable, read OOB out directly */
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, mtd->writesize, -1);
	chip->read_buf(mtd, chip->oob_poi, mtd->oobsize);

	/* If this is a free page, return 0xff values without reading */
	if(is_freepage(chip->oob_poi + seccloc - CONFIG_SYS_NAND_SELF_DATA_LEN)){  //self data is before ecc code 2
		memset(buf, 0xff, mtd->writesize);
		memset(chip->oob_poi,0xff,mtd->oobavail);
		return 0;
	}

	/* Check if error occured in the first 32 bytes */
	//nand_correct_data_bch(mtd, chip->oob_poi + 2, 0, chip->oob_poi + seccloc);
  #if defined(CONFIG_NAND_SECC)
    secc_correct=nand_correct_data_bch(mtd, chip->oob_poi + chip->ecc.layout->oobfree[0].offset, 0, chip->oob_poi + seccloc);
    if(secc_correct)
        printk("secc err_correct =%d\n",secc_correct);

    if (secc_correct < 0)
        mtd->ecc_stats.failed++;
    else if (secc_correct > 0)
        mtd->ecc_stats.corrected++;
  #endif 

	/* Read data main page data and do error correction */
	chip->cmdfunc(mtd, NAND_CMD_RNDOUT, 0, -1);
	for (;eccsteps; eccsteps--, p += eccsize, meccloc += eccbytes){
		/* Write relative ECC into NFMECCDX */
		writel((*(uint32_t *)(chip->oob_poi + meccloc)),
		   regs + iMAPX200_NFMECCD0);
		writel((*(uint32_t *)(chip->oob_poi + meccloc + 4)),
		   regs + iMAPX200_NFMECCD1);
		writel((*(uint32_t *)(chip->oob_poi + meccloc + 8)),
		   regs + iMAPX200_NFMECCD2);

		chip->ecc.hwctl(mtd, NAND_ECC_READ);
		chip->read_buf(mtd, p, eccsize);
		stat = chip->ecc.correct(mtd, p, NULL, chip->oob_poi + meccloc);

		if (stat < 0)
		    mtd->ecc_stats.failed++;
		else if (stat > 0)
		    mtd->ecc_stats.corrected++;
	}  
	return 0;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_write_page(struct mtd_info *mtd, struct nand_chip * chip,
 *                    const uint8_t *buf)
 *
 * -Description:
 *    write pagelength bytes from buf to current page, 
 *    fill SECC MECC to oob_poi, and write oob_poi to OOB
 *
 * -Input Param
 *    mtd    mtd device
 *    chip   nand_chip structure
 *    buf    buffer contains data to write
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_nand_write_page(struct mtd_info *mtd, struct nand_chip * chip,
   const uint8_t *buf)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	int meccloc  = mtd->oobsize - /*(eccbytes * eccsteps)*/chip->ecc.total;
	const uint8_t * p = buf;
	int seccloc = meccloc - CONFIG_SYS_NAND_SECC_LEN;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize){
		chip->ecc.hwctl(mtd, NAND_ECC_WRITE);
		chip->write_buf(mtd, p, eccsize);
		chip->ecc.calculate(mtd, p, chip->oob_poi + meccloc + i);
	}

	/* Mark this page as dirty */
	//*(uint32_t *)(chip->oob_poi + imap_oob_cfg->dirtymark) = 0;
    dirty_mark(chip->oob_poi + seccloc - CONFIG_SYS_NAND_SELF_DATA_LEN);
	
	/* protect the first 32 byte */
	//nand_calculate_ecc_bch(mtd, chip->oob_poi + 2, chip->oob_poi + seccloc);
  #if defined(CONFIG_NAND_SECC)
	nand_calculate_ecc_bch(mtd, chip->oob_poi + chip->ecc.layout->oobfree[0].offset, chip->oob_poi + seccloc);
  #endif   
	/* Program the date in OOB buffer to spare area on NAND */
	chip->write_buf(mtd, chip->oob_poi, mtd->oobsize);
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_hwinit(void)
 *
 * -Description:
 *    this is used before nand_scan to config base status.
 *
 * -Input Param
 *    none
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_nand_hwinit(void)
{
	/* Setting time args */
	uint32_t nfconf, nfcont;
	void __iomem *regs = imap_nand.regs;

	nfconf = readl(regs + iMAPX200_NFCONF);
	nfconf &= ~iMAPX200_NFCONF_TACLS_(iMAPX200_NFCONF_TMSK);
	nfconf &= ~iMAPX200_NFCONF_TWRPH0_(iMAPX200_NFCONF_TMSK);
	nfconf &= ~iMAPX200_NFCONF_TWRPH1_(iMAPX200_NFCONF_TMSK);
	nfconf |= iMAPX200_NFCONF_TACLS_(0x01);
	nfconf |= iMAPX200_NFCONF_TWRPH0_(0x03);
	nfconf |= iMAPX200_NFCONF_TWRPH1_(0x01);

	/* 
	 * If there are more than one chips with different buswidth on board,
	 * I'll have to move this into select_chip func.
	 */
#if defined(CONFIG_MTD_NAND_IMAPX200_BUSW16)
	nfconf |= iMAPX200_NFCONF_BusWidth16;
#else
	nfconf &= ~iMAPX200_NFCONF_BusWidth16;
#endif
	writel(nfconf, regs + iMAPX200_NFCONF);

	nfcont = readl(regs + iMAPX200_NFCONT);
	nfcont |= iMAPX200_NFCONT_MODE;
#if defined(NAND_USE_IRQ)
	/* Disable ALL INT */
	nfcont &= ~iMAPX200_NFCONT_INTMSK;
	nfcont |= iMAPX200_NFCONT_DMACompleteINT;
//	nfcont |= iMAPX200_NFCONT_EnECCEncINT;
//	nfcont |= iMAPX200_NFCONT_EnECCDecINT;
#endif
	nfcont &= ~iMAPX200_NFCONT_SoftLock;
	nfcont &= ~iMAPX200_NFCONT_LockTight;
	writel(nfcont, regs + iMAPX200_NFCONT);
	return ;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_init_chip(struct nand_chip *chip)
 *
 * -Description:
 *    this is used before nand_scan to fill nand_chip structure.
 *
 * -Input Param
 *    chip   chip to fill will function pointer.
 *                
 * -Return
 *    none
 ***********************************************************************
 */
static void imap_init_chip(struct nand_chip *chip)
{
	chip->IO_ADDR_W = chip->IO_ADDR_R = (imap_nand.regs) + iMAPX200_NFDATA;

    chip->parse_id      = flash_parse_id;
	chip->cmd_ctrl			= imap_nand_cmd_ctrl;
	chip->dev_ready			= imap_nand_ready;
	chip->select_chip		= imap_nand_select_chip;
	chip->read_buf			= imap_nand_read_buf;
	chip->write_buf			= imap_nand_write_buf;
	chip->options			= 0;
	chip->chip_delay	    = 60;
#if defined(CONFIG_MTD_NAND_IMAPX200_BUSW16)
	chip->options			|= NAND_BUSWIDTH_16;
#endif

	chip->ecc.hwctl			= imap_nand_ecc_hwctl;
	chip->ecc.calculate		= imap_nand_ecc_calculate;
	chip->ecc.correct		= imap_nand_ecc_correct;
	chip->ecc.read_page		= imap_nand_read_page;
	chip->ecc.write_page	= imap_nand_write_page;
    chip->ecc.read_oob      = imap_nand_read_oob_std;
    chip->ecc.write_oob     = imap_nand_write_oob_std;

	chip->ecc.mode			= NAND_ECC_HW;
#if defined(CONFIG_MTD_NAND_IMAPX200_MLC)
	chip->ecc.size			= 512;
	chip->ecc.bytes			= 9;
#else
	chip->ecc.size			= 2048;
	chip->ecc.bytes			= 8;
#endif
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_probe(struct platform_device *pdev)
 *
 * -Description:
 *    platform_driver probe
 *
 * -Input Param
 *    pdev    platform device pointer
 *                
 * -Return
 *    0 on success
 *    errno is any error happens
 ***********************************************************************
 */
static int imap_nand_probe(struct platform_device *pdev)
{
	struct resource *res;
#ifndef FPGA_TEST
	uint32_t clkrate;
#endif
	int err = 0, ret = 0, size;

	pr_debug("In probe function.\n");
#ifndef FPGA_TEST
	/* Get clk source and enable it */
	imap_nand.clk = clk_get(&pdev->dev, "nand");
	if(IS_ERR(imap_nand.clk))
	{
		dev_err(&pdev->dev, "iMAP NAND: Failed to get clock.\n");
		err = -ENOENT;
		goto exit_error;
	}

	clk_enable(imap_nand.clk);
#endif

	/* Allocate and remap the resources */
	res = pdev->resource;
	size = res->end - res->start + 1;

	imap_nand.area = request_mem_region(res->start, size, pdev->name);

	if(imap_nand.area == NULL)
	{
		dev_err(&pdev->dev, "Can not reserve register region.\n");
		err = -ENOENT;
		goto exit_error;
	}

	imap_nand.dev = &pdev->dev;
	imap_nand.regs = ioremap(res->start, size);

	if(imap_nand.regs == NULL)
	{
		dev_err(&pdev->dev, "Can not remap register address.\n");
		err = -EIO;
		goto exit_error;
	}

	/* Allocate memory for MTD, structure and private data */
	imap_nand.mtd = kmalloc((sizeof(struct mtd_info) + sizeof(struct nand_chip)), GFP_KERNEL);

	if(imap_nand.mtd == NULL)
	{
		dev_err(&pdev->dev, "Unable to allocate NAND MTD structure\n");
		return -ENOMEM;
	}

	/* Set zero to the structures */
	memset((char *)(imap_nand.mtd), 0, sizeof(struct mtd_info) + sizeof(struct nand_chip));

	/* Link nand_chip to private data of mtd */
	imap_nand.chip = (struct nand_chip *)(&(imap_nand.mtd)[1]);
	(imap_nand.mtd)->priv = imap_nand.chip;

	/* Initialize MTD */
	imap_nand.mtd->owner = THIS_MODULE;

	/* Initialize nand_chip options */
	imap_init_chip(imap_nand.chip);

#ifndef FPGA_TEST
	/* Initialize the hardware */
	clkrate = clk_get_rate(imap_nand.clk);
#endif
	imap_nand_hwinit();

	/* Clear the original STATUS bit */
	writel(readl(imap_nand.regs + iMAPX200_NFSTAT), imap_nand.regs + iMAPX200_NFSTAT);
#if defined(NAND_USE_IRQ)
	/* Get IRQ Number */
	imap_nand.irqno = platform_get_irq(pdev, 0);
	if (imap_nand.irqno < 0) {
		dev_err(&pdev->dev, "Get NAND IRQ No. failed.\n");
		return -ENOENT;
	}

	err = request_irq(imap_nand.irqno, imap_nand_irq,
				 IRQF_DISABLED, "imap-nand", &imap_nand);

	if (err) {
		dev_err(&pdev->dev, "IRQ%d error %d\n", imap_nand.irqno, ret);
		return err;
	}

	init_waitqueue_head(&imap_nand.wq);
#endif

    /* Init BCH module */
  #if defined(CONFIG_NAND_SECC)  
    bch_init();
  #endif
	/* Scan NAND device to see if every thing OK */
	if (nand_scan(imap_nand.mtd, 1)){
		ret = -ENXIO;
		goto exit_error;
	}

	/* Assign oob layout, this must be done after nand_scan */
	//imap_nand_assign_layout(imap_nand.mtd);

	/* Register the partitions */
	add_mtd_partitions(imap_nand.mtd, imap_nand_parts, ARRAY_SIZE(imap_nand_parts));
	
	pr_debug("Initialized OK.\n");
	return 0;

exit_error:
	kfree(imap_nand.mtd);

	return ret;
}

/*!
 ***********************************************************************
 * -Function:
 *    imap_nand_suspend(struct platform_device *pdev, pm_message_t pm)
 *    imap_nand_resume(struct platform_device *pdev)
 *
 * -Description:
 *    PM functions
 *
 * -Input Param
 *                
 * -Return
 *    0
 ***********************************************************************
 */
#ifdef CONFIG_PM
static int imap_nand_suspend(struct platform_device *pdev, pm_message_t pm)
{
	/* Pull high nCE0 to disable NAND chip */
	imap_nand.chip->select_chip(NULL, -1);
#ifndef FPGA_TEST
	/* Stop clock */
	clk_disable(imap_nand.clk);
#endif
	
	return 0;
}

static int imap_nand_resume(struct platform_device *pdev)
{
#ifndef FPGA_TEST
	/* Enable clock */
	clk_enable(imap_nand.clk);
#endif

	/* Set nfconf & nfcont */
	imap_nand_hwinit();

	/* Pull nCE0 low to enable NAND chip */
	imap_nand.chip->select_chip(NULL, 0);

	return 0;
}
#endif

static int imap_nand_remove(struct platform_device *pdev)
{
	kfree(imap_nand.mtd);
	platform_set_drvdata(pdev, NULL);
	return 0;
}

static struct platform_driver imap_nand_driver = {
	.probe		= imap_nand_probe,
	.remove		= imap_nand_remove,
#ifdef CONFIG_PM
	.suspend	= imap_nand_suspend,
	.resume		= imap_nand_resume,
#endif
	.driver		= {
		.name		= "imapx200_nand",
		.owner		= THIS_MODULE,
	},
};

static int __init imap_nand_init(void)
{
	int _dma_en = 0, _secc_en = 0, _irq_en = 0;
	printk(KERN_INFO "iMAPx200 NAND MTD Driver (c) 2009,2014 InfoTM\n");

#if defined(CONFIG_MTD_NAND_IMAPX200_SDMA)
	_dma_en = 1;
#endif
#if defined(NAND_USE_IRQ)
	_irq_en = 1;
#endif
#if defined(CONFIG_MTD_NAND_IMAPX200_OOBECC)
	_secc_en = 1;
#endif

	printk(KERN_INFO "Method: IRQ(%d), DMA(%d), SECC(%d), DEC.RDY\n",
	   _irq_en, _dma_en, _secc_en);
	return platform_driver_register(&imap_nand_driver);
}

static void __exit imap_nand_exit(void)
{
	platform_driver_unregister(&imap_nand_driver);
}

module_init(imap_nand_init);
module_exit(imap_nand_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("warits <warits.wang@infotmic.com.cn>");
MODULE_DESCRIPTION("iMAPx200 NAND MTD Driver");
