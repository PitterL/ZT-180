/***************************************************************************** 
**  driver/dsp/dspipc.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: InfoTM iDPS operation APIs.
**
** Author:
**      
**      warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 08/09/2010 XXX	
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
#include <linux/cpufreq.h>
#include <linux/interrupt.h>

#include <asm/io.h>
#include <asm/irq.h>
#include <mach/hardware.h>

#include <plat/idsp.h>

struct ix_dsp_dev ix_dsp;

/* func: config DPS extend external interrupt */
void idsp_set_extend_intr(uint32_t int1, uint32_t int2, int en)
{
	if(en)
	{
	    writel(readl(rDEINTMSK1) & ~(int1), rDEINTMSK1); 
	    writel(readl(rDEINTMSK2) & ~(int2), rDEINTMSK2); 
	} else {
	    writel(readl(rDEINTMSK1) | (int1), rDEINTMSK1); 
	    writel(readl(rDEINTMSK2) | (int2), rDEINTMSK2); 
	}
}

/* func: config DSP side external interrupts */
void idsp_set_external_intr(uint32_t bits, int en)
{
	if(en)
	  dsp_writel(dsp_readl(iDSP_EXTINT_MASK) & ~(bits),
		 iDSP_EXTINT_MASK);
	else
	  dsp_writel(dsp_readl(iDSP_EXTINT_MASK) | bits,
		 iDSP_EXTINT_MASK);
}

/* XXX dsp register operatons */

/* func: reset dsp system */
void idsp_reset(void)
{
	dsp_writel(dsp_readl(iDSP_DSP_RESET) | iDSP_DoReset,
	   iDSP_DSP_RESET); 
}

/* func: turn on/off iDSP module */
void idsp_enable(int en)
{
	if(en)
	{
		/* power on DSP module */
		writel(readl(rNPOW_CFG) | iDSP_POWER_BIT, rNPOW_CFG);

		while(!(readl(rPOW_ST) & iDSP_POWER_BIT));

		/* open CLK mask */
		writel(readl(rHCLK_MASK) & ~IMAP_CLKCON_HCLK_DSP, rHCLK_MASK);
		writel(readl(rSCLK_MASK) & ~IMAP_CLKCON_SCLK_DSP, rSCLK_MASK);

		/* reset DSP */
		writel(readl(rMD_RST) | iDSP_POWER_BIT, rMD_RST);

		/* reset APB */
		writel(readl(rAHBP_RST) | IMAP_CLKCON_HCLK_DSP_AHB, rAHBP_RST);
		udelay(20);

		/* release DSP APB interface */
		writel(readl(rAHBP_RST) & ~IMAP_CLKCON_HCLK_DSP_AHB, rAHBP_RST);

		/* release isolation */
		writel(readl(rMD_ISO) & ~iDSP_POWER_BIT, rMD_ISO);

		/* release reset */
		writel(readl(rMD_RST) & ~iDSP_POWER_BIT, rMD_RST);

		/* release boe */
		writel(readl(rAHBP_EN) | IMAP_CLKCON_HCLK_DSP_AHB, rAHBP_EN);
	} else {
		/* power down DSP module */
		/* isolation */
		writel(readl(rMD_ISO) | iDSP_POWER_BIT, rMD_ISO);
		 
		/* reset DSP */
		writel(readl(rMD_RST) | iDSP_POWER_BIT, rMD_RST);

		/* mask DSP clock */
		writel(readl(rHCLK_MASK) | IMAP_CLKCON_HCLK_DSP, rHCLK_MASK);
		writel(readl(rSCLK_MASK) | IMAP_CLKCON_SCLK_DSP, rSCLK_MASK);

		/* power off */
		writel(readl(rNPOW_CFG) & ~iDSP_POWER_BIT, rNPOW_CFG);
		
		/* wait until no power is left */
		while(readl(rPOW_ST) & iDSP_POWER_BIT);
	}
}

/* mail one double word through mail system
 * no:  mailbox number
 * dat: dat to be transfered or save received data
 * dir: transfer direction
 *      0, write to mailbox
 *      1, read from mailbox
 */
