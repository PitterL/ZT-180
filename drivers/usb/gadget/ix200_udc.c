/***************************************************************************** 
** drivers/usb/gadget/ix200_udc.c
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
**      warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 0  XXX 06/30/2010 XXX	
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/list.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>
#include <linux/clk.h>
#include <linux/gpio.h>

#include <linux/debugfs.h>
#include <linux/seq_file.h>

#include <linux/usb.h>
#include <linux/usb/gadget.h>

#include <asm/byteorder.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/system.h>
#include <asm/unaligned.h>
#include <mach/irqs.h>

#include <mach/hardware.h>
#include "ix200_udc.h"

static int reset_imap_available = 1;

/* FIXME: Need extern in .h */

static const struct usb_ep_ops ix_ep_ops = {
	.enable			= ix_ep_enable,
	.disable		= ix_ep_disable,
	.alloc_request	= ix_alloc_request,
	.free_request	= ix_free_request,
	.queue			= ix_queue,
	.dequeue		= ix_dequeue,
	.set_halt		= ix_set_halt,
};

static const struct usb_gadget_ops ix_gadget_ops = {
	.get_frame			= ix_get_frame,
	.wakeup				= ix_wakeup,
	.set_selfpowered	= ix_set_selfpowered,
	.pullup				= ix_pullup,
	.vbus_session		= ix_vbus_session,
	.vbus_draw			= ix_vbus_draw,
};

/* the iMAPx200 UDC */
static struct ix200_udc ix_udc = {
	.gadget = {
		.is_otg		= 0,
		.ops		= &ix_gadget_ops,
		.ep0		= &ix_udc.ep[0],
		.name		= "iMAPx200-UDC",
		.dev		= {
			.init_name = "gadget",
		},
	},

	/* ep0 */
	.ep[0].name = "ep0",
	.ep[0].maxpacket = EP0_MAXPACKET,
	.ep[1].name = "ep1",
	.ep[1].maxpacket = EP_MAXPACKET,
	.ep[2].name = "ep2",
	.ep[2].maxpacket = EP_MAXPACKET,
	.ep[3].name = "ep3",
	.ep[3].maxpacket = EP_MAXPACKET,
	.ep[4].name = "ep4",
	.ep[4].maxpacket = EP_MAXPACKET,
	.ep[5].name = "ep5",
	.ep[5].maxpacket = EP_MAXPACKET,
	.ep[6].name = "ep6",
	.ep[6].maxpacket = EP_MAXPACKET,

	.devstat = 0,
};

int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
	int ret;
	dprintk(DEBUG_NORMAL, "usb_gadget_register_driver() '%s'\n",
		driver->driver.name);

	if(ix_udc.driver)
	  return -EBUSY;

	if (!driver->bind || !driver->setup
			|| driver->speed < USB_SPEED_FULL) {
		printk(KERN_ERR "Invalid driver: bind %p setup %p speed %d\n",
			driver->bind, driver->setup, driver->speed);
		return -EINVAL;
	}
#if defined(MODULE)
	if (!driver->unbind) {
		printk(KERN_ERR "Invalid driver: no unbind method\n");
		return -EINVAL;
	}
#endif

	/* Hook the driver */
	ix_udc.driver = driver;
	ix_udc.gadget.dev.driver = &driver->driver;

	/* Bind the driver */
	if ((ret = device_add(&ix_udc.gadget.dev)) != 0) {
		printk(KERN_ERR "Error in device_add() : %d\n",ret);
		goto register_error;
	}

	dprintk(DEBUG_NORMAL, "binding gadget driver '%s'\n",
		driver->driver.name);

	dprintk(1, "ep0max=%x, driver->bind = 0x%x\n", ix_udc.ep[0].maxpacket, driver->bind);
	if ((ret = driver->bind (&ix_udc.gadget)) != 0) {
		device_del(&ix_udc.gadget.dev);
		goto register_error;
	}

	/* Enable UDC */
	ix_udc_en(1);

	return 0;

register_error:
	ix_udc.driver = NULL;
	ix_udc.gadget.dev.driver = NULL;
	return ret;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
	if (!driver || driver != ix_udc.driver || !driver->unbind)
		return -EINVAL;

	dprintk(DEBUG_NORMAL,"usb_gadget_register_driver() '%s'\n",
		driver->driver.name);

	driver->unbind(&ix_udc.gadget);
	device_del(&ix_udc.gadget.dev);
	ix_udc.driver = NULL;

	/* Disable UDC */
	ix_udc_en(0);

	return 0;
}
EXPORT_SYMBOL(usb_gadget_unregister_driver);
EXPORT_SYMBOL(usb_gadget_register_driver);


