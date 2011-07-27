/***************************************************************************** 
** drivers/ixdsp/core/mailbox.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: IPC mailbox APIs.
**
** Author:
**     warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  08/11/2010
*****************************************************************************/
#include <linux/types.h>
#include <linux/sched.h>

#include <asm/io.h>
#include <mach/hardware.h>

#include <plat/idsp.h>

#define dsp_dbg(x...) printk(KERN_ERR "D/iDSP(Mail): " x)
//#define dsp_dbg(x...)

struct idsp_mail_desc ix_dsp_mb[2];

/* mailbox interrupt triggered event */
void idsp_mail_ite(void)
{
	uint32_t dspint, i;

	dspint = idsp_intr_status(1);

	dsp_dbg("dspint = %08x\n", dspint);
	dsp_dbg("%d msgs in fifo\n", dsp_readl(iDSP_MSG_COUNT(1)));
	for(i = 0; i < IDSP_MAILBOX_COUNT; i++)
	{
		if(ix_dsp_mb[i].act == IDSP_MAIL_SENDER)
		{
			if(dspint & iDSP_INTSTAT_NOFULL(i))
			{
//				idsp_intr_enable(1, iDSP_INTSTAT_NOFULL(i), 0);
				idsp_intr_clrpnd(1, iDSP_INTSTAT_NOFULL(i));
			}
		} else {
			if(dspint & iDSP_INTSTAT_NEWMSG(i)) {
//				idsp_intr_enable(1, iDSP_INTSTAT_NEWMSG(i), 0);
				idsp_intr_clrpnd(1, iDSP_INTSTAT_NEWMSG(i));
			}
		}
	}

	return ;
}

/* clear mailbox Tx/Rx FIFO
 * no: mailbox number
 */
void idsp_mail_fifo_clear(int no)
{
	uint32_t i, tmp;

	dsp_dbg("clearing fifo %d\n", no);
	if(no != !!no)
	  /* not valid mailbox */
	  return;

	dsp_dbg("clearing fifo %d ..step2\n", no);
	for(i = 0; i < 4; i++)
	  idsp_mail_dword(no, &tmp, 1);

	return ;
}

/* request the correct stat of mailbox no.
 * this is a block invoke, function will return
 * until the correct stat arrived.
 * NOTE: If requiring a NOTFULL state, this function will not block
 * the process. If the mailbox keeps full in 20ms, then an error 
 * is returned.
 *       If requring a NEW_MSG state, this function will block
 * the process until a new msg arrived.
 */
int idsp_mail_state_request(int no, enum idsp_mail_state stat)
{
	unsigned long to = jiffies + 20 * HZ / 1000;

	if((no != !!no) || (stat > IDSP_MAIL_BUSY))
	  /* invalid state */
	  return -EINVAL;

	switch(stat)
	{
		case IDSP_MAIL_NOTFULL:
		{
			/* requiring a NOTFULL state */
			while(dsp_readl(iDSP_FIFOSTATUS(no)) & iDSP_FIFOSTATUS_FULL)
			{
				if(jiffies > to)
				  /* time out, return error */
				  return -EAGAIN;
				schedule();
			}
			break;
		}
		case IDSP_MAIL_UNREAD_MSG:
		{
			/* requiring a NEW_MSG state */
			while(!dsp_readl(iDSP_MSG_COUNT(no)))
			{
				if(jiffies > to)
				  /* time out, return error */
				  return -EAGAIN;
				schedule();
			}
			break;
		}
		case IDSP_MAIL_IDLE:
		case IDSP_MAIL_BUSY:
		default:
			/* keep gcc happy */;
	}

	return 0;
}

/* This function will block the process until the mail is sent
 */
int idsp_mail_send(int no, uint32_t *dat, int count)
{
	int i;

	if(ix_dsp_mb[no].act != IDSP_MAIL_SENDER)
	  dsp_msg("Warnning: Mailbox(%d) do not act as sender!\n", no);

	for(i = 0; i < count; i++)
	{
		/* wait until mailbox have space */
		while(idsp_mail_state_request(no, IDSP_MAIL_NOTFULL));
		idsp_mail_dword(no, dat + i, 0);
	}
	return 0;
}


/* This function will block the process until the required number
 * of message is sent.
 */
int idsp_mail_receive(int no, uint32_t *dat, int count)
{
	int i;

	if(ix_dsp_mb[no].act != IDSP_MAIL_RECEIVER)
	  dsp_msg("Warnning: Mailbox(%d) do not act as sender!\n", no);

	for(i = 0; i < count; i++)
	{
		/* wait until new mail recieved */
		while(idsp_mail_state_request(no, IDSP_MAIL_UNREAD_MSG));
		idsp_mail_dword(no, dat + i, 1);
	}

	return 0;
}

/* Initialize mailbox system */
int idsp_mail_init(struct idsp_mail_desc *desc)
{
	int i;

	/* set mailbox attribute */
	for(i = 0; i < IDSP_MAILBOX_COUNT; i++)
	{
		ix_dsp_mb[i].act  = desc[i].act;
		dsp_dbg("Mailbox %d act as %d\n", i, ix_dsp_mb[i].act);

		/* clear fifo */
		idsp_mail_fifo_clear(i);

		/* clear pnd */
		idsp_intr_clrpnd(1,
		   iDSP_INTSTAT_NOFULL(i) | iDSP_INTSTAT_NEWMSG(i));
		/* turn off interrupt */
		idsp_intr_enable(1, iDSP_INTSTAT_NOFULL(i)
		   | iDSP_INTSTAT_NEWMSG(i), 0);
		if(ix_dsp_mb[i].act == IDSP_MAIL_RECEIVER)
		{
			idsp_intr_enable(1, iDSP_INTSTAT_NEWMSG(i), 1);
			dsp_dbg("New message interrupt actived for box %d\n", i);
		} else {
			dsp_dbg("Enable DSP new message interrupt for box %d\n", i);
			idsp_intr_enable(0, iDSP_INTSTAT_NEWMSG(i), 1);
		}
	}

	return 0;
}

int idsp_mail_unread_msg(int no)
{
	return (int)dsp_readl(iDSP_MSG_COUNT(no));
}
