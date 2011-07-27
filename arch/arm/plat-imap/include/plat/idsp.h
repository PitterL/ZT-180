/***************************************************************************** 
** plat/idsp.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: iDPS structures and functions.
**
** Author:
**     warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  08/13/2010
*****************************************************************************/

#ifndef __IX_DSP_H__
#define __IX_DSP_H__

#define iDSP_POWER_BIT (1 << 5)

#define iDSP_TCM_BASE	(0x2c000000)
#define iDSP_TCM_LEN	(0x1c800)

#define IDSP_MAILBOX_COUNT 2

//#define dsp_msg(args...) printk(KERN_INFO "MSG/iDSP: " args)
//#define dsp_dbg(args...) printk(KERN_ERR "iDSP: " args)
#define dsp_msg(args...)
#define dsp_dbg(args...)

struct idsp_desc {
	uint32_t		endian0;
	uint32_t		endian1;
	uint32_t		epm_base;
	uint32_t		edm_base;
	uint32_t		csr_base;
	uint32_t		b_addr;
};

struct ix_dsp_dev {

	void __iomem	* mapped_tcm;
	void __iomem	* regs;
	int				irqno;
	void			(* ite)(void);
	spinlock_t		lock;
	uint32_t		b_addr;
};

enum idsp_mail_actor {
	IDSP_MAIL_SENDER = 0,
	IDSP_MAIL_RECEIVER,
};

enum idsp_mail_state {
	IDSP_MAIL_IDLE = 0, /* not used */
	IDSP_MAIL_NOTFULL,
	IDSP_MAIL_UNREAD_MSG,
	IDSP_MAIL_BUSY,		/* not used */
};

struct idsp_mail_desc {
	
	enum idsp_mail_actor
			act;
};

/* iDSP APIs */
extern void idsp_set_extend_intr(uint32_t int1, uint32_t int2, int en);
extern void idsp_set_external_intr(uint32_t bits, int en);
extern void idsp_reset(void);
extern void idsp_enable(int en);
extern int idsp_mail_dword(int no, uint32_t *dat, int dir);
extern int idsp_mail_isfull(int no);
extern int idsp_mail_get_count(int no);
extern uint32_t idsp_intr_status(int side);
extern int idsp_intr_enable(int side, uint32_t bits, int en);
extern int idsp_intr_clrpnd(int side, uint32_t bits);
extern void inline idsp_set_epm(uint32_t addr);
extern void inline idsp_set_edm(uint32_t addr);
extern void inline idsp_set_csr(uint32_t addr);
extern void inline idsp_set_endian(int dir, uint32_t mode);
extern void inline idsp_cache_on(int en);
extern void inline idsp_cache_flush(void);
extern void inline idsp_set_tcm(uint8_t *dat, uint32_t offs, uint32_t len);
extern void idsp_boot_jump(void);
extern int idsp_sw_init(struct idsp_desc *desc);
extern int idsp_set_ite(void (* func)(void));
extern void * idsp_get_tcm_base(void);

/* iDSP mail APIs */
extern int idsp_mail_send(int no, uint32_t *dat, int count);
extern int idsp_mail_receive(int no, uint32_t *dat, int count);
extern int idsp_mail_init(struct idsp_mail_desc *desc);
extern int idsp_mail_unread_msg(int no);
extern void idsp_mail_ite(void);

/* functions */
extern struct ix_dsp_dev ix_dsp;
static uint32_t inline dsp_readl(uint32_t offs)
{
	return readl(ix_dsp.regs + offs);
}

static void inline dsp_writel(uint32_t value, uint32_t offs)
{
	writel(value, ix_dsp.regs + offs);
}

#endif  /* __IX_DSP_H__ */
