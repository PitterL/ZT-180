/***************************************************************************** 
** drivers/usb/gadget/ix200_udc.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: PCB test, module .
**
** Author:
**      warits <warits.wang@infotm.com.cn>
**    
** Revision History: 
** ----------------- 
** 1.1   06/30/2010
*****************************************************************************/

#ifndef __IMAP_UDC_H__
#define __IMAP_UDC_H__

#define IX_EP_COUNT	7
#define EP0_MAXPACKET 64
#define EP_MAXPACKET 512

struct ix200_udc {

    struct work_struct irq_work;
    uint32_t isr;

    uint32_t configured;

	spinlock_t					lock;
	struct usb_ep				ep[IX_EP_COUNT];
	struct list_head			ep_req[IX_EP_COUNT];
	const struct usb_endpoint_descriptor
								*desc[IX_EP_COUNT];
	struct usb_gadget			gadget;
	struct usb_gadget_driver	*driver;
	struct usb_request			req;
	uint32_t					devstat;
	uint32_t					portstat;
	uint32_t					ep0stat;

	/* Extra status */
	uint32_t					got_irq : 1;
	unsigned					req_std : 1;
	unsigned					req_config : 1;
	unsigned					req_pending : 1;
	uint32_t					vbus;
	void			__iomem		*regbase;
	int							irqno;
	struct	clk					*clk;
};


/* externs */
static int ix_ep_enable(struct usb_ep *ep, 
   const struct usb_endpoint_descriptor *desc);
static int ix_ep_disable(struct usb_ep *ep);
static struct usb_request *
ix_alloc_request(struct usb_ep *ep, gfp_t mem_flags);
static void
ix_free_request(struct usb_ep *ep, struct usb_request *req);
static int ix_queue(struct usb_ep *ep, struct usb_request *req,
   gfp_t gfp_flags);
static int ix_dequeue(struct usb_ep *ep, struct usb_request *req);
static int ix_set_halt(struct usb_ep *ep, int value);
static int ix_get_frame(struct usb_gadget *gadget);
static int ix_wakeup(struct usb_gadget *gadget);
static int ix_set_selfpowered(struct usb_gadget *gadget, int value);
static int ix_pullup(struct usb_gadget *gadget, int is_on);
static int ix_vbus_session(struct usb_gadget *gadget, int is_active);
static int ix_vbus_draw(struct usb_gadget *gadget, unsigned ma);
static void ix_udc_en(int en);
static void ix_ep_init(void);
static void __req_done(struct usb_ep *ep,
   struct usb_request *req, int stat);
static void __req_nuke(struct usb_ep *ep, int stat);

static void __iomem *__reg;
static uint8_t *d_buffer;

#define DEBUG_NORMAL 0
#define DEBUG_VERBOSE 0
static int dprintk(int level, const char *fmt, ...)
{
	static char printk_buf[1024];
	va_list args;

//	if(level != 4)	/* 1 is always observable */
	if(level < 1)	/* 1 is always observable */
//	if(0)
	  return 0;
	va_start(args, fmt);
	vscnprintf(printk_buf,
	   sizeof(printk_buf), fmt, args);
	va_end(args);

	return printk(KERN_ERR "%s", printk_buf);
}

static void dprintreg(void)
{
	uint32_t i;
	printk(KERN_ERR "Medusa regs:\n");
	for(i = USB_ACR; i <= USB_EDR6; i += 4)
	  printk(KERN_ERR "0x%04x: 0x%08x\n", i, readl(__reg + i));

	printk(KERN_ERR "Link regs:\n");
	for(i = USB_BCWR; i <= USB_IPCR; i += 4)
	  printk(KERN_ERR "0x%04x: 0x%08x\n", i, readl(__reg + i));

	while(1);
	return ;
}
#endif /* __IMAP_UDC_H__ */