int idsp_mail_dword(int no, uint32_t *dat, int dir)
{
	if(no != !!no)
	  /* not a valid mailbox */
	  return -1;

	if(dir)
	{
		/* read from mailbox */
		*dat = dsp_readl(iDSP_MAILBOX(no));

		/* clear mailbox */
		dsp_writel(0, iDSP_MSG_COUNT(no));
	} else {
		/* write data to mailbox */
		dsp_writel(*dat, iDSP_MAILBOX(no));
	}
	return 0;
}

/* func: check if mailbox is full
 * retn: 1, full
 *       0, empty
 */
int idsp_mail_isfull(int no)
{
	int stat;

	stat = dsp_readl(iDSP_FIFOSTATUS(no))
			& iDSP_FIFOSTATUS_FULL;

	return !!stat;
}

/* func: get word count recived */
int idsp_mail_get_count(int no)
{
	return dsp_readl(iDSP_MSG_COUNT(no)) & iDSP_MSG_COUNT_MSK;
}

/* func: get interrupt status
 * side: 0, get DSP interrupt status
 *       1, get CPU interrupt status
 */
uint32_t idsp_intr_status(int side)
{
	if(side != !!side)
	  return 0;

	return dsp_readl(iDSP_INTSTAT(side)) & 0xf;
}

/* func: set intr bits
 * side: 0, set INTDSP
 *       1, set INTCPU
 */
int idsp_intr_enable(int side, uint32_t bits, int en)
{
	if(side != !!side)
	  /* unrecognized side */
	  return -1;

	if(en)
	  dsp_writel(dsp_readl(iDSP_INTEN(side)) | bits,
		 iDSP_INTEN(side));
	else 
	  dsp_writel(dsp_readl(iDSP_INTEN(side)) & ~bits,
		 iDSP_INTEN(side)); 
	
	return 0;
}

/* func: clear int pend */
int idsp_intr_clrpnd(int side, uint32_t bits)
{
	if(side != !!side)
	  /* unrecognized side */
	  return -1;

	dsp_writel(bits, iDSP_INTSTAT(side));
	return 0;
}

/* func: set epm base */
void inline idsp_set_epm(uint32_t addr)
{
	dsp_writel(addr, iDSP_EPM_BASE);
}

/* func: set edm base */
void inline idsp_set_edm(uint32_t addr)
{
	dsp_writel(addr, iDSP_EDM_BASE);
}

/* func: set csr base */
void inline idsp_set_csr(uint32_t addr)
{
	dsp_writel(addr, iDSP_CSR_BASE);
}

/* func: set dsp endian mode */
void inline idsp_set_endian(int dir, uint32_t mode)
{
	if(dir != !!dir)
	  return ;

	dsp_writel(mode, iDSP_ENDIAN_MODE(dir));
}

/* func: switch on/off dsp cache */
void inline idsp_cache_on(int en)
{
	dsp_writel(!!en, iDSP_CACHE_ENABLE);
}

/* func: flush cache */
void inline idsp_cache_flush(void)
{
	dsp_writel(dsp_readl(iDSP_CACHE_FLUSH) | 0x1,
	   iDSP_CACHE_FLUSH);
}

/* func: copy data to DSP TCM area
 * note: must be 4byte aligned
 */
void inline idsp_set_tcm(uint8_t *dat, uint32_t offs, uint32_t len)
{
	writesl(ix_dsp.mapped_tcm + offs, (uint32_t *)dat,
	   len >> 2);
}

/* func: get TCM base */
void * idsp_get_tcm_base(void)
{
	return ix_dsp.mapped_tcm;
}

/* func: set dsp jump address and run program */
void idsp_boot_jump(void)
{
	dsp_writel(ix_dsp.b_addr & 0xffffff, iDSP_BOOT_INSTR);
}

/* func: software init DSP module */
int idsp_sw_init(struct idsp_desc *desc)
{
	/* set endian mode */
	idsp_set_endian(0, desc->endian0);
	idsp_set_endian(1, desc->endian1);

	/* set extend interrupt */
	idsp_set_extend_intr(0xffffffff, 0xffffffff, 0);

	/* set dsp external interrupt */
	idsp_set_external_intr(0xffffff, 1);

	/* set memory base */
	idsp_set_epm(desc->epm_base);
	idsp_set_edm(desc->edm_base);
	idsp_set_csr(desc->csr_base);

	/* set boot address */
	ix_dsp.b_addr = desc->b_addr;

	return 0;
}