/* the implement */
static inline struct list_head *
ix_find_queue(struct usb_ep *ep)
{
	struct ix200_udc *udc = &ix_udc;
	int i;
	for(i = 0; i < IX_EP_COUNT; i++)
	  if(ep == &udc->ep[i])
		return &udc->ep_req[i];

	return NULL;
}
static void ix_set_desc(struct usb_ep *ep, const struct usb_endpoint_descriptor *desc)
{
	return ;
}
static int ix_get_ep_num(struct usb_ep *ep)
{
	int i;

	for (i = 0; i < IX_EP_COUNT; i++)
	  if(&ix_udc.ep[i] == ep)
		break;

	return i;
}
static int ix_ep_enable(struct usb_ep *ep, 
   const struct usb_endpoint_descriptor *desc)
{
	uint32_t max, edr = 0;
	unsigned long flags;
	int ep_num = ix_get_ep_num(ep);

	printk("ix_ep_enable: %s, ep%d\n", __func__, ep_num);
	if (!ep || !desc /* || ep->desc */
			/* || ep->name == "ep0" */
			|| desc->bDescriptorType != USB_DT_ENDPOINT)
		return -EINVAL;

	if(!ix_udc.driver || ix_udc.gadget.speed == USB_SPEED_UNKNOWN)
	  return -ESHUTDOWN;

	max = desc->wMaxPacketSize & 0x1fff;
	local_irq_save(flags);
	ep->maxpacket = max & 0x7ff;

	dprintk(1, "maxpacket:%x\n", max & 0x7ff);
	ix_udc.desc[ep_num] = desc;

	/* write maxpacket value to hw */
	if(!ep_num)
	  writel(readl(__reg + USB_IER) | USB_EP0,
		 __reg + USB_IER);
	else
	{
		if(desc->bEndpointAddress & USB_DIR_IN)
		{
			edr |= USB_EP_DirIn;
			edr |= USB_InBuf(1);		/* select TBCR1 */
		}

		dprintk(1, "DescriptorType %x\n", desc->bDescriptorType);
		switch(desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		{
			case USB_ENDPOINT_XFER_BULK:
				edr |= USB_EP_Type(0x1);
				break;
			case USB_ENDPOINT_XFER_ISOC:
				edr |= USB_EP_Type(0x4);
				break;
			case USB_ENDPOINT_XFER_INT:
				edr |= USB_EP_Type(0x2);
				break;
		}

		edr |= USB_LogicNo(ep_num);		/* set logic number */
//		edr |= USB_EP_Phy(ep_num);		/* set logic number */
//		edr |= USB_EP_Alt(ep_num);		/* set logic number */
		/* FIXME: some interface values is not set */

		writel(readl(__reg + USB_IER) | USB_PEP_Tran(ix_get_ep_num(ep)),
		   __reg + USB_IER);
	}

	/* set maxpacket */
	edr |= USB_MaxPacket(ep->maxpacket);

	dprintk(1, "write desc to ep%d, desc=%x\n", ep_num, edr);
	/* write desc */
	writel(edr, __reg + USB_EDR(ep_num));

	local_irq_restore(flags);
	return 0;
}

static int ix_ep_disable(struct usb_ep *ep)
{
	unsigned long flags;

	printk("ix_ep_disable: %s\n", __func__);
	if (!ep /* || !ep->desc */) {
		dprintk(DEBUG_NORMAL, "%s not enabled\n",
			ep ? ep->name : NULL);
		return -EINVAL;
	}

	local_irq_save(flags);

	if(ep == &ix_udc.ep[0]) {
		/* clear all ep0 status bits */
		writel(readl(__reg + USB_BFCR) | USB_Flush_TXB(0),
		   __reg + USB_BFCR);
		/* disable ep0 interrupts */
//		writel(readl(__reg + USB_IDR) | USB_EP0,
//		   __reg + USB_IDR);
	} else {
		writel(readl(__reg + USB_BFCR) | USB_Flush_TXB(1),
		   __reg + USB_BFCR);
//		writel(readl(__reg + USB_IDR) | USB_PEP_Tran(ix_get_ep_num(ep)),
//		   __reg + USB_IDR);
	}

	dprintk(1, "ep_disable: %s\n", ep->name);
	__req_nuke(ep, -ESHUTDOWN);

	local_irq_restore(flags);
	dprintk(1, "%s disabled\n", ep->name);

	return 0;
}

static struct usb_request *
ix_alloc_request(struct usb_ep *ep, gfp_t mem_flags)
{
	struct usb_request *req;

	dprintk(DEBUG_VERBOSE,"%s(%p,%d)\n", __func__, ep, mem_flags);

	if(!ep)
	  return NULL;

	req = kzalloc(sizeof(struct usb_request), mem_flags);
	if(!req)
	  return NULL;

	return req;
}

static void
ix_free_request(struct usb_ep *ep, struct usb_request *req)
{
	dprintk(DEBUG_VERBOSE, "%s(%p,%p)\n", __func__, ep, req);

	if (!ep || !req)
	  return ;

	kfree(req);
}

static int ix_queue(struct usb_ep *ep, struct usb_request *req,
   gfp_t gfp_flags)
{
	unsigned long flags;
	struct usb_request *p;

	dprintk(0, "udc: %s, ep=%d\n", __func__, ix_get_ep_num(ep));
	if(unlikely(!ep)) {
		dprintk(DEBUG_NORMAL, "%s: invalid args\n", __func__);
		return -EINVAL;
	}

	if(unlikely(!ix_udc.driver
		   || ix_udc.gadget.speed == USB_SPEED_UNKNOWN)) {
		return -ESHUTDOWN;
	}

	local_irq_save(flags);

	list_for_each_entry(p, ix_find_queue(ep), list)
	{
		if(p == req)
		{
			dprintk(4, "req: %p already in queue.\n", req);
			goto __exit_queue__;
		}
	}

	if (unlikely(!req || !req->complete
			|| !req->buf /*|| !list_empty(&req->queue)*/)) {
		if (!req)
			dprintk(DEBUG_NORMAL, "%s: 1 X X X\n", __func__);
		else {
			dprintk(DEBUG_NORMAL, "%s: 0 %01d %01d %01d\n",
				__func__, !req->complete,!req->buf, 0);
		}

		local_irq_restore(flags);
		return -EINVAL;
	}

	req->status = -EINPROGRESS;
	req->actual = 0;

	dprintk(DEBUG_VERBOSE, "%s: len %d\n",
		 __func__, req->length);

#if 0
	if(ep->bEndpointAddress) {
		/* Registery operations */
	} else
	{
	}
#endif

	/* pio or dma irq handler advances the queue. */
	if(likely(req))
	{
		struct list_head *queue = ix_find_queue(ep);
		if(queue){
		  list_add_tail(&req->list, queue);
		}else
		{
			printk(KERN_ERR "Can not find queue for ep %p\n", ep);
			return -EINVAL;
		}
	}

__exit_queue__:

	local_irq_restore(flags);

	dprintk(0, "%s ok\n", __func__);
	return 0;
}

static int ix_dequeue(struct usb_ep *ep, struct usb_request *req)
{
	struct usb_request *p;
	struct list_head *queue = ix_find_queue(ep);
	unsigned long flags;
	int ret;

	dprintk(DEBUG_VERBOSE, "%s(%p,%p)\n", __func__, ep, req);

	if(!queue)
	{
		printk(KERN_ERR "Can not find queue for ep %p\n", ep);
		return -EINVAL;
	}

	if(!ix_udc.driver)
	  return -ESHUTDOWN;

	if(!ep || !req)
	  return -EINVAL;

	local_irq_save(flags);

	list_for_each_entry(p, ix_find_queue(ep), list)
	{
		if(p == req) {
			list_del_init(&p->list);
			req->status = -ECONNRESET;
			ret = 0;
			break;
		}
	}

	if(!ret) {
		dprintk(DEBUG_VERBOSE,
			"dequeued req %p from %s, len %d buf %p\n",
			req, ep->name, req->length, req->buf);

		__req_done(ep, req, -ECONNRESET); //TODO
	}

	local_irq_restore(flags);
	return ret;
}

static int ix_set_halt(struct usb_ep *ep, int value)
{
	unsigned long flags;
	int ep_num = ix_get_ep_num(ep);

	dprintk(1, "udc: %s\n", __func__);
	if(unlikely(!ep)) {
		dprintk(DEBUG_NORMAL, "%s: inval 2\n", __func__);
		return -EINVAL;
	}

	local_irq_save(flags);

	writel(readl(__reg + USB_FHHR) | USB_PEP_HALT(ep_num),
	   __reg + USB_FHHR);

	local_irq_restore(flags);
	return 0;
}

/* gadget operations */
static int ix_get_frame(struct usb_gadget *gadget)
{
	dprintk(DEBUG_VERBOSE, "%s()\n", __func__);

	return readl(__reg + USB_FNCR) & 0x7ff;
}

static int ix_wakeup(struct usb_gadget *gadget)
{
	dprintk(DEBUG_NORMAL, "%s()\n", __func__);
	/* Not implemented in s3c */
	return 0;
}

static int ix_set_selfpowered(struct usb_gadget *gadget, int value)
{
	dprintk(DEBUG_NORMAL, "%s()\n", __func__);

	if(value)
	  ix_udc.devstat |=  (1 << USB_DEVICE_SELF_POWERED);
	else
	  ix_udc.devstat &= ~(1 << USB_DEVICE_SELF_POWERED);

	return 0;
}

static int ix_pullup(struct usb_gadget *gadget, int is_on)
{
	dprintk(DEBUG_NORMAL, "%s()\n", __func__);

#if 0
	if(xxx)
	  return -EOPNOTSUPP;
#endif

	if(is_on)
	{
		if(ix_udc.gadget.speed != USB_SPEED_UNKNOWN) {
			if(ix_udc.driver && ix_udc.driver->disconnect)
			  ix_udc.driver->disconnect(gadget);
		}
		/* Disable UDC */
		ix_udc_en(0);
	} else
	  ix_udc_en(1);

	return 0;
}

static int ix_vbus_session(struct usb_gadget *gadget, int is_active)
{
	dprintk(DEBUG_NORMAL, "%s()\n", __func__);

	ix_udc.vbus = !!is_active;
	ix_pullup(gadget, !is_active);
	return 0;
}

static int ix_vbus_draw(struct usb_gadget *gadget, unsigned ma)
{
	dprintk(1, "udc: %s\n", __func__);
	return -ENOTSUPP;
}

static void __config_setup(void)
{
	struct usb_ctrlrequest crq;
	int ret;

	dprintk(2, "%s()\n", __func__);

	crq.wValue = 0x01;
	crq.bRequest = 0x09;
	crq.bRequestType = 0x00;
	crq.wLength = 0x00;
	crq.wIndex = 0x00;

	ret = ix_udc.driver->setup(&ix_udc.gadget, &crq);
	if(ret < 0){
		if(ix_udc.req_config) {
			dprintk(DEBUG_NORMAL, "config change %02x fail %d?\n",
			   crq.bRequest, ret);
			return ;
		}

		if(ret == -EOPNOTSUPP)
			dprintk(DEBUG_NORMAL, "udc: Operation not supported\n");
		else
			dprintk(DEBUG_NORMAL,
				"dev->driver->setup failed. (%d)\n", ret);

		/* XXX */
		writel(USB_PEP_HALT(0), __reg + USB_FHHR);
		writel(USB_Flush_All, __reg + USB_BFCR);
	} else if (ix_udc.req_pending) {
		dprintk(0, "dev->req_pending... what now?\n");
		ix_udc.req_pending=0;
	}

	ix_udc.req_config = crq.wValue;

	return ;
}


/* interrupt handle */
static void __ep0_setup(void)
{
	struct usb_ctrlrequest crq;
	int ret;

	dprintk(1, "%s()\n", __func__);
	/* read crq from reg */                               
	*(((uint32_t *)&crq) + 0) = readl(__reg + USB_STR0);  
	*(((uint32_t *)&crq) + 1) = readl(__reg + USB_STR1);  

	dprintk(0, "r%x\n", crq.bRequest);
	dprintk(0, "r%xrt%xv%xl%xi%x\n", crq.bRequest, crq.bRequestType,
	   crq.wValue, crq.wLength, crq.wIndex);
	dprintk(1, "bRequestType=%x bRequeset=%x wValue=%x "
	   "wIndex=%x, wLength=%x\n",
	   crq.bRequestType, crq.bRequest, crq.wValue, crq.wIndex,
	   crq.wLength);

	/* cope with automagic for some standard requests */
	ix_udc.req_std = (crq.bRequestType & USB_TYPE_MASK)
		== USB_TYPE_STANDARD;
	ix_udc.req_config = 0;
	ix_udc.req_pending = 1;

#if 0
	if(crq.bRequestType & USB_DIR_IN)
	  ix_udc.ep0stat = EP0_IN_DATA_PHASE;
	else
	  ix_udc.ep0stat = EP0_OUT_DATA_PHASE;
#endif

	ret = ix_udc.driver->setup(&ix_udc.gadget, &crq);
	if(ret < 0){
		if(ix_udc.req_config) {
			dprintk(DEBUG_NORMAL, "config change %02x fail %d?\n",
			   crq.bRequest, ret);
			return ;
		}

		if(ret == -EOPNOTSUPP)
			dprintk(DEBUG_NORMAL, "udc: Operation not supported\n");
		else
			dprintk(DEBUG_NORMAL,
				"dev->driver->setup failed. (%d)\n", ret);

		/* XXX */
		writel(USB_PEP_HALT(0), __reg + USB_FHHR);
		writel(USB_Flush_All, __reg + USB_BFCR);
	} else if (ix_udc.req_pending) {
		dprintk(0, "dev->req_pending... what now?\n");
		ix_udc.req_pending=0;
	}

	switch(crq.bRequest) {
		case USB_REQ_SET_CONFIGURATION:
			printk(KERN_ERR "set configuration, not supported yet.\n");
			break;
		case USB_REQ_SET_INTERFACE:
			printk(KERN_ERR "set interface, not supported yet.\n");
			break;
		case USB_REQ_SET_ADDRESS:
			printk(KERN_ERR "set address, not supported yet\n");
			break;
		case USB_REQ_GET_STATUS:
			printk(KERN_ERR "get status, not supported yet\n");
			break;
		case USB_REQ_CLEAR_FEATURE:
			printk(KERN_ERR "clear feature, not supported yet\n");
			break;
		case USB_REQ_SET_FEATURE:
			printk(KERN_ERR "set feature, not supported yet\n");
			break;
		case USB_REQ_GET_DESCRIPTOR:
			dprintk(0, "get descripor\n");
			break;
		default:
			;
	}

	writel(0x400000, __reg + USB_EDR0);
	return ;
}

static void __req_done(struct usb_ep *ep,
   struct usb_request *req, int stat)
{
	list_del_init(&req->list);

	if (likely (req->status == -EINPROGRESS))
	  req->status = stat;
	  
	req->complete(ep, req);
}

static void __req_nuke(struct usb_ep *ep, int stat)
{
	struct list_head *list = ix_find_queue(ep);

	if(!list)
	  return ;

	while (!list_empty (list)) {
		struct usb_request *req;
		req = list_entry (list->next, struct usb_request,
		   list);
		__req_done(ep, req, stat);
	}
}

static int __ep_rx(int ep_num)
{
	struct usb_ep *ep = &ix_udc.ep[ep_num];
	struct list_head *list = ix_find_queue(ep);
	struct usb_request *req;
	uint8_t *buf;
	uint32_t size, acr, max_fifo;

	dprintk(0, "%s() ep%d\n", __func__,ep_num);
	if(unlikely(list_empty(list)))
	  req = NULL;
	else
	  req = list_entry(list->next, struct usb_request, list);

	if(!req)
	{
		dprintk(1, "Rx: no request in queue\n");
		return 1;
	}
	/* begin transfer */
	max_fifo = readl(__reg + USB_PRIR) & USB_ReqCnt_MSK;
	buf = req->buf + req->actual;
	size = min(req->length - req->actual, max_fifo);
	req->dma = dma_map_single(NULL, (void *)d_buffer, size, DMA_FROM_DEVICE);

	dprintk(0, "Rx, size=%x, max=%x, ep=%d, PRIR=%08x, req->length = 0x%x, req->actual = 0x%x\n",
	   size, max_fifo, ep_num, readl(__reg + USB_PRIR),req->length,req->actual);
	writel(req->dma, __reg + USB_MDAR);
	writel(USB_ReqLength(size) /*| USB_TxBuffer(0) | USB_IN_Prebuffer */
	   | USB_DMA_RxEn | USB_QueryACK, __reg + USB_ACR);

	/* wait DMA finish */
	while(1) {
		acr = readl(__reg + USB_ACR);
		if(acr & USB_ReqError)
		{
			dprintk(1, "udc detect: rx dma error.\n");
			/* clear error state */
			writel(acr | USB_ReqError, __reg + USB_ACR);
			writel(readl(__reg + USB_FHHR) |
			   USB_PEP_HALT(ep_num), __reg + USB_FHHR);
			writel(readl(__reg + USB_BFCR) | 0x1, __reg + USB_BFCR);
			goto __exit_rx__;
		} else if(!(acr & USB_DMA_RxEn))
		{
			dprintk(0, "udc detect: dma finished.\n");
			break;
		}
	}

	dprintk(0, "Rx, size=%x, max=%x, ep=%d, PRIR=%08x\n",
	   size, max_fifo, ep_num, readl(__reg + USB_PRIR));

	memcpy(buf, d_buffer, size);
	req->actual += size;

	dprintk(0, "udc_rx: size=%x, act=%x, len=%x\n",
	   size, req->actual, req->length);

	if(0){
		int i;
		char descx[1024];
		for (i = 0; i < size; i++)
		  sprintf(descx + 3 * i, "%02x ", d_buffer[i]);

		dprintk(1, "desc->[%s]\n", descx);
	}

    dprintk(0, "rx size %d actual %d len %d max %d\n", size,req->actual,req->length,ep->maxpacket);
	/* transfer finished */
	if((size != ep->maxpacket) || (req->actual == req->length))
	  __req_done(ep, req, 0);

__exit_rx__:
	dma_unmap_single(NULL, req->dma, size, DMA_FROM_DEVICE);
	return 0;

}
static int __ep_tx(int ep_num)
{
	struct usb_ep *ep = &ix_udc.ep[ep_num];
	const struct usb_endpoint_descriptor *desc = ix_udc.desc[ep_num];
	struct list_head *list = ix_find_queue(ep);
	struct usb_request *req;
	uint8_t *buf;
	uint32_t size, acr;

	dprintk(0, "%s(),e%d, reg %p, list %p\n",
	   __func__, ep_num, __reg, list);
	/* get ep status */
	if(readl(__reg + USB_EDR(ep_num)) & USB_EP_HALT)
	{
		dprintk(1, "udc: ep%d halted. nuke queue.\n", ep_num);
		__req_nuke(ep, 0);
	}

	dprintk(0, "a???\n");
	if(unlikely(list_empty(list)))
	  req = NULL;
	else
	  req = list_entry(list->next, struct usb_request, list);

	if(!req)
	{
		dprintk(0, "Tx: no request in queue\n");
		return 1;
	}

	dprintk(0, "req=%p, d_buffer=%p\n", req, d_buffer);
	/* begin transfer */
	buf = req->buf + req->actual;
	size = min((uint32_t)(req->length - req->actual),
	   (ep?EP_MAXPACKET:EP0_MAXPACKET));
	memcpy(d_buffer, buf, size);

#if 0
	size = 0x12;
	*(uint32_t *)(d_buffer + 0x00) = 0x02000112;
	*(uint32_t *)(d_buffer + 0x04) = 0x400000FF;
	*(uint32_t *)(d_buffer + 0x08) = 0x12345345;
	*(uint32_t *)(d_buffer + 0x0c) = 0x00000000;
	*(uint32_t *)(d_buffer + 0x10) = 0x00000100;
	*(uint32_t *)(d_buffer + 0x14) = 0x00000000;
	*(uint32_t *)(d_buffer + 0x18) = 0x00000000;
	*(uint32_t *)(d_buffer + 0x1c) = 0x0110060a;
	*(uint32_t *)(d_buffer + 0x20) = 0x08000000;
	*(uint32_t *)(d_buffer + 0x24) = 0xbeaf0001;
	*(uint32_t *)(d_buffer + 0x28) = 0x00010090;
	*(uint32_t *)(d_buffer + 0x2c) = 0x00000100;
	*(uint32_t *)(d_buffer + 0x30) = 0x00000000;
	*(uint32_t *)(d_buffer + 0x34) = 0x00000000;
	*(uint32_t *)(d_buffer + 0x38) = 0x00000000;
#endif

	dprintk(0, "Tx, len:%x act:%x, max:%x, size=%x, ep=%d\n",
	   req->length, req->actual,
	   (ep?EP_MAXPACKET:EP0_MAXPACKET),
	   size, ep_num);
	
	if(0){
		int i;
		char descx[1024];
		for (i = 0; i < size; i++)
		  sprintf(descx + 3 * i, "%02x ", d_buffer[i]);

		dprintk(1, "desc->[%s]\n", descx);
	}


	dprintk(0, "%s()3\n", __func__);
	req->dma = dma_map_single(NULL, (void *)d_buffer, size, DMA_TO_DEVICE);

	dprintk(0, "size = 0x%x, req=%p, req.buf=%p, req.dma=%x, req.length=%x, req.actual=%x\n",
	   	size, req, req->buf, req->dma, req->length, req->actual);
	writel(req->dma, __reg + USB_MDAR);

	acr = USB_ReqLength(size) | USB_QueryACK | USB_DMA_TxEn | USB_TxBuffer(ep_num);
	dprintk(0, "enabling dma, acr=%x\n", acr);
	writel(acr, __reg + USB_ACR);

//	dprintreg();
	dprintk(0, "%s()4\n", __func__);
	/* wait DMA finish */
	while(1) {
		acr = readl(__reg + USB_ACR);
		if(acr & USB_ReqError)
		{
			dprintk(1, "udc detect: dma error.\n");
			/* clear error state */
			writel(acr | USB_ReqError, __reg + USB_ACR);
			writel(readl(__reg + USB_FHHR) |
			   USB_PEP_HALT(ep_num), __reg + USB_FHHR);
			writel(readl(__reg + USB_BFCR) | 0x1, __reg + USB_BFCR);
			goto __exit_tx__;
		} else if(!(acr & 0x830))
		{
			dprintk(0, "udc detect: dma finished.\n");
			break;
		}
	}

//	writel(1, __reg + USB_BFCR);

	req->actual += size;

	/* transfer finished */
	if((req->zero && !size) ||
	   (size != ep->maxpacket) ||
	   (!req->zero && (req->actual == req->length)))
	  __req_done(ep, req, 0);

__exit_tx__:
	dma_unmap_single(NULL, req->dma, size, DMA_TO_DEVICE);
	dprintk(0, "__ep_tx exit\n");
	
	return 0;
}

static void __b_connect(void)
{
	while(!(readl(__reg + USB_BCSR) & USB_B_Valid));
	dprintk(0, "b device votage OK\n");
	writel(readl(__reg + USB_BCWR) | USB_B_Connect,
	   __reg + USB_BCWR);
	while((readl(__reg + USB_BCSR) & USB_DRDB_CS) != 0x10);
	dprintk(0, "connected with host\n");
	writel(readl(__reg + USB_BCWR) & ~USB_B_Connect,
	   __reg + USB_BCWR);

	writel(0x00000000, __reg + USB_TBCR0);
	writel(0x00000010, __reg + USB_TBCR1);
	writel(0x00000110, __reg + USB_TBCR2);
	writel(0x00000190, __reg + USB_TBCR3);

	writel(0x00000003, __reg + USB_ACR);
	writel(0x030f7c1f, __reg + USB_IER);
	//writel(0x030f381f, __reg + USB_IER);
	writel(0x0, __reg + USB_FNCR);
	//writel(0x20000000, __reg + USB_FNCR);
	//msleep(100);
	writel(readl(__reg + USB_UDCR) | 0x00070001,
	   __reg + USB_UDCR);

}

static void ix_handle_ep0(uint32_t isr)
{
	if(isr & USB_EP0_Setup) {
		__ep0_setup();
		writel(USB_EP0_Setup, __reg + USB_ISR);
	}

	if(isr & USB_EP0_OUT) {
		__ep_rx(0);
		writel(USB_EP0_OUT, __reg + USB_ISR);
	}

	if(isr & USB_EP0_IN) {
		__ep_tx(0);
		writel(USB_EP0_IN, __reg + USB_ISR);
	}

	if(isr & USB_EP0_Query)
		writel(USB_EP0_Query, __reg + USB_ISR);
	  /* none */;

	return ;
}

static int ix_handle_ep(int ep_num)
{
	uint32_t fhhr = readl(__reg + USB_FHHR);
	const struct usb_endpoint_descriptor *desc =
		ix_udc.desc[ep_num];

	dprintk(0, "%s(), ep%d addr=0x%x\n", __func__, ep_num,desc->bEndpointAddress);
	if(desc->bEndpointAddress & USB_DIR_IN)
	{
		/* the stall handshake from host */
		if (fhhr & USB_PEP_HALT(ep_num))
		  /* clear stall */
		  writel(fhhr | USB_PEP_HALT(ep_num), __reg + USB_FHHR);
		return __ep_tx(ep_num);                                                             
	}                                                                                         
	else {                                                                                    
		/* the stall handshake from host */
		if (fhhr & USB_PEP_HALT(ep_num))
		  /* clear stall */
		  writel(fhhr | USB_PEP_HALT(ep_num), __reg + USB_FHHR);
		return __ep_rx(ep_num);
	}                                                                                         

	return 0;
}


static int handle_set_configure()
{
    struct usb_ctrlrequest crq;
    uint32_t ccr;

    ccr=readl(__reg + USB_CCR);
    dprintk(0, "ccr 0x%x\n",ccr);
    if(ccr&0xf00){
        crq.bRequestType=0;
        crq.bRequest=0x09;
        crq.wValue=(ccr>>8)&0xf;
        crq.wIndex=0;
        crq.wLength=0;
        ix_udc.driver->setup(&ix_udc.gadget, &crq);
        writel(USB_SOF, __reg + USB_IDR);
        return 0;
    }

    return -1; 
}

static unsigned int ep1_count = 0;

static irqreturn_t ix_udc_irq(int dummy, void *dev)
{

	struct ix200_udc *udc = dev;
	unsigned long		flags;
	/*
	uint32_t isr = readl(__reg + USB_ISR);

    spin_lock_irqsave (&udc->lock, flags);
    writel(isr,__reg + USB_ISR);
    udc->isr=isr;
    spin_unlock_irqrestore (&udc->lock, flags);
    */

#if 1
	uint32_t isr = readl(__reg + USB_ISR);
	uint32_t ccr;
	//unsigned long flags;

    //writel(isr,__reg + USB_ISR);

	//dprintk(1, "udc irq. 0x%08x\n", isr);
	if(!isr)
	{
		dprintk(1, "no irq stat detected.\n");
		return IRQ_HANDLED;
	}

	spin_lock_irqsave(&udc->lock, flags);
	/* Driver connected ? */
	if(!udc->driver) {
		/* simply clear interrupt */
		dprintk(1, "no driver.\n");
		writel(isr, __reg + USB_ISR);
		goto __exit_irq__;
	}

#if 0
	if((isr & USB_SOF) || (isr & USB_DMA_Done))
	{
		writel(isr, __reg + USB_ISR);
		dprintk(0, "sof event received\n");
	}
#endif

	/* connect */
	if(isr & USB_Connect) {
		writel(USB_Connect, __reg + USB_ISR);
		dprintk(1, "usb connect\n");
	}

	/* disconnect */
	if(isr & USB_Disconnect) {
		writel(USB_Disconnect, __reg + USB_ISR);
		dprintk(1, "usb disconnect\n");
		writel(USB_SOF, __reg + USB_IDR);
	}

	/* reset */
	if(isr & USB_Reset) {

		writel(USB_Reset, __reg + USB_ISR);
		if(readl(__reg + USB_UDCR) & USB_DSI)	// different from csl
		  udc->gadget.speed = USB_SPEED_HIGH;
		else
		  udc->gadget.speed = USB_SPEED_FULL;

		dprintk(1, "usb reset, speed=%d\n", udc->gadget.speed);
		writel(USB_SOF, __reg + USB_IER);    //open sof 
		writel(USB_SOF, __reg + USB_ISR);
	}

	/* suspend */
	if(isr & USB_Suspend) {
		writel(USB_Suspend, __reg + USB_ISR);
		dprintk(1, "usb suspend , isr = 0x%x\n",isr);
		udc->configured=0;
		
		// 拔掉otg线后，会导致系统僵死（jtag连上之后程序跑飞），但是otg线插上后，系统又能正常运行。
		//writel(USB_SOF, __reg + USB_IDR);
	}

	/* resume */
	if(isr & USB_DMA_Done) {
		writel(USB_DMA_Done, __reg + USB_ISR);
		dprintk(0, "usb dma, isr = 0x%x\n",isr);
	}	

    if(isr & USB_SOF){
        dprintk(0, "USB_SOF %d\n",udc->configured);
        if(!udc->configured){
            if(!handle_set_configure()){
                udc->configured=1;   
                writel(USB_SOF, __reg + USB_IDR);
            }
        }
        writel(USB_SOF, __reg + USB_ISR);
    }	
	
	/* ep0 */
	if(isr & USB_EP0)
	{
		ix_handle_ep0(isr);
		//writel(readl(__reg + USB_IER) | (0x3 << 24), __reg + USB_IER);
		//printk("USB_IER : 0x%x\r\n",readl(__reg + USB_IER));
	}

	/* eps */
	if(isr & USB_PEP)
	{
		int i;
		if(isr & 0x2000000){
			writel(0x2000000, __reg + USB_IDR);
			//printk("ix_udc_irq : ep1 is disabled 0x%x\r\n",__reg + USB_IDR);
		}
	
		
		for(i = 1; i < IX_EP_COUNT; i++) {
			if(isr & USB_PEP_Tran(i)){
				ix_handle_ep(i);
				writel(USB_PEP_Tran(i), __reg + USB_ISR);
			}
		}
	}

	/* interface */
	if(isr & USB_Set_Interface)
	{
		writel(USB_Set_Interface, __reg + USB_ISR);
		__config_setup();
	}
	//else

	/* clear other interrupts */
	  //writel(isr, __reg + USB_ISR);
	dprintk(0, "irq done, isr: 0x%08x, ier: 0x%08x\n",
	   isr, readl(__reg + USB_IER));

__exit_irq__:

	spin_unlock_irqrestore(&udc->lock, flags);
#endif	
    //schedule_work(&udc->irq_work);

	return IRQ_HANDLED;
}


static void ix_udc_en(int en)
{
	if(en)
	{
		/* set phy interface */
		writel(0xff111100, __reg + USB_PIR0);
		writel(0xffffff21, __reg + USB_PIR1);

		/* dev->gadget.speed = USB_SPEED_UNKNOWN; */ 
		ix_udc.gadget.speed = USB_SPEED_FULL;          


		dprintk(0, "reg USB_BCWR: 0x%08x\n", readl(__reg + USB_BCWR));
		dprintk(0, "reg USB_BCIER: 0x%08x\n", readl(__reg + USB_BCIER));
		dprintk(0, "reg USB_BCIDR: 0x%08x\n", readl(__reg + USB_BCIDR));
		dprintk(0, "reg USB_BCISR: 0x%08x\n", readl(__reg + USB_BCISR));
		dprintk(0, "reg USB_TBCR0: 0x%08x\n", readl(__reg + USB_TBCR0));
		dprintk(0, "reg USB_TBCR1: 0x%08x\n", readl(__reg + USB_TBCR1));
		dprintk(0, "reg USB_TBCR2: 0x%08x\n", readl(__reg + USB_TBCR2));
		dprintk(0, "reg USB_TBCR3: 0x%08x\n", readl(__reg + USB_TBCR3));
		dprintk(0, "reg USB_TBCR0: 0x%08x\n", readl(__reg + USB_TBCR0));
		dprintk(0, "reg USB_ACR: 0x%08x\n", readl(__reg + USB_ACR));
		dprintk(0, "reg USB_IER: 0x%08x\n", readl(__reg + USB_IER));
	}

		dprintk(0, "rDIV_CFG2: 0x%08x\n", readl(rDIV_CFG2));
//		writel(readl(rSCLK_MASK) | 0x2030, rSCLK_MASK);
		dprintk(0, "rSCLK_MASK: 0x%08x\n", readl(rSCLK_MASK));

		
		writel(readl(rPAD_CFG) & ~0x8, rPAD_CFG);                 
		writel(0xc,rUSB_SRST);                
		udelay(100);                         
		writel(readl(rPAD_CFG) | 0x8, rPAD_CFG);                 
		mdelay(4);                        
		writel(0xd,rUSB_SRST);                
		udelay(200);                         
		writel(0xf,rUSB_SRST);                
		/*
		writel(readl(rPAD_CFG) & ~0xe, rPAD_CFG);                 
		writel(0x0,rUSB_SRST);                
		udelay(100);                         
		writel(readl(rPAD_CFG) | 0xe, rPAD_CFG);                 
		mdelay(4);                        
		writel(0x5,rUSB_SRST);                
		udelay(200);                         
		writel(0xf,rUSB_SRST); 		
		*/
		
		dprintk(0, "rPAD_CFG: 0x%08x\n", readl(rPAD_CFG));
		dprintk(0, "rUSB_SRST: 0x%08x\n", readl(rUSB_SRST));

		/* reset */
		writel(0x00000000, __reg + USB_BCWR);
		writel(0x00000000, __reg + USB_BCIER);
		writel(0x00000000, __reg + USB_BCIDR);
		writel(0x0000003f, __reg + USB_BCISR);

		dprintk(1, "waiting connection\n");
		__b_connect();

	return ;
}

static void ix_ep_init(void)
{
	uint32_t i;

	/* init gadget ep_list */
	INIT_LIST_HEAD(&ix_udc.gadget.ep_list);
	INIT_LIST_HEAD(&ix_udc.gadget.ep0->ep_list);

	for(i = 0; i < IX_EP_COUNT; i++)
	{
		ix_udc.ep[i].ops = &ix_ep_ops;
		if(i)
		  list_add_tail(&ix_udc.ep[i].ep_list, &ix_udc.gadget.ep_list);

		/* Init EP queues */
		INIT_LIST_HEAD(&ix_udc.ep_req[i]);
	}
}

static struct clk *bus_clk;
static int ix_udc_probe(struct platform_device *pdev)
{
	struct ix200_udc *udc = &ix_udc;
	struct device *dev = &pdev->dev;
	struct resource *res, *area;
	int size, ret;

	printk("%s()\n", __func__);

	bus_clk = clk_get(NULL, "usb-bus-gadget");
	if(IS_ERR(udc->clk)) {
		dev_err(dev, "failed to get usb bus clock source\n");
		return PTR_ERR(bus_clk);
	}

	clk_enable(bus_clk);

	udc->clk = clk_get(NULL, "usb-device");
	if(IS_ERR(udc->clk)) {
		dev_err(dev, "failed to get udc clock source\n");
		return PTR_ERR(udc->clk);
	}

	clk_enable(udc->clk);

	mdelay(10);
	dprintk(DEBUG_NORMAL, "got and enabled clocks\n");

	spin_lock_init(&udc->lock);
	res = pdev->resource;
	size = res->end - res->start + 1;

#if 0
	area = request_mem_region(res->start, size, pdev->name);
	if(!area)
	{
		dev_err(&pdev->dev, "Can not reserve register region. start:%x, size:%x, name:%s\n",
		   res->start, size, pdev->name);
		return -ENOENT;
	}
#endif

	ix_udc.regbase = ioremap_nocache(res->start, size); 
	__reg = ix_udc.regbase; /* easy to use */

	if(!__reg)
	{
		dev_err(&pdev->dev, "Can not remap register address..\n");
		release_mem_region(res->start, size);
		return -EIO;
	}

	device_initialize(&udc->gadget.dev);
	udc->gadget.dev.parent = &pdev->dev;
	udc->gadget.dev.dma_mask = pdev->dev.dma_mask;

	/* allocate dma buffer */
	//d_buffer = kmalloc(4096, GFP_KERNEL);
	d_buffer = kmalloc(4096*4, GFP_KERNEL);
	if(!d_buffer)
	{
		dprintk(1, "allocate global buffer failed.\n");
		return -ENOMEM;
	}
	if(virt_addr_valid((uint32_t)d_buffer))
	  dprintk(1, "allocate valid global buffer %x.\n", d_buffer);
	else
	  dprintk(1, "allocated but not valid global buffer %x.\n", d_buffer);

	platform_set_drvdata(pdev, udc);


    //INIT_WORK(&udc->irq_work, udc_irq_work);

	udc->irqno = platform_get_irq(pdev, 0);
	dprintk(DEBUG_NORMAL, "udc: irq no. %d\n", udc->irqno);
	ret = request_irq(udc->irqno, ix_udc_irq, IRQF_DISABLED, pdev->name, udc);
	if(ret)
	{
		dev_err(dev, "cannot get irq %i, err %d\n", udc->irqno, ret);
		iounmap(udc->regbase);
		release_mem_region(res->start, size);
		return ret;
	}

	//ix_udc_en(0);
//	dprintk(1, "probe ok\n");
	ix_ep_init();
	dprintk(1, "begin_en\n");
	//ix_udc_en(1);
	dprintk(1, "probe ok\n");
	return 0;
}

static int ix_udc_remove(struct platform_device *pdev)
{
	struct ix200_udc *udc = platform_get_drvdata(pdev);
	struct resource *res;

	dev_dbg(&pdev->dev, "%s()\n", __func__);

	res = pdev->resource;

	if(udc->driver)
	  return -EBUSY;

	free_irq(udc->irqno, udc);
	iounmap(__reg);
	release_mem_region(res->start, res->end - res->start + 1);

	platform_set_drvdata(pdev, NULL);

	if(!IS_ERR(udc->clk) && udc->clk) {
		clk_disable(udc->clk);
		clk_put(udc->clk);
		udc->clk = NULL;
	}

	if(!IS_ERR(bus_clk) && bus_clk) {
		clk_disable(bus_clk);
		clk_put(bus_clk);
		bus_clk = NULL;
	}

	dev_dbg(&pdev->dev, "%s: remove ok\n", __func__);
	return 0;
}

#ifdef CONFIG_PM
static int ix_udc_suspend(
   struct platform_device *pdev, pm_message_t message)
{
	return 0;
}

static int ix_udc_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define ix_udc_suspend NULL
#define ix_udc_resume NULL
#endif

static struct platform_driver ix_udc_driver = {
	.driver		= {
		.name	= "ix-udc",
		.owner	= THIS_MODULE,
	},
	.probe		= ix_udc_probe,
	.remove		= ix_udc_remove,
	.suspend	= ix_udc_suspend,
	.resume		= ix_udc_resume,
};

static int __init ix_udc_init(void)
{
	printk(KERN_INFO "iMAPx200 USB Device controller (c) 2009~2014\n");

	return 
		platform_driver_register(&ix_udc_driver);
}

static void __exit ix_udc_exit(void)
{
	platform_driver_unregister(&ix_udc_driver);
}

module_init(ix_udc_init);
module_exit(ix_udc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("warits <warits.wang@infotmic.com.cn>");
MODULE_DESCRIPTION("UDC driver for iMAPx200");
MODULE_VERSION("v0");