/* func: DSPIPC irq service function */
static irqreturn_t idsp_interrupt(int irq, void *id)
{
	/* call registered ite */
	if(ix_dsp.ite)
	{
		/* call mailbox ite */
		idsp_mail_ite();

		(*ix_dsp.ite)();
	}

	/* we do not handle any remaining pending interrupt
	 * after every ite has executed.
	 */
	idsp_intr_clrpnd(1, 0xffffffff);

	return IRQ_HANDLED;
}

/* func: set an event which will be trigger by
 * dsp interrupt.
 * set interrupt triggered event will cause
 * isr mounted.
 * func: NULL to disbind function from interrupt
 */
int idsp_set_ite(void (* func)(void))
{
	int err;

	if(!func)
	{
		/* remove ite and free irq */
		if(ix_dsp.ite)
		{
			ix_dsp.ite = NULL;
			free_irq(ix_dsp.irqno, &ix_dsp);
		}

		return 0;
	}

	ix_dsp.ite = func;

	err = request_irq(ix_dsp.irqno, idsp_interrupt,
	   IRQF_DISABLED, "dspipc", &ix_dsp);

	if(err) {
		/* set to NULL */
		ix_dsp.ite = NULL;

		printk(KERN_ERR "IRQ %d error %d\n", ix_dsp.irqno, err);
		return err;
	}

	printk(KERN_INFO "Function (%p) bind to DSPIPC interrupt.\n", func);
	return 0;
}

static int ix_dsp_probe(struct platform_device *pdev)
{
	struct resource *res;
	int err = 0;

	memset(&ix_dsp, 0, sizeof(struct ix_dsp_dev));
	/* reloc tcm area */
	ix_dsp.mapped_tcm = ioremap(iDSP_TCM_BASE, iDSP_TCM_LEN);
	dsp_dbg("Alloc copvp %p\n", ix_dsp.mapped_tcm);

	if(!ix_dsp.mapped_tcm)
	{
		dev_err(&pdev->dev, "can not remap tcm\n");
		return -EIO;
	}

	/* register IRQ */
	res = pdev->resource;
	ix_dsp.irqno = platform_get_irq(pdev, 0);
	if(ix_dsp.irqno < 0) {
		dev_err(&pdev->dev, "Get IDSPIPC IRQ No. failed.\n");
		err = -ENOENT;
		goto __err_exit_1;
	}

	/* remap iDSP registers */
	ix_dsp.regs = ioremap(res->start, res->end - res->start + 1);
	if(!ix_dsp.regs)
	{
		dev_err(&pdev->dev, "Can not remap register for iDSP.\n");
		err = -EIO;
		goto __err_exit_2;
	}

	/* init spin lock */
	spin_lock_init(&ix_dsp.lock);

	dsp_dbg("probe ok.\n");
	return 0;

__err_exit_2:
	iounmap(ix_dsp.regs);
__err_exit_1:
	iounmap(ix_dsp.mapped_tcm);

	return err;
}

static int ix_dsp_remove(struct platform_device *pdev)
{
	iounmap(ix_dsp.regs);
    iounmap(ix_dsp.mapped_tcm);

	return 0;
}

#define ix_dsp_suspend	NULL
#define ix_dsp_resume	NULL

static struct platform_driver ix210_dsp_driver = {
	.probe		= ix_dsp_probe,
	.remove		= ix_dsp_remove,
#ifdef CONFIG_PM
	.suspend	= ix_dsp_suspend,
	.resume		= ix_dsp_resume,
#endif
	.driver		= {
		.name		= "imap_dsp",
		.owner		= THIS_MODULE,
	},
};

static int __init ix_dsp_init(void)
{
	printk(KERN_INFO "IX210 iDSP driver (c) 2009, 2014 InfoTM\n");
	return platform_driver_register(&ix210_dsp_driver);
}

static void __exit ix_dsp_exit(void)
{
	platform_driver_unregister(&ix210_dsp_driver);
}

module_init(ix_dsp_init);
module_exit(ix_dsp_exit);
MODULE_LICENSE("Dual BSD/GPL");
MODULE_AUTHOR("warits <warits.wang@infotmic.com.cn>");
MODULE_DESCRIPTION("IX210 DSP Driver");

