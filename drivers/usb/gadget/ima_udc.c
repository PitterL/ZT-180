/***************************************************************************** 
** drivers/usb/gadget/ima_udc.c
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
#define DEBUG

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/ioport.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/kthread.h>
#include <linux/init.h>
#include <linux/timer.h>
#include <linux/freezer.h>
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
#include <mach/debug.h>
#include "ima_udc.h"

#define ASYNC_HANDLE_PHY_EVENT
#define ASYNC_SET_CONFIG
//#define CACHE_PIPE_DATA

static int nuke(struct ima_ep_struct *ima_ep, int status);
static void done(struct ima_ep_struct *ima_ep,
					struct ima_request *req, int status);
static void ep0_chg_stat(const char *label,
			struct ima_udc_struct *ima_usb, enum ep0_state stat);
static void ep0_set_stat(const char *label,
            struct ima_udc_struct *ima_usb, enum ep0_state stat);
static void ep0_clr_stat(const char *label,
            struct ima_udc_struct *ima_usb, enum ep0_state stat);
static int ep0_get_clr_stat(struct ima_udc_struct *ima_usb,int stat_clr);
static int ep0_test_stat(struct ima_udc_struct *ima_usb,int stat_test);

static void ima_udc_reset(struct ima_udc_struct *ima_usb);
static void udc_en(struct ima_udc_struct *ima_usb,int enable);
static void ep_events_sync(struct ima_ep_struct  *ima_ep,int ep0state);
static void handle_config(unsigned long data);
//static void handle_adb_data(unsigned long data);


static const char driver_name[] = "iMAPx200-UDC";
static const char ep0name[] = "ep0";

static const struct usb_ep_ops ima_ep_ops = {
	.enable			= ima_ep_enable,
	.disable		= ima_ep_disable,
	.alloc_request	= ima_ep_alloc_request,
	.free_request	= ima_ep_free_request,
	.queue			= ima_ep_queue,
	.dequeue		= ima_ep_dequeue,
	.set_halt		= ima_set_halt,
};

static const struct usb_gadget_ops ima_gadget_ops = {
	.get_frame			= ima_get_frame,
	.wakeup				= ima_wakeup,
	.set_selfpowered	= ima_set_selfpowered,
	.pullup				= ima_pullup,
	.vbus_session		= ima_vbus_session,
	.vbus_draw			= ima_vbus_draw,
};

static struct ima_udc_struct controller = {
	.gadget = {
	    .is_otg		= 0,
		.ops		= &ima_gadget_ops,
		.ep0		= &controller.ima_ep[0].ep,
		.name		= driver_name,
		.dev = {
			.init_name	= "gadget",
		},
	},

    //see ep_matches()
	.ima_ep[0] = {
		.ep = {
			.name		= ep0name,
			.ops		= &ima_ep_ops,
			.maxpacket	= 64,
		},
		.ima_usb		= &controller,
		.descriptor ={
		    .bLength            = USB_DT_ENDPOINT_SIZE,
		    .bDescriptorType    = USB_DT_ENDPOINT,
		    .bEndpointAddress   = 0,
		    .bmAttributes       = USB_ENDPOINT_XFER_CONTROL,
		    .wMaxPacketSize     = EP0_MAXPACKET,
		    .bInterval          = 1,        
		}
	 },
	.ima_ep[1] = {
		.ep = {
			.name		= "ep1in-bulk",
			.ops		= &ima_ep_ops,
			.maxpacket	= 512,
		},
		.ima_usb		= &controller,
		.descriptor ={
		    .bLength            = USB_DT_ENDPOINT_SIZE,
		    .bDescriptorType    = USB_DT_ENDPOINT,
		    .bEndpointAddress   = 0x81,
		    .bmAttributes       = USB_ENDPOINT_XFER_BULK,
		    .wMaxPacketSize     = EP_MAXPACKET,
		    .bInterval          = 4,        
		}		
	 },
	.ima_ep[2] = {
		.ep = {
			.name		= "ep2out-bulk",
			.ops		= &ima_ep_ops,
			.maxpacket	= 512,
		},
		.ima_usb		= &controller,
		.descriptor ={
		    .bLength            = USB_DT_ENDPOINT_SIZE,
		    .bDescriptorType    = USB_DT_ENDPOINT,
		    .bEndpointAddress   = 0x2,
		    .bmAttributes       = USB_ENDPOINT_XFER_BULK,
		    .wMaxPacketSize     = EP_MAXPACKET,
		    .bInterval          = 4,        
		}		
	 },
	.ima_ep[3] = {
		.ep = {
			.name		= "ep3in-bulk",
			.ops		= &ima_ep_ops,
			.maxpacket	= 512,
		},
		.ima_usb		= &controller,
		.descriptor ={
		    .bLength            = USB_DT_ENDPOINT_SIZE,
		    .bDescriptorType    = USB_DT_ENDPOINT,
		    .bEndpointAddress   = 0x83,
		    .bmAttributes       = USB_ENDPOINT_XFER_BULK,
		    .wMaxPacketSize     = EP_MAXPACKET,
		    .bInterval          = 4,        
		}		
	 },
	.ima_ep[4] = {
		.ep = {
			.name		= "ep4out-bulk",
			.ops		= &ima_ep_ops,
			.maxpacket	= 512,
		 },
		.ima_usb		= &controller,
		.descriptor ={
		    .bLength            = USB_DT_ENDPOINT_SIZE,
		    .bDescriptorType    = USB_DT_ENDPOINT,
		    .bEndpointAddress   = 0x4,
		    .bmAttributes       = USB_ENDPOINT_XFER_BULK,
		    .wMaxPacketSize     = EP_MAXPACKET,
		    .bInterval          = 4,        
		}		
	 },
	.ima_ep[5] = {
		.ep = {
			.name		= "ep5",
			.ops		= &ima_ep_ops,
			.maxpacket	= 512,
		},
		.ima_usb		= &controller,
	 },
	.ima_ep[6] = {
		.ep = {
			.name		= "ep6",
			.ops		= &ima_ep_ops,
			.maxpacket	= 512,
		},
		.ima_usb		= &controller,
	 },	 
};

static void dump_ep_descriptor(const struct usb_endpoint_descriptor *desc)
{
    D_DBG("bLength %d type 0x%x address 0x%x attr 0x%x max_pkt 0x%x interval 0x%x\n",
        desc->bLength,
        desc->bDescriptorType,
        desc->bEndpointAddress,
        desc->bmAttributes,
        desc->wMaxPacketSize,
        desc->bInterval);
}

inline uint8_t * get_cache(struct ima_udc_struct *ima_usb,int ep_num)
{
    uint8_t * buffer;
    if(ep_num==0){
        buffer=ima_usb->d_buffer;
    }else{
        buffer=ima_usb->d_buffer + EP0_MAXPACKET + (ep_num-1)*EP_MAXPACKET;
    }

    return buffer;
}

inline int device_active(struct ima_udc_struct *ima_usb)
{
    return (int)!!ima_usb->active_config;
}

void ep_pipe_ready(struct ima_udc_struct *ima_usb,int number){
    if(number<IMA_EP_COUNT){
      #if defined(DEBUG)
        D_DBG("%ld:ep %d interval=%ld\n",jiffies,number,jiffies-ima_usb->ima_ep[number].jiffies_token);
        ima_usb->ima_ep[number].jiffies_token=jiffies;
        ima_usb->ima_ep[number].jiffies_irq = jiffies;
      #endif
        complete(&ima_usb->ima_ep[number].pipe_ready);
    }
}

void ep_data_ready(struct ima_udc_struct *ima_usb,int number){
    if(number<IMA_EP_COUNT){
      /*#if defined(DEBUG)
        ima_usb->ima_ep[number].jiffies_req = jiffies;
      #endif*/
        wake_up(&ima_usb->ima_ep[number].data_ready);
    }
}

void force_phy_suspend(struct ima_udc_struct *ima_usb,int suspend)
{
    if(suspend){
        writel(readl(Reg(USB_ACR))|(1<<bsACR_FORCE_PHY_SUSPEND), 
            Reg(USB_ACR));
    }else{
        writel(readl(Reg(USB_ACR))&~(1<<bsACR_FORCE_PHY_SUSPEND), 
            Reg(USB_ACR));
    }
}

void force_phy_disconnect(struct ima_udc_struct *ima_usb,int disconnect)
{
    uint32_t val;
        
    val = readl(Reg(USB_UDCR));
    D_TRACE("%s UDCR 0x%x %d\n",__func__,val,disconnect);

    if(disconnect){
        writel(val|UDC_SOFT_DISCONNECT,
            Reg(USB_UDCR));
    }else{
        writel(val|UDC_SOFT_CONNECT, 
            Reg(USB_UDCR));
    }
}

inline void clr_setup_irq(struct ima_udc_struct *ima_usb)
{
    writel(IRQ_CONTROL_SETUP, Reg(USB_ISR));
    writel(IRQ_CONTROL_SETUP, Reg(USB_IER));
}

inline void clr_epx_irq(struct ima_udc_struct *ima_usb,int ep_num)
{
    uint32_t ier;
    
    if(ep_num == 0){
        ier = IRQ_CONTROL_IN|IRQ_CONTROL_OUT;
    }else{
        ier = 1<<(IRQ_EPx_SHIFT + ep_num -1);
    }
    writel(ier,Reg(USB_ISR));
}

inline void clr_enable_epx_irq(struct ima_udc_struct *ima_usb,int ep_num)
{
    uint32_t ier;
    
    if(ep_num == 0){
        ier = IRQ_CONTROL_IN|IRQ_CONTROL_OUT;
    }else{
        ier = 1<<(IRQ_EPx_SHIFT + ep_num -1);
    }

    writel(ier,Reg(USB_ISR));
    writel(ier,Reg(USB_IER));
}

inline void disable_epx_irq(struct ima_udc_struct *ima_usb,int ep_num)
{
    uint32_t ier;
    
    if(ep_num == 0){
        ier = IRQ_CONTROL_IN|IRQ_CONTROL_OUT;
    }else{
        ier = 1<<(IRQ_EPx_SHIFT + ep_num -1);
    }
    writel(ier,Reg(USB_IDR));
}

inline void enable_epx_irq(struct ima_udc_struct *ima_usb,int ep_num)
{
    uint32_t ier;
    
    if(ep_num == 0){
        ier = IRQ_CONTROL_IN|IRQ_CONTROL_OUT;
    }else{
        ier = 1<<(IRQ_EPx_SHIFT + ep_num -1);
    }
    writel(ier,Reg(USB_IER));
}

inline void test_clr_enable_sof_irq(struct ima_udc_struct *ima_usb)
{
    uint32_t ier;
    
    ier=readl(Reg(USB_IER));
    if(!(ier&IRQ_SOF)){
        writel(IRQ_SOF,Reg(USB_ISR));
        writel(IRQ_SOF,Reg(USB_IER));
    }
}

inline void disable_sof_irq(struct ima_udc_struct *ima_usb)
{
    writel(IRQ_SOF,Reg(USB_IDR));
}



inline void clr_enable_phy_irq(struct ima_udc_struct *ima_usb)
{
    writel(IRQ_CONNECT|IRQ_DISCONNECT|IRQ_RESET|IRQ_SUSPEND|IRQ_RESUME|IRQ_SYNC_FRAME,Reg(USB_ISR));
    writel(IRQ_CONNECT|IRQ_DISCONNECT|IRQ_RESET|IRQ_SUSPEND|IRQ_RESUME|IRQ_SYNC_FRAME,Reg(USB_IER));
}

static void enable_basic_irq(struct ima_udc_struct *ima_usb)
{
    writel(IRQ_CONTROL_QUERY|
            IRQ_CONTROL_IN|
            IRQ_CONTROL_OUT|
            IRQ_CONTROL_SETUP|
            IRQ_DMA_DONE|
            IRQ_DMA_ERROR|
            IRQ_RESUME|
            IRQ_SUSPEND|
            IRQ_RESET|
            IRQ_DISCONNECT|
            IRQ_CONNECT,
        Reg(USB_IER));
}

uint32_t get_bus_event(struct ima_bus_event_struct * bus_event,int clear)
{
    uint32_t event;

    unsigned long flags;
    
    local_irq_save(flags);
    
    event = bus_event->event;
    if(clear)
        bus_event->event=0;

    local_irq_restore(flags);

    return event;
}

void set_bus_event(struct ima_bus_event_struct * bus_event,uint32_t event)
{
    unsigned long flags;
    
    local_irq_save(flags);
    
    bus_event->event|=event;

    local_irq_restore(flags);
}

void enum_set_config_data(struct usb_ctrlrequest *rq,uint16_t  configure)
{
    rq->bRequestType=0;
    rq->bRequest=USB_REQ_SET_CONFIGURATION;
    rq->wValue=configure;
    rq->wIndex=0;
    rq->wLength=0;

    D_DBG( "%s %d",__func__,configure);
}

void enum_adb_data0(struct usb_request *usb_req,uint32_t ep2_counter)
{
    uint8_t *data;

    int i=0;

    if(ep2_counter == 1){
        data=(uint8_t *)usb_req->buf;
        //68 6f 73 74 3a 3a 00;
        data[i++]='C';
        data[i++]='N';
        data[i++]='X';
        data[i++]='N';
        
        data[i++]=0x00;
        data[i++]=0x00;
        data[i++]=0x00;
        data[i++]=0x01;

        data[i++]=0x00;
        data[i++]=0x10;
        data[i++]=0x00;
        data[i++]=0x00;

        data[i++]=0x07;
        data[i++]=0x00;
        data[i++]=0x00;
        data[i++]=0x00;

        data[i++]=0x32;
        data[i++]=0x02;
        data[i++]=0x00;
        data[i++]=0x00;

        data[i++]=0xbc;
        data[i++]=0xb1;
        data[i++]=0xa7;
        data[i++]=0xb1;

        usb_req->actual = i;
        usb_req->status = 0;

    }else if(ep2_counter == 2){
        data=(uint8_t *)usb_req->buf;
        //68 6f 73 74 3a 3a 00;
        data[i++]='h';
        data[i++]='o';
        data[i++]='s';
        data[i++]='t';
        data[i++]=':';
        data[i++]=':';
        data[i++]='\0';

        usb_req->actual = i;
        usb_req->status = 0;
    }
}

static int get_ep_num(const struct usb_ep *ep)
{
    struct ima_udc_struct *ima_usb;
    struct ima_ep_struct  *ima_ep;

    int num;

    ima_ep=container_of(ep,struct ima_ep_struct,ep);
    ima_usb=ima_ep->ima_usb;

    num=ima_ep-&ima_usb->ima_ep[0];

    if(num >= IMA_EP_COUNT || num<0){
        D_DBG( "num = %d ima_ep = %p\n", num ,&ima_usb->ima_ep[0]);
    }

    return num;
}

/*******************************************************************************
 * USB gadget callback functions
 *******************************************************************************
 */
 
static int ima_ep_enable(struct usb_ep *ep, 
   const struct usb_endpoint_descriptor *desc)
{
	struct ima_ep_struct *ima_ep = container_of(ep,
						struct ima_ep_struct, ep);
	struct ima_udc_struct *ima_usb = ima_ep->ima_usb;

    int ep_num = get_ep_num(ep);
    uint32_t val;


    D_EPX(ima_usb->dev, "<%s> ep %d\n", __func__,ep_num);

	if (!ima_usb->driver || ima_usb->gadget.speed == USB_SPEED_UNKNOWN) {
		D_ERR(ima_usb->dev, "<%s> bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

    if(ima_ep->inited){
		D_ERR(ima_usb->dev, "<%s> ep %d is already enabled\n", __func__,ep_num);
		return 0;
    }

    if(!desc)
        desc = &ima_ep->descriptor;
    else    
        memcpy(&ima_ep->descriptor,desc,sizeof(struct usb_endpoint_descriptor));

	if(desc->bLength !=USB_DT_ENDPOINT_SIZE||
	    desc->bDescriptorType != USB_DT_ENDPOINT) {
			D_ERR(ima_usb->dev,
				"<%s> bad ep %d or descriptor\n", __func__,ep_num);
	    return -EINVAL;
	}

    clr_enable_epx_irq(ima_usb,ep_num);

	if(ep_num == 0){
        val=0;
        SET_BITS(val,EDR_TYPE,EDR_TYPE_CONTROL);
        SET_BITS(val,EDR_NO,0);
        SET_BITS(val,EDR_ALT_INTERFACE,0);
        SET_BITS(val,EDR_INTERFACE,0);
        SET_BITS(val,EDR_MAX_PKT,64);
        SET_BITS(val,EDR_BUFFER_SELECT,EP0_BUFFER_SEL);
        
        writel(val,Reg(USB_EDR0));
        
	}else{
        val=0;
        SET_BITS(val,EDR_INTERFACE,0);

        if(desc->bEndpointAddress & USB_DIR_IN){
            SET_BITS(val,EDR_DIR,EDR_DIR_IN);
            SET_BITS(val,EDR_BUFFER_SELECT,EPx_IN_BUFFER_SEL);
        }else{
            SET_BITS(val,EDR_DIR,EDR_DIR_OUT);
            SET_BITS(val,EDR_BUFFER_SELECT,EPx_OUT_BUFFER_SEL);
        }
	
		switch(desc->bmAttributes & USB_ENDPOINT_XFERTYPE_MASK)
		{
		    case USB_ENDPOINT_XFER_CONTROL:
				SET_BITS(val,EDR_TYPE,EDR_TYPE_CONTROL);
				break;
			case USB_ENDPOINT_XFER_BULK:
				SET_BITS(val,EDR_TYPE,EDR_TYPE_BULK);
				break;
			case USB_ENDPOINT_XFER_ISOC:
				SET_BITS(val,EDR_TYPE,EDR_TYPE_ISO);
				break;
			case USB_ENDPOINT_XFER_INT:
				SET_BITS(val,EDR_TYPE,EDR_TYPE_INTERRUPT);
				break;
			default:
			    D_ERR(ima_usb->dev, "<%s> ep%d unknow attr 0x%x\n", __func__,
			        ep_num,desc->bmAttributes);
		}

		SET_BITS(val,EDR_NO,desc->bEndpointAddress & ~USB_DIR_IN);
        SET_BITS(val,EDR_MAX_PKT,desc->wMaxPacketSize);
        writel(val,Reg(USB_EDRx + ep_num*4));

        //clr_enable_epx_irq(ima_usb,ep_num);

        /*D_EPX(ima_usb->dev,*/D_TRACE("<%s> ep%d address 0x%x %s enabled\n", __func__,
            ep_num,desc->bEndpointAddress,(desc->bEndpointAddress & USB_DIR_IN)?"IN":"OUT");
	}

    //clr_enable_epx_irq(ima_usb,ep_num);

    ima_ep->inited = 1;

    dump_ep_descriptor(&ima_ep->descriptor);

	return 0;
}

static int ima_ep_disable(struct usb_ep *ep)
{
	struct ima_ep_struct *ima_ep = container_of(ep,
						struct ima_ep_struct, ep);
	struct ima_udc_struct *ima_usb = ima_ep->ima_usb;
						
	uint32_t val;

    int pending;

    int ep_num = get_ep_num(ep);

    D_EPX(ima_usb->dev, "<%s> ep %d\n", __func__,ep_num);

	if (!ep || !ep_num ) {
		D_ERR(ima_ep->ima_usb->dev, "<%s> %s can not be disabled\n",
			__func__, ep ? ima_ep->ep.name : NULL);
		return -EINVAL;
	}
    	
	ima_ep->inited = 0;
	pending = nuke(ima_ep, -ESHUTDOWN);

	if (ima_usb->gadget.speed == USB_SPEED_UNKNOWN) {
		D_ERR(ima_usb->dev, "<%s> bogus device state\n", __func__);
		return -ESHUTDOWN;
	}

    disable_epx_irq(ima_usb,ep_num);
    clr_epx_irq(ima_usb,ep_num);

    if(pending){
        ep_data_ready(ima_usb,ep_num);  //release ep thread
    }
    //flush all
    if(ep_num){
        //disable ep int
        val = 1<<(IRQ_EPx_SHIFT + ep_num -1);
        writel(val,Reg(USB_IDR));

        if(ima_ep->descriptor.bEndpointAddress & USB_DIR_IN){
            val = 1<<BFCR_FLUSH_TXB;
            
        }else{
            val = 1<<BFCR_FLUSH_RXB;
        }
        writel(val,Reg(USB_BFCR));
    }

	/*D_EPX(ima_ep->ima_usb->dev,*/D_TRACE(
		"<%s> DISABLED %s\n", __func__, ep->name);
	return 0;
}

/*******************************************************************************
 * USB request control functions
 *******************************************************************************
 */

static void ep_add_request(struct ima_ep_struct *ima_ep,
							struct ima_request *req)
{
	if (unlikely(!req))
		return;

	req->in_use = 1;
	list_add_tail(&req->queue, &ima_ep->queue);
}

static void ep_del_request(struct ima_ep_struct *ima_ep,
							struct ima_request *req)
{
	if (unlikely(!req))
		return;

	req->in_use = 0;
}

static struct usb_request *ima_ep_alloc_request
                    (struct usb_ep *usb_ep, gfp_t gfp_flags)
{
    struct ima_request *req;

    if (!usb_ep)
        return NULL;

    req = kzalloc(sizeof *req, gfp_flags);
    if (!req)
        return NULL;

    INIT_LIST_HEAD(&req->queue);
    req->in_use = 0;

    return &req->req;
}

static void ima_ep_free_request
            (struct usb_ep *usb_ep, struct usb_request *usb_req)
{
    struct ima_request *req;

    req = container_of(usb_req, struct ima_request, req);
    WARN_ON(!list_empty(&req->queue));
    kfree(req);
}

static int ima_ep_queue
	(struct usb_ep *usb_ep, struct usb_request *usb_req, gfp_t gfp_flags)
{
	struct ima_ep_struct	*ima_ep;
	struct ima_udc_struct	*ima_usb;
	struct ima_request	*req;
	int num;
	int			ret = 0;

	ima_ep = container_of(usb_ep, struct ima_ep_struct, ep);
	ima_usb = ima_ep->ima_usb;
	req = container_of(usb_req, struct ima_request, req);
    num = get_ep_num(usb_ep);


    D_REQ(ima_usb->dev, "<%s> ep %d req 0x%p buf 0x%p len 0x%x\n", __func__,
        num,usb_req,usb_req->buf,usb_req->length);

	if (unlikely(!usb_req || !req || !usb_req->complete || !usb_req->buf)) {
		D_ERR(ima_usb->dev, "<%s> ep %d bad params\n", __func__,num);
		return -EINVAL;
	}

	if (unlikely(!usb_ep || !ima_ep)) {
		D_ERR(ima_usb->dev, "<%s> bad ep %d\n", __func__,num);
		return -EINVAL;
	}

	if (!ima_usb->driver || ima_usb->gadget.speed == USB_SPEED_UNKNOWN) {
		D_ERR(ima_usb->dev, "<%s> ep %d bogus device state\n", __func__,num);
		return -ESHUTDOWN;
	}

	if (!ima_ep->inited) {
		D_ERR(ima_usb->dev,
			"<%s> refusing to queue req %p (ep %d stop)\n",
			__func__, req,num);	
		usb_req->status = -ESHUTDOWN;
		return -ESHUTDOWN;
	}

    if(wait_for_completion_timeout(&ima_ep->queue_lock,ima_ep->queue_timeout)<=0){
        D_ERR(ima_ep->ima_usb->dev,"<%s> ep%d wait lock time out\n",__func__,num);
		usb_req->status = -ETIME;
		return -ETIME;
    }

	if (req->in_use) {
		D_ERR(ima_usb->dev,
			"<%s> refusing to queue req %p ust %d ep %d (already queued)\n",
			__func__, req,req->in_use,num);
		goto out;
	}

	usb_req->status = -EINPROGRESS;
	usb_req->actual = 0;

  #if defined(DEBUG)
    req->jiffies_pkt_req = jiffies;
  #endif
	ep_add_request(ima_ep, req);
    ep_data_ready(ima_usb,num);
out:
    complete(&ima_ep->queue_lock);
	return ret;
}

static int ima_ep_dequeue(struct usb_ep *ep, struct usb_request *usb_req)
{
	struct ima_ep_struct	*ima_ep;
	struct ima_udc_struct	*ima_usb;
	struct ima_request	*req;

    int num;

    int ret = 0;

	ima_ep = container_of
					(ep, struct ima_ep_struct, ep);
	ima_usb = ima_ep->ima_usb;
    num = get_ep_num(ep);

    D_REQ(ima_usb->dev, "<%s> ep %d\n", __func__,num);

	if (unlikely(!ep || !get_ep_num(ep))) {
		D_ERR(ima_ep->ima_usb->dev, "<%s> bad ep\n", __func__);
		return -EINVAL;
	}

    if(wait_for_completion_timeout(&ima_ep->queue_lock,ima_ep->queue_timeout)<=0){
        D_ERR(ima_ep->ima_usb->dev,"<%s> ep%d wait lock time out\n",__func__,num);
		usb_req->status = -ETIME;
		return -ETIME;
    }

	/* make sure it's actually queued on this endpoint */
	list_for_each_entry(req, &ima_ep->queue, queue) {
		if (&req->req == usb_req)
			break;
	}
	if (&req->req != usb_req) {
	    D_ERR(ima_ep->ima_usb->dev, "<%s> bad req 0x%p 0x%p\n", __func__,usb_req,&req->req);
		ret = -EINVAL;
		goto out;
	}

    list_del_init(&req->queue);
out:
    complete(&ima_ep->queue_lock);

    if(ret == 0)
	    done(ima_ep, req, -ECONNRESET);

	return 0;
}

static int ima_set_halt(struct usb_ep *ep, int value)
{
	struct ima_ep_struct *ima_ep = container_of(ep,
						struct ima_ep_struct, ep);
	struct ima_udc_struct *ima_usb = ima_ep->ima_usb;

	int ep_num;

    int val;

	D_DBG( "ima_usb: %s\n", __func__);
	if(unlikely(!ep)) {
		D_DBG( "%s: inval 2\n", __func__);
		return -EINVAL;
	}

    ep_num = get_ep_num(ep);

	val = 1<<(bsFHHR_EPx_HALT+ep_num);
    writel(val,Reg(USB_FHHR));   

	return 0;
}

/* gadget operations */
static int ima_get_frame(struct usb_gadget *gadget)
{
    struct ima_udc_struct *ima_usb;

    ima_usb = container_of(gadget,struct ima_udc_struct,gadget);

	D_DBG("%s()\n", __func__);

	return readl(Reg(USB_FNCR)) & 0x7ff;
}

static int ima_wakeup(struct usb_gadget *gadget)
{
	D_DBG( "%s()\n", __func__);
	/* Not implemented in s3c */
	return 0;
}

static int ima_set_selfpowered(struct usb_gadget *gadget, int value)
{
    struct ima_udc_struct *ima_usb;

    ima_usb = container_of(gadget,struct ima_udc_struct,gadget);

	D_DBG( "%s()\n", __func__);

	if(value){
        writel(readl(Reg(USB_UDCR))|UDC_SELF_POWERD,
            Reg(USB_UDCR));  
	}else{
        writel(readl(Reg(USB_UDCR))&~UDC_SELF_POWERD,
            Reg(USB_UDCR));  	  
    }

	return 0;
}

static int ima_pullup(struct usb_gadget *gadget, int is_on)
{
    struct ima_udc_struct *ima_usb;

    ima_usb = container_of(gadget,struct ima_udc_struct,gadget);

	D_DBG( "%s()\n", __func__);

	if(!is_on){
		force_phy_disconnect(ima_usb,1);
	} else{
	    force_phy_disconnect(ima_usb,0);
    }
    
	return 0;
}

static int ima_vbus_session(struct usb_gadget *gadget, int is_active)
{
	D_DBG( "%s()\n", __func__);

	//return ima_pullup(gadget,!is_active);
	return -ENOTSUPP;
}

static int ima_vbus_draw(struct usb_gadget *gadget, unsigned ma)
{
	D_DBG("%s()\n", __func__);
	return -ENOTSUPP;
}

/* interrupt handle */
static void ep0_setup(struct ima_udc_struct *ima_usb,int state)
{
	struct usb_ctrlrequest crq;
	uint32_t val;
	uint32_t * buf;
	int ret;

    if(state & EP0_SETUP_CONFIG){
        if(state & EP0_SETUP_TEST_CONFIG){
            val = 1;
        }else{
            val = readl(Reg(USB_CCR));
            val = GET_BITS(val,CCR_CONFIGURE); 
        }
        enum_set_config_data(&crq,val);
        ima_usb->active_config = val;
    }else{
    	buf = (uint32_t *)&crq;
    	writel(readl(Reg(USB_STR0)),buf++);
    	writel(readl(Reg(USB_STR1)),buf++);
    }

	D_DBG( "bRequestType=%x bRequeset=%x wValue=%x "
	   "wIndex=%x, wLength=%x\n",
	   crq.bRequestType, crq.bRequest, crq.wValue, crq.wIndex,
	   crq.wLength);
	
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
			{
                switch((crq.wValue)>>8){  //check Descriptor Types
                    case USB_DT_DEVICE:
                        D_DBG( "get device descripor\n");
                    break;
                    case USB_DT_CONFIG:
                        D_DBG( "get config descripor\n");
                    break;
                    case USB_DT_STRING:
                        D_DBG( "get string descripor\n");
                      #if defined(ASYNC_SET_CONFIG)
                        mod_timer(&ima_usb->timer, jiffies + HZ/10);
                      #else
                        test_clr_enable_sof_irq(ima_usb);
                      #endif
                    break;
                    case USB_DT_INTERFACE:
                    break;
                    case USB_DT_ENDPOINT:
                    break;
                    case USB_DT_DEVICE_QUALIFIER:
                    break;
                    case USB_DT_OTHER_SPEED_CONFIG:
                    break;
                    case USB_DT_INTERFACE_POWER:
                    break;
                    default:
                        D_DBG( "get unknown type descripor %d\n",(crq.wValue)>>8);
                    
                }
			}
			break;
		default:
		    printk("usb req 0x%x unknow\n",crq.bRequest);
			;
	}

	ret = ima_usb->driver->setup(&ima_usb->gadget, &crq);
	if(ret < 0){
        D_DBG(
            "dev->driver->setup failed. (%d)\n", ret);
	}

    if(state & EP0_SETUP)
        clr_setup_irq(ima_usb);
    
	return ;
}

static void done(struct ima_ep_struct *ima_ep,
					struct ima_request *req, int status)
{
    if(!req)
        return;

	ep_del_request(ima_ep, req);

	req->req.status = status;

	if (status && status != -ESHUTDOWN)
		D_ERR(ima_ep->ima_usb->dev,
			"<%s> failed complete %s req %p stat %d len %u/%u\n", __func__,
			ima_ep->ep.name, &req->req, status,
			req->req.actual, req->req.length);
    else{
      #if defined(DEBUG)
        D_DBG("%s time req %d %ld trans %d %ld len 0x%x\n",ima_ep->ep.name,
            req->jiffies_pkt_req,
            jiffies-req->jiffies_pkt_req,
            req->jiffies_pkt_trans,
            jiffies-req->jiffies_pkt_trans,
            req->req.actual);
      #endif
        D_REQ(ima_ep->ima_usb->dev,
            "<%s> done %s req %p stat %d(%d) len %u/%u\n", __func__,
            ima_ep->ep.name, &req->req, req->req.status,status,
            req->req.actual, req->req.length);
    }
	req->req.complete(&ima_ep->ep, &req->req);
}

static int nuke(struct ima_ep_struct *ima_ep, int status)
{
	struct ima_request *req;
    int pending_req=0;

    if(wait_for_completion_timeout(&ima_ep->queue_lock,ima_ep->queue_timeout)<=0){
        D_ERR(ima_ep->ima_usb->dev,"<%s> ep%d wait lock time out #1\n",__func__,get_ep_num(&ima_ep->ep));
		return pending_req;
    }

	while (!list_empty(&ima_ep->queue)) {
		req = list_entry(ima_ep->queue.next, struct ima_request, queue);		
        D_REQ(ima_ep->ima_usb->dev,"<%s> delete req %p (use =%d)\n",__func__,req,req->in_use);
        
		list_del_init(&req->queue);

		complete(&ima_ep->queue_lock);
		done(ima_ep, req, status);
        pending_req ++;
        if(wait_for_completion_timeout(&ima_ep->queue_lock,ima_ep->queue_timeout)<=0){
            D_ERR(ima_ep->ima_usb->dev,"<%s> ep%d wait lock time out #2\n",__func__,get_ep_num(&ima_ep->ep));
            return pending_req;
        }
	}

	complete(&ima_ep->queue_lock);

	return pending_req;
}

//ret : status
static int ep_out(struct ima_udc_struct *ima_usb,int ep_num,struct ima_request *req)
{
	struct ima_ep_struct  *usb_ep;
	struct usb_request *usb_req;
    uint32_t val;
    uint32_t bytes=0;
    
    uint8_t *data,*cache;

    const unsigned long transfer_timeout=/*100*/HZ/2;
    const unsigned long dma_timeout=/*10*/HZ/20;
    int ret = 0;

    D_EPX(ima_usb->dev, "<%s> ep %d req 0x%p\n", __func__,ep_num,&req->req);

    usb_ep = &ima_usb->ima_ep[ep_num];

    usb_req = &req->req;
    if(!usb_req || !usb_req->buf){
        D_ERR(ima_usb->dev, "<%s> ep %d req 0x%p empty\n", __func__,ep_num,usb_req);
        ret = -EBADR;
        goto out;
    }

	//check halt
    val = readl(Reg(USB_EDRx) + ep_num*4);
    if(GET_BITS(val,EDR_HALT_STS)){
        D_ERR(ima_usb->dev, "<%s> ep %d halt\n", __func__,ep_num);   
        ret = -ENOPROTOOPT;
        goto out;
    }

    if(wait_for_completion_timeout(&ima_usb->transfer_done,transfer_timeout) <= 0){
        D_ERR(ima_usb->dev, "<%s> ep %d transfer dma timeout\n", __func__,ep_num);
        ret = -ETIME;

        msleep(500);
        try_wait_for_completion(&ima_usb->transfer_done);
    }else{

        /* begin transfer */
        val = readl(Reg(USB_PRIR));
        bytes = GET_BITS(val,PRIR_BYTE_CNT);
        bytes = min(usb_req->length - usb_req->actual, bytes);
      
        data = usb_req->buf + usb_req->actual;
      #if defined(CACHE_PIPE_DATA)
        cache = get_cache(ima_usb,ep_num);
      #else
        cache = data;
      #endif

        usb_req->dma = dma_map_single(NULL, (void *)cache, bytes, DMA_FROM_DEVICE);
        
        writel(usb_req->dma, Reg(USB_MDAR));
        ima_usb->active_ep=ep_num;

        try_wait_for_completion(&ima_usb->dma_done); //clean the tag

        val = 0;
        //SET_BITS(val,ACR_PRE_BUFFER_SELECT,0);
        SET_BITS(val,ACR_AUTO_ACK,1);
        SET_BITS(val,ACR_DMA_ENABLE,ACR_DMA_ENABLE_RX);
        //SET_BITS(val,ACR_DATA_TOKEN,0);
        SET_BITS(val,ACR_TX_BUFFER_SELECT,ep_num?EPx_OUT_BUFFER_SEL:EP0_BUFFER_SEL);
        //SET_BITS(val,ACR_DMA_REQ_ERROR,0);
        SET_BITS(val,ACR_DMA_DATA_LEN,bytes);
    	writel(val, Reg(USB_ACR));

        //wait dma finish
        
        if(wait_for_completion_timeout(&ima_usb->dma_done,dma_timeout)<=0){
            D_ERR(ima_usb->dev, "<%s> ep %d dma timeout\n", __func__,ep_num);
            val = readl(Reg(USB_ACR));
            if(!GET_BITS(val,ACR_DMA_ENABLE)){
                ret = -EINPROGRESS;
            }else if(GET_BITS(val,ACR_DMA_REQ_ERROR)){
                D_ERR(ima_usb->dev, "<%s> ep %d dma error\n", __func__,ep_num);
                val = 0;
                SET_BITS(val,ACR_DMA_REQ_ERROR,1);
                writel(val,Reg(USB_ACR));

                val = 1<<(bsFHHR_EPx_HALT+ep_num);
                writel(val,Reg(USB_FHHR));
                val = 1<<BFCR_FLUSH_RXB;
                writel(val,Reg(USB_BFCR));
                ret = -ESTRPIPE;
            }else{
                D_ERR(ima_usb->dev, "<%s> ep %d dma unknow acr =0x%x\n", __func__,ep_num,val);
                ret = -ESTRPIPE;
            }
        }else{
            ret = -EINPROGRESS;
        }

        dma_unmap_single(NULL, usb_req->dma, bytes, DMA_FROM_DEVICE);

      #if defined(CACHE_PIPE_DATA)
        if(ret == -EINPROGRESS || ret ==0)
            memcpy(data,cache,bytes);
      #endif

        print_mem(0,cache,bytes,usb_ep->ep.name);
    }

    complete(&ima_usb->transfer_done);

    if(ret == -EINPROGRESS){
        usb_req->actual += bytes;
        if((usb_req->actual == usb_req->length)||
            (usb_ep->descriptor.wMaxPacketSize!=bytes)){
            ret = 0;
        }
    }else{
        D_ERR(ima_usb->dev, "<%s> ep %d failed  len 0x%x 0x%x ret %d\n", __func__,
            ep_num,usb_req->length,usb_req->actual,ret);
        goto out;
    }
    
out:
    D_REQ(ima_usb->dev,"<%s> ep %d  len 0x%x actual 0x%x this 0x%x ret %d\n", __func__,
        ep_num,usb_req->length,usb_req->actual,bytes,ret);
        
    return ret;
}

//ret : status
static int ep_in(struct ima_udc_struct *ima_usb,int ep_num,struct ima_request *req)
{
    struct ima_ep_struct  *usb_ep;
	struct usb_request *usb_req;
    uint32_t val;

	uint8_t *data,*cache;
	uint32_t size=0;

    const unsigned long transfer_timeout=/*100*/HZ/2;
    const unsigned long dma_timeout=/*10*/HZ/20;

    int ret = 0;

    D_EPX(ima_usb->dev, "<%s> ep %d req 0x%p\n", __func__,ep_num,&req->req);

    usb_ep = &ima_usb->ima_ep[ep_num];

    usb_req = &req->req;
    if(!usb_req || !usb_req->buf){
        D_ERR(ima_usb->dev, "<%s> ep %d req 0x%p empty\n", __func__,ep_num,usb_req);   
        ret = -EBADR;
        goto out;
    }

	//check halt
    val = readl(Reg(USB_EDRx) + ep_num*4);
    if(GET_BITS(val,EDR_HALT_STS)){
        D_ERR(ima_usb->dev, "<%s> ep %d halt\n", __func__,ep_num);   
        ret = -ENOPROTOOPT;
        goto out;
    }


    if(wait_for_completion_timeout(&ima_usb->transfer_done,transfer_timeout) <= 0){
        D_ERR(ima_usb->dev, "<%s> ep %d transfer dma timeout\n", __func__,ep_num);
        ret = -ETIME;

        msleep(500);
        try_wait_for_completion(&ima_usb->transfer_done);
    }else{

    	/* begin transfer */
    
    	size = min((uint32_t)(usb_req->length - usb_req->actual),
    	                (uint32_t)usb_ep->descriptor.wMaxPacketSize);
    
        data = usb_req->buf + usb_req->actual;
      #if defined(CACHE_PIPE_DATA)
    	cache = get_cache(ima_usb,ep_num);
    	memcpy(cache, data, size);
      #else
        cache = data;
      #endif
    	usb_req->dma = dma_map_single(NULL, (void *)cache, size, DMA_TO_DEVICE);
    
    	writel(usb_req->dma, Reg(USB_MDAR));
    
        ima_usb->active_ep=ep_num;
    
        try_wait_for_completion(&ima_usb->dma_done); //clean the tag
    
        val = 0;
        //SET_BITS(val,ACR_PRE_BUFFER_SELECT,0);
        SET_BITS(val,ACR_AUTO_ACK,1);
        SET_BITS(val,ACR_DMA_ENABLE,ACR_DMA_ENABLE_TX);
        //SET_BITS(val,ACR_DATA_TOKEN,0);
        SET_BITS(val,ACR_TX_BUFFER_SELECT,ep_num?EPx_IN_BUFFER_SEL:EP0_BUFFER_SEL);
        //SET_BITS(val,ACR_DMA_REQ_ERROR,0);
        SET_BITS(val,ACR_DMA_DATA_LEN,size);
    	writel(val, Reg(USB_ACR));
    
        //wait dma finish
    
        if(wait_for_completion_timeout(&ima_usb->dma_done,dma_timeout)<=0){
            D_ERR(ima_usb->dev, "<%s> ep %d dma timeout\n", __func__,ep_num);
            val = readl(Reg(USB_ACR));
            if(!GET_BITS(val,ACR_DMA_ENABLE)){
                ret = -EINPROGRESS;
            }else if(GET_BITS(val,ACR_DMA_REQ_ERROR)){
                D_ERR(ima_usb->dev, "<%s> ep %d dma error\n", __func__,ep_num);
                val = 0;
                SET_BITS(val,ACR_DMA_REQ_ERROR,1);
                writel(val,Reg(USB_ACR));
    
                val = 1<<(bsFHHR_EPx_HALT+ep_num);
                writel(val,Reg(USB_FHHR));
                val = BFCR_FLUSH_TXB;
                writel(val,Reg(USB_BFCR));
                ret = -ESTRPIPE;
            }else{
                D_ERR(ima_usb->dev, "<%s> ep %d dma unknow acr =0x%x\n", __func__,ep_num,val);
                ret = -ESTRPIPE;
            }
        }else{
            ret = -EINPROGRESS;
        }
    
        dma_unmap_single(NULL, usb_req->dma, size, DMA_TO_DEVICE);

        print_mem(0,cache,size,usb_ep->ep.name);
    }
    complete(&ima_usb->transfer_done);
    
    //dump_regs("end",ima_usb);
    
    if(ret == -EINPROGRESS){
	    usb_req->actual += size;
	    if((usb_req->zero && !size) ||
	        (usb_ep->descriptor.wMaxPacketSize!=size) ||
	        (!usb_req->zero && (usb_req->actual == usb_req->length))){
	        ret = 0;
	    }    
    }else{
        D_ERR(ima_usb->dev, "<%s> ep %d failed  len 0x%x 0x%x ret %d\n", __func__,
            ep_num,usb_req->length,usb_req->actual,ret); 
        goto out;    
        
    }
    
out:    
    D_REQ(ima_usb->dev, "<%s> ep %d  len 0x%x actual 0x%x this 0x%x ret %d\n", __func__,
        ep_num,usb_req->length,usb_req->actual,size,ret); 
	
	return ret;
}


static void handle_ep0_event(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    int stat = /*EP0_IDLE*/ima_usb->ep0state;
    
	if(isr & (IRQ_CONTROL_SETUP|IRQ_CONTROL_QUERY|IRQ_SET_INTERFACE)) {
        stat |= EP0_SETUP;
    	if(isr & (IRQ_CONTROL_OUT|IRQ_CONTROL_IN)){
            if(isr & IRQ_CONTROL_IN) {
                stat |=EP0_IN_DATA_PHASE; 
            }else if(isr & IRQ_CONTROL_OUT){
                stat |=EP0_OUT_DATA_PHASE;
            }
    	}
        if(isr & IRQ_SET_INTERFACE){
            stat |= EP0_SETUP_CONFIG;
        }
	}else if(isr & (IRQ_CONTROL_OUT|IRQ_CONTROL_IN)){
        if(isr & IRQ_CONTROL_IN) {
            stat =EP0_IN_DATA_PHASE; 
        }else if(isr & IRQ_CONTROL_OUT){
            stat =EP0_OUT_DATA_PHASE;
        }
	}

    if(stat != EP0_IDLE){
        ep0_set_stat(__func__, ima_usb, stat);
        ep_pipe_ready(ima_usb,0);
    }
}

#if !defined(ASYNC_SET_CONFIG)
static void handle_ep0_event_sync(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    int stat = EP0_IDLE;
    
	if(isr & (IRQ_CONTROL_SETUP|IRQ_CONTROL_QUERY|IRQ_SET_INTERFACE)) {
        ep0_setup(ima_usb,isr&IRQ_SET_INTERFACE?EP0_SETUP_CONFIG:0);
	    writel(IRQ_CONTROL_SETUP, Reg(USB_ISR));
	}

	if(isr & (IRQ_CONTROL_OUT|IRQ_CONTROL_IN)){
        if(isr & IRQ_CONTROL_IN) {
            stat =EP0_IN_DATA_PHASE; 
        }else if(isr & IRQ_CONTROL_OUT){
            stat =EP0_OUT_DATA_PHASE;
        }

        writel(IRQ_DMA_DONE, Reg(USB_IDR));
        ep_events_sync(&ima_usb->ima_ep[0],stat);
        writel((isr & (IRQ_CONTROL_OUT|IRQ_CONTROL_IN)), Reg(USB_ISR));
        writel(IRQ_DMA_DONE, Reg(USB_ISR));
        writel(IRQ_DMA_DONE, Reg(USB_IER));
	}
}
#endif

static int handle_epx_event(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    int ep_num;
    uint32_t bit;

    isr &= IRQ_EP1|IRQ_EP2|IRQ_EP3|IRQ_EP4|IRQ_EP5|IRQ_EP6;
    
    for(ep_num =1;ep_num <=6;ep_num++){
        bit = 1<<(IRQ_EPx_SHIFT + ep_num -1);
        if(isr & bit){
            isr &= ~bit;
            ep_pipe_ready(ima_usb,ep_num);            
        }
    }

    return (isr==0);
}

static void handle_phy_event_async(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    set_bus_event(&ima_usb->bus_event,isr);
    
    wake_up(&ima_usb->bus_event.wait);
}

static void handle_phy_event(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    if(isr&IRQ_CONNECT){
        D_DBG( "usb connect\n");
    }
    if(isr&IRQ_DISCONNECT){
        D_DBG( "usb disconnect\n");
        ima_usb->gadget.speed = USB_SPEED_UNKNOWN;
        ima_usb->driver->disconnect(&ima_usb->gadget);
    }
    if(isr&IRQ_RESET){
        D_DBG( "usb reset\n");
        ima_usb->gadget.speed=(readl(Reg(USB_UDCR)) & UDC_DEVICE_SPEED)?
            USB_SPEED_HIGH:USB_SPEED_FULL;
        ima_usb->active_config=0;

        enable_basic_irq(ima_usb);
    }
    if(isr&IRQ_SUSPEND){
        D_DBG( "usb suspend\n");
        ima_usb->gadget.speed = USB_SPEED_UNKNOWN;
    #if defined(ASYNC_HANDLE_PHY_EVENT)
        msleep(HZ*2);  //sleep for thread ex/rx end
    #endif
        writel((IRQ_EP1|IRQ_EP2|IRQ_EP3|IRQ_EP4|IRQ_EP5|IRQ_EP6|IRQ_CONTROL_OUT|IRQ_CONTROL_IN),
            Reg(USB_IDR));

        ima_usb->driver->suspend(&ima_usb->gadget);
        ima_usb->driver->disconnect(&ima_usb->gadget);
    }
    if(isr&IRQ_RESUME){
        D_DBG( "usb resume\n");
        ima_usb->gadget.speed = USB_SPEED_UNKNOWN;
        ima_usb->driver->resume(&ima_usb->gadget);

        enable_basic_irq(ima_usb);
    }

    if(isr&IRQ_SYNC_FRAME){
        D_DBG( "usb sync frame\n");
    }

    if(ep0_get_clr_stat(ima_usb,-1)){
        D_DBG( "send null ep0 packet\n");
        ep_data_ready(ima_usb,0);
        clr_setup_irq(ima_usb);
    }

  #if defined(ASYNC_HANDLE_PHY_EVENT)
    clr_enable_phy_irq(ima_usb);
  #endif
}

static void handle_sof_event(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    if(isr&IRQ_SOF){
        D_DBG( "usb sof %d\n",ima_usb->active_config);
        handle_config((unsigned long)ima_usb);
        if(ima_usb->active_config){
            disable_sof_irq(ima_usb);
        }
    }
}


static void handle_dma_event(struct ima_udc_struct *ima_usb,uint32_t isr)
{
    //clr_enable_epx_irq(ima_usb,ima_usb->active_ep);

    if(isr&IRQ_DMA_DONE){
        D_DBG( "usb dma done(ep %d)\n",ima_usb->active_ep);
    }

    if(isr&IRQ_DMA_ERROR){
        D_DBG( "usb dma error(ep %d)\n",ima_usb->active_ep);
    }

    complete(&ima_usb->dma_done);
}
static irqreturn_t ima_udc_irq(int dummy, void *dev)
{
	struct ima_udc_struct *ima_usb = (struct ima_udc_struct *)dev;
	uint32_t isr,ier,idr;

    //for fix chip bug: should send packet before clear irq

    idr = IRQ_CONTROL_SETUP|IRQ_CONTROL_IN|IRQ_CONTROL_OUT|IRQ_EP1|IRQ_EP2|IRQ_EP3|IRQ_EP4|IRQ_EP5|IRQ_EP6;
    #if defined(ASYNC_HANDLE_PHY_EVENT)
    idr |= IRQ_CONNECT|IRQ_DISCONNECT|IRQ_RESET|IRQ_SUSPEND|IRQ_RESUME|IRQ_SYNC_FRAME;
    #endif
    isr = readl(Reg(USB_ISR));
    ier = readl(Reg(USB_IER));
    writel(isr&~idr, Reg(USB_ISR));
    #if !defined(ASYNC_SET_CONFIG)
    idr &=~(IRQ_CONTROL_SETUP|IRQ_CONTROL_IN|IRQ_CONTROL_OUT);
    #endif
    writel(isr&idr,Reg(USB_IDR));

    isr&=ier;
    
	D_REQ(ima_usb->dev,"irq++ isr=0x%x read isr=0x%x ier=0x%x idr=0x%x\n",
	   isr,readl(Reg(USB_ISR)), readl(Reg(USB_IER)),readl(Reg(USB_IDR)));
    
	if(!isr){
		D_ERR(ima_usb->dev, "%s isr not set in phy\n",__func__);
		return IRQ_HANDLED;
	}

	if(!ima_usb->driver) {
		D_ERR(ima_usb->dev, "%s skip uninitialized interface\n",__func__);
		goto exit;
	}

    if(isr&(
        IRQ_CONNECT|IRQ_DISCONNECT|IRQ_RESET|IRQ_SUSPEND|IRQ_RESUME|IRQ_SYNC_FRAME)){
      #if defined(ASYNC_HANDLE_PHY_EVENT)
        handle_phy_event_async(ima_usb,isr);
      #else
        handle_phy_event(ima_usb,isr);
      #endif
    }

    if(isr&(
        IRQ_DMA_DONE|IRQ_DMA_ERROR)){
        handle_dma_event(ima_usb,isr);
    }

    if(isr&IRQ_SOF){
        handle_sof_event(ima_usb,isr);
    }

    //ep0
	if(isr & (
	    IRQ_CONTROL_QUERY|IRQ_CONTROL_IN|IRQ_CONTROL_OUT|IRQ_CONTROL_SETUP|IRQ_SET_INTERFACE)){
      #if defined(ASYNC_SET_CONFIG)
		handle_ep0_event(ima_usb,isr);
	  #else	
        handle_ep0_event_sync(ima_usb,isr);
      #endif
	}

	//epx
	if(isr & (
	    IRQ_EP1|IRQ_EP2|IRQ_EP3|IRQ_EP4|IRQ_EP5|IRQ_EP6)){
		handle_epx_event(ima_usb,isr);
	}

exit:

	return IRQ_HANDLED;
}

static void ep_events_sync(struct ima_ep_struct  *ima_ep,int ep0state)
{
    struct ima_udc_struct *ima_usb;
    struct ima_request *req;
    int num;

    int status;

    ima_usb=ima_ep->ima_usb;
    num = get_ep_num(&ima_ep->ep);
    
    if (!list_empty(&ima_ep->queue)){
        req = list_entry(ima_ep->queue.next,
            struct ima_request, queue);
        list_del_init(&req->queue);
    }else {
        D_REQ(ima_ep->ima_usb->dev, "<%s> no request on %s\n",
            __func__, ima_ep->ep.name);
        return;
    }

    status = -EINPROGRESS;
    if(num > 0){
        if(ima_ep->descriptor.bEndpointAddress & USB_ENDPOINT_DIR_MASK){
            status = ep_in(ima_usb,num,req);
        }else{
            status = ep_out(ima_usb,num,req);
        }
    }else{
        if(ep0state & (EP0_IN_DATA_PHASE|EP0_OUT_DATA_PHASE)){
            if(ep0state & EP0_IN_DATA_PHASE){
                status = ep_in(ima_usb,num,req);
            }else if(ep0state & EP0_OUT_DATA_PHASE){
                status = ep_out(ima_usb,num,req);
            }
        }
    }
    
    if(status != -EINPROGRESS)
        done(ima_ep, req, status);
    else{
        list_add(&req->queue,&ima_ep->queue);
    }
}



static void ep_events(struct ima_ep_struct *ima_ep)
{
    struct ima_udc_struct *ima_usb;
    struct ima_request *req; 
    int num;

    unsigned long timeout;
    unsigned long timeleft;

    int status,ep0state;

    const unsigned long timeout_in = HZ*3;
    const unsigned long timeout_out = HZ*5;
    
#if defined(DEBUG)
    uint32_t jiffies_irq;
    uint32_t jiffies_req;
#endif

    const int adb_epin =
#if defined(CONFIG_USB_ANDROID_ADB)  
#if defined(CONFIG_USB_ANDROID_MASS_STORAGE)  //test at mass/adb composite device
      3
#else
      1
#endif
#else
      -1
#endif  
    ;


    ima_usb=ima_ep->ima_usb;
    num = get_ep_num(&ima_ep->ep);

    //D_DBG("%s ep %d ++\n",__func__,num);
    
    if(num == 0){  //ep0
        ep0state = ep0_get_clr_stat(ima_usb,EP0_SETUP|EP0_SETUP_CONFIG);
        if(ep0state & EP0_SETUP){
            ep0_setup(ima_usb,ep0state);
        }
        ep0state = ep0_test_stat(ima_usb,EP0_IN_DATA_PHASE|EP0_OUT_DATA_PHASE);
        if(!ep0state){
            return;
        }
    }

    if(!(ima_ep->descriptor.bEndpointAddress&USB_DIR_IN)){
        timeout = timeout_out;
    }else{
        if(num == adb_epin){
            timeout = 1;
        }else{
            timeout = timeout_in;
        }
    }

    #if defined(DEBUG)
      jiffies_irq = jiffies-ima_ep->jiffies_irq;
    #endif

    #if defined(DEBUG)
        ima_ep->jiffies_req = jiffies;
    #endif

    timeleft = wait_event_timeout(ima_ep->data_ready, !list_empty(&ima_ep->queue)||
        !ima_ep->inited||ima_usb->gadget.speed==USB_SPEED_UNKNOWN,timeout);

    /*
    if(timeleft <= 0)
        D_ERR(ima_ep->ima_usb->dev, "<%s> %s data ready(%d) timeout %ld %ld\n",
            __func__, ima_ep->ep.name,list_empty(&ima_ep->queue),timeout,timeleft);*/

    if(ima_usb->gadget.speed == USB_SPEED_UNKNOWN){
        return;
    }
/*
#if defined(CONFIG_USB_ANDROID_ADB)
    if(num == adb_epin){
        while(list_empty(&ima_ep->queue)){
            if(ima_usb->gadget.speed!=USB_SPEED_UNKNOWN){  //closed
                D_TRACE("%s wait req empty %d\n",ima_ep->ep.name,list_empty(&ima_ep->queue));
                //msleep(10);
                wait_event_timeout(ima_ep->data_ready, !list_empty(&ima_ep->queue)||
                    !ima_ep->inited||ima_usb->gadget.speed==USB_SPEED_UNKNOWN,timeout);
                clr_enable_epx_irq(ima_usb,num);
            }else{
                break;
            }
        }
    }
#endif*/

    if(wait_for_completion_timeout(&ima_ep->queue_lock,ima_ep->queue_timeout)<=0){
        D_ERR(ima_ep->ima_usb->dev,"<%s> ep%d wait lock time out\n",__func__,num);
        status = -ETIME;
		goto out;
    }

    if (!list_empty(&ima_ep->queue)){
        req = list_entry(ima_ep->queue.next,
            struct ima_request, queue);
        list_del_init(&req->queue);
        
        D_DBG( "<%s> request on %s\n",
            __func__, ima_ep->ep.name);        
    }else {
      #if defined(CONFIG_USB_ANDROID_MASS_STORAGE)
        D_ERR(ima_ep->ima_usb->dev, "<%s> no request on %s\n",
            __func__, ima_ep->ep.name);
      #endif      
        req = NULL;        
        status = -EBADMSG;
        goto out;
    }

  #if defined(DEBUG)
    if(req->req.actual == 0)
        req->jiffies_pkt_trans= jiffies;
  #endif

    status = -EINPROGRESS;
    if(num > 0){
        if(ima_ep->descriptor.bEndpointAddress & USB_ENDPOINT_DIR_MASK){
            status = ep_in(ima_usb,num,req);
        }else{
            status = ep_out(ima_usb,num,req);
        }
    }else{
        ep0state = ep0_get_clr_stat(ima_usb,EP0_IN_DATA_PHASE|EP0_OUT_DATA_PHASE);
        if(ep0state & (EP0_IN_DATA_PHASE|EP0_OUT_DATA_PHASE)){
            if(ep0state & EP0_IN_DATA_PHASE){
                status = ep_in(ima_usb,num,req);
            }else if(ep0state & EP0_OUT_DATA_PHASE){
                status = ep_out(ima_usb,num,req);
            }else{
                status = -EBADMSG;
            }
        }
    }

#if defined(DEBUG)
    jiffies_req=jiffies-ima_ep->jiffies_req;
    D_DBG( "<%s> %s irq %d req %d jiffies (HZ %d)\n",
        __func__, ima_ep->ep.name,jiffies_irq,jiffies_req,HZ);          
#endif
    
    if(status == -EINPROGRESS){
        list_add/*_tail*/(&req->queue,&ima_ep->queue);
        ep_data_ready(ima_usb,num);
    }

out:

    complete(&ima_ep->queue_lock);

    if(req){
        if(status != -EINPROGRESS){
            done(ima_ep, req, status);
            if(status){
                D_ERR(ima_ep->ima_usb->dev, "<%s> %s event failed %d\n",
                    __func__, ima_ep->ep.name,status);
            }
        }
    }
    if(ima_usb->gadget.speed!=USB_SPEED_UNKNOWN){  //closed
        clr_enable_epx_irq(ima_usb,num);
    }
    //D_DBG("%s ep %d --\n",__func__,num);
}

static int endpoint_thread(void * para)
{
    struct ima_udc_struct *ima_usb;
    struct ima_ep_struct *ima_ep;

	int ep_num;

	/* khubd needs to be freezable to avoid intefering with USB-PERSIST
	 * port handover.  Otherwise it might see that a full-speed device
	 * was gone before the EHCI controller had handed its port over to
	 * the companion full-speed controller.
	 */
    D_DBG("%s:  entering\n", __func__);

    ima_ep = (struct ima_ep_struct *)para;
	ima_usb = ima_ep->ima_usb;
    ep_num = get_ep_num(&ima_ep->ep);
	
    D_DBG("%s:  after set_freezable\n", __func__);

	do {	    
        wait_for_completion(&ima_ep->pipe_ready);				
		if(!kthread_should_stop())
		    ep_events(ima_ep);
	} while (!kthread_should_stop());

	D_DBG("%s:  exiting\n", __func__);
	return 0;
}

static int bus_event_thread(void * para)
{
    struct ima_udc_struct *ima_usb;
    struct ima_bus_event_struct *bus_event;
	/* khubd needs to be freezable to avoid intefering with USB-PERSIST
	 * port handover.  Otherwise it might see that a full-speed device
	 * was gone before the EHCI controller had handed its port over to
	 * the companion full-speed controller.
	 */
    D_DBG("%s:  entering\n", __func__);

	ima_usb = (struct ima_udc_struct *)para;
	bus_event = &ima_usb->bus_event;
	//set_freezable();
    D_DBG("%s:  after set_freezable\n", __func__);

	do {	    
		//wait_event_freezable(ima_ep->pipe_ready,
		wait_event(bus_event->wait,
				(get_bus_event(bus_event,0))||
				kthread_should_stop());
		if(!kthread_should_stop())
		    handle_phy_event(ima_usb,get_bus_event(bus_event,1));
	} while (!kthread_should_stop());

	D_DBG("%s:  exiting\n", __func__);
	return 0;
}


static void handle_config(unsigned long data)
{
	struct ima_udc_struct *ima_usb = (void *)data;
	struct usb_ctrlrequest crq;

    uint32_t val;
    uint16_t configure;

    if(!ima_usb->active_config){
        val = readl(Reg(USB_CCR));
        configure = GET_BITS(val,CCR_CONFIGURE);
        if(configure){
            D_EP0(ima_usb->dev, "set config %d\n",configure);
            enum_set_config_data(&crq,configure);
            ima_usb->active_config = configure;
            ima_usb->driver->setup(&ima_usb->gadget, &crq);
          #if defined(ASYNC_SET_CONFIG)
            del_timer(&ima_usb->timer);
          #endif  
        }/*else{
          #if defined(ASYNC_SET_CONFIG)  
            if(--check_time)
                mod_timer(&ima_usb->timer, jiffies + HZ/4);
            else
                D_EP0(ima_usb->dev, "set config timeout\n");
          #endif
        }*/
    }
}

static void udc_en(struct ima_udc_struct *ima_usb,int enable)
{
    int val;
    int timeout;

    writel(0, Reg(USB_BCWR));
    writel(0, Reg(USB_BCIER));
    writel((volatile unsigned int *)-1, Reg(USB_BCIDR));
    writel((volatile unsigned int *)-1,Reg(USB_BCISR));

	if(enable){
	    D_DBG( "udc_en able\n");
        ima_usb->gadget.speed = USB_SPEED_FULL;
        
		//set configure
        writel((0xf<<PIR_INTERFACE1_MAXALT_SHIFT)|   //configure 0/1
                (1<<PIR_CONFIGURE1_NUMBER_SHIFT)|
                (1<<PIR_INTERFACE1_NUMBER_SHIFT)|
                (1<<PIR_INTERFACE1_ACTIVE_ALT_SHIFT)|
                (0xf<<PIR_INTERFACE0_MAXALT_SHIFT)|
                (1<<PIR_CONFIGURE0_NUMBER_SHIFT)|
                (0<<PIR_INTERFACE0_NUMBER_SHIFT)|
                (0<<PIR_INTERFACE0_ACTIVE_ALT_SHIFT),
                 Reg(USB_PIR0));
        writel((0xf<<PIR_INTERFACE3_MAXALT_SHIFT)|   //configure 3/2
                (1<<PIR_CONFIGURE3_NUMBER_SHIFT)|
                (3<<PIR_INTERFACE3_NUMBER_SHIFT)|
                (3<<PIR_INTERFACE3_ACTIVE_ALT_SHIFT)|
                (0xf<<PIR_INTERFACE2_MAXALT_SHIFT)|
                (1<<PIR_CONFIGURE2_NUMBER_SHIFT)|
                (2<<PIR_INTERFACE2_NUMBER_SHIFT)|
                (2<<PIR_INTERFACE2_ACTIVE_ALT_SHIFT),
                 Reg(USB_PIR1));
        /*     
        timeout=2000;
        while(--timeout){
            if(readl(Reg(USB_BCSR)) & USB_B_Valid)
                break;
            msleep(1);    
        }

        if(!timeout){
            dprintk(0, "b device not connect\n");
        }*/

        //make B-Device enter peripheral mode
        writel(readl(Reg(USB_BCWR))|USB_B_Connect,
           Reg(USB_BCWR));
           
        timeout=2000;
        while(--timeout){
            val=readl(Reg(USB_BCSR));
            if(((val>>BCSR_B_STATUS_SHIFT)&0x3)==0x1)  //B_PERI
                break;
            msleep(1);    
        }

        if(!timeout){
            D_DBG( "wait be B_PERI failed\n");
        }
        
        //clear B-Device event
        writel(readl(Reg(USB_BCWR))&~USB_B_Connect,
           Reg(USB_BCWR));

        //set buffer
        val=0;
        writel(val, Reg(USB_TBCR0));/*val+=0x10;*/ 
        writel(val, Reg(USB_TBCR1));/*val+=0x100;*/
        writel(val, Reg(USB_TBCR2));/*val+=0x80;*/
        writel(val, Reg(USB_TBCR3));
        writel(0, Reg(USB_MDAR));

        writel((1<<bsACR_AUTO_ACK)|
                (1<<bsACR_PRE_BUFFER_SELECT), Reg(USB_ACR));

        writel((uint32_t)-1,Reg(USB_ISR));
                
        enable_basic_irq(ima_usb);    

        writel(0,Reg(USB_FNCR));
        writel(UDC_DEVICE_SPEED|
                UDC_IPG_DISABLE|
                UDC_SYNC_FRAME_SUPPORT|
                //UDC_SOFT_DISCONNECT|
                //UDC_SELF_POWERD|
                UDC_SOFT_CONNECT|
                UDC_CORE_ENABLE,
            Reg(USB_UDCR));

        //set epx halt
        val = 0x3e<<16;
        writel(val,Reg(USB_FHHR));   
        
        //ep 0
        ima_ep_enable(&ima_usb->ima_ep[0].ep,NULL);
	}else{
        D_DBG( "udc_en disable\n");
        writel(0, Reg(USB_UDCR));        
	}
	return ;
}

/*******************************************************************************
 * USB endpoint control functions
 *******************************************************************************
 */

static void ep0_chg_stat(const char *label,
			struct ima_udc_struct *ima_usb, enum ep0_state stat)
{
    unsigned long flags;
    
	D_EP0(ima_usb->dev, "<%s>#1  ep0 state from 0x%x to 0x%x\n",
		label, ima_usb->ep0state, stat);

	local_irq_save(flags);	
	ima_usb->ep0state = stat;

	local_irq_restore(flags);
	
}
static void ep0_set_stat(const char *label,
            struct ima_udc_struct *ima_usb, enum ep0_state stat)
{
    unsigned long flags;
    
    local_irq_save(flags);
    
    ima_usb->ep0state |= stat;
    local_irq_restore(flags);

    D_EP0(ima_usb->dev, "<%s>ep0 set state from 0x%x | 0x%x\n",
        label, ima_usb->ep0state, stat);
}

static void ep0_clr_stat(const char *label,
            struct ima_udc_struct *ima_usb, enum ep0_state stat)
{
    unsigned long flags;
    /*D_EP0(ima_usb->dev, "<%s>ep0 clr state from 0x%x &~ 0x%x\n",
        label, ima_usb->ep0state, stat);*/

    local_irq_save(flags);
    
    ima_usb->ep0state &= ~stat;

    local_irq_restore(flags);
}

static int ep0_get_clr_stat(struct ima_udc_struct *ima_usb,int stat_clr)
{
    enum ep0_state stat;
    unsigned long flags;

    /*D_EP0(ima_usb->dev, " %s current state 0x%x,query 0x%x\n",
        __func__,ima_usb->ep0state,stat_clr);*/

    local_irq_save(flags);
    
    stat = ima_usb->ep0state;
    ima_usb->ep0state &= ~stat_clr;
    
    local_irq_restore(flags);

    return stat&stat_clr;
}

static int ep0_test_stat(struct ima_udc_struct *ima_usb,int stat_test)
{
    enum ep0_state stat;

    unsigned long flags;

    /*D_EP0(ima_usb->dev, " %s current state 0x%x,test 0x%x\n",
        __func__,ima_usb->ep0state,stat_test);*/

    local_irq_save(flags);
    
    stat = ima_usb->ep0state;
    
    local_irq_restore(flags);

    return stat&stat_test;
}


static int usb_init_data(struct ima_udc_struct *ima_usb)
{
	struct ima_ep_struct *ima_ep;
	u8 i;

	/* device/ep0 records init */
	INIT_LIST_HEAD(&ima_usb->gadget.ep_list);
	INIT_LIST_HEAD(&ima_usb->gadget.ep0->ep_list);
	init_completion(&ima_usb->dma_done);
	init_completion(&ima_usb->transfer_done);
	complete(&ima_usb->transfer_done);
	ep0_chg_stat(__func__, ima_usb, EP0_IDLE);

#if defined(ASYNC_HANDLE_PHY_EVENT)
    init_waitqueue_head(&ima_usb->bus_event.wait);
    ima_usb->bus_event.event=0;

    ima_usb->bus_event.task = kthread_run(bus_event_thread, (void *)ima_usb,"ima usb bus thread");
    if (IS_ERR(ima_usb->bus_event.task)){
        D_DBG( "%s bus event thread failed %d\n",__func__,(int)ima_usb->bus_event.task);
        return (int)ima_usb->bus_event.task;
    }
#endif
	/* basic endpoint records init */
	for (i = 0; i < IMA_EP_COUNT; i++) {
		ima_ep = &ima_usb->ima_ep[i];

        INIT_LIST_HEAD(&ima_ep->queue);

        init_completion(&ima_ep->queue_lock);
        ima_ep->queue_timeout=2*HZ;
        complete(&ima_ep->queue_lock);
        
        init_waitqueue_head(&ima_ep->data_ready);
        init_completion(&ima_ep->pipe_ready);

        ima_ep->kepd_task = kthread_run(endpoint_thread, (void *)ima_ep,ima_ep->ep.name);
        if (IS_ERR(ima_ep->kepd_task)){
            D_DBG( "%s %s kepd_task failed %d\n",__func__,ima_ep->ep.name,(int)ima_ep->kepd_task);
            return (int)ima_ep->kepd_task;
        }
        ima_ep->ima_usb=ima_usb;
        
        if(i)
		    list_add_tail(&ima_ep->ep.ep_list,
			    &ima_usb->gadget.ep_list);
	}

	return 0;
}

static void ima_udc_reset(struct ima_udc_struct *ima_usb)
{
    //reset pad
	writel(readl(rPAD_CFG) & ~0x8, rPAD_CFG);                 
	writel(0xc,rUSB_SRST);                
	udelay(100);                         
	writel(readl(rPAD_CFG) | 0x8, rPAD_CFG);                 
	mdelay(4);                        
	writel(0xd,rUSB_SRST);                
	udelay(200);                         
	writel(0xf,rUSB_SRST);  

    writel(0,
        Reg(USB_UDCR));

	
}

void ima_udc_init_int(struct ima_udc_struct *ima_usb)
{
    writel((uint32_t)-1,
        Reg(USB_IDR));
}

void ima_udc_init_ep(struct ima_udc_struct *ima_usb)
{
    //...not finish
}

void ima_udc_init_fifo(struct ima_udc_struct *ima_usb)
{
    //...not finish
}

static void usb_init_udc(struct ima_udc_struct *ima_usb)
{
	/* Reset UDC */
	ima_udc_reset(ima_usb);

	/* Download config to enpoint buffer */
	//ima_udc_config(ima_usb);

	/* Setup interrups */
	ima_udc_init_int(ima_usb);

	/* Setup endpoints */
	ima_udc_init_ep(ima_usb);

	/* Setup fifos */
	ima_udc_init_fifo(ima_usb);
}

static int ima_udc_probe(struct platform_device *pdev)
{
	struct ima_udc_struct *ima_usb = &controller;
	struct device *dev = &pdev->dev;	
	struct clk *clk;
	struct resource *res;
	void __iomem    *base;
	int res_size;

	int ret = 0;

	printk("%s()\n", __func__);

	//clk
	clk = clk_get(NULL, "usb-device");
	if(IS_ERR(clk)) {
		dev_err(dev, "failed to get ima_usb clock source\n");
		return PTR_ERR(clk);
	}
	clk_enable(clk);

	mdelay(10);
	D_DBG( "%s clock enable\n",__func__);

    res = pdev->resource;
	res_size = resource_size(res);
	if (!request_mem_region(res->start, res_size, res->name)) {
		dev_err(&pdev->dev, "can't allocate %d bytes at %d address\n",
			res_size, res->start);
		return -ENOMEM;
	}
	
	base = ioremap_nocache(res->start, res_size);
	if(!base){
		dev_err(dev, "Can not remap register address..\n");
		ret = -EIO;
		goto fail0;
	}

	/* allocate dma buffer */
    ima_usb->d_buffer=(uint8_t *)__get_free_pages(GFP_KERNEL|GFP_DMA,2);
	if(!ima_usb->d_buffer){
		D_DBG( "allocate global buffer failed.\n");
		ret = -ENOMEM;
		goto fail1;
	}
	if(virt_addr_valid((uint32_t)ima_usb->d_buffer)){
        D_DBG( "allocate valid global buffer %p.\n", ima_usb->d_buffer);
	}else{
        dev_err(dev, "allocated but not valid global buffer %p.\n", ima_usb->d_buffer);
        ret = -ENOMEM;
        goto fail1;
    }
    memset(ima_usb->d_buffer,0,PAGE_SIZE<<2);

	ima_usb->res = res;
	ima_usb->base = base;
	ima_usb->clk = clk;
	ima_usb->dev = dev;
    usb_init_udc(ima_usb);  //forbid controller
    
	ima_usb->irq = platform_get_irq(pdev, 0);
	D_DBG( "ima_usb: irq no. %d\n", ima_usb->irq);
	ret = request_irq(ima_usb->irq, ima_udc_irq, IRQF_DISABLED, pdev->name, ima_usb);
	if(ret){
		dev_err(dev, "cannot get irq %i, err %d\n", ima_usb->irq, ret);
		goto fail1;
	}

	device_initialize(&ima_usb->gadget.dev);
	ima_usb->gadget.dev.parent = dev;
	ima_usb->gadget.dev.dma_mask = dev->dma_mask;

	platform_set_drvdata(pdev, ima_usb);

	ret = usb_init_data(ima_usb);
	if(ret){
		dev_err(dev, "%s usb_init_data failed\n",__func__);
		goto fail2;
	}	
	//usb_init_udc(ima_usb);

	init_timer(&ima_usb->timer);
	ima_usb->timer.function = handle_config;
	ima_usb->timer.data = (unsigned long)ima_usb;

	return 0;
fail2:
    disable_irq(ima_usb->irq);
fail1:
    kfree(ima_usb->d_buffer);
    iounmap(base);
fail0:
    release_mem_region(res->start, res_size);

	return -1;
}

static int ima_udc_remove(struct platform_device *pdev)
{
	struct ima_udc_struct *ima_usb = platform_get_drvdata(pdev);
	struct resource *res;

	dev_dbg(&pdev->dev, "%s()\n", __func__);

	res = pdev->resource;

	if(ima_usb->driver)
	  return -EBUSY;

	free_irq(ima_usb->irq, ima_usb);
	iounmap(ima_usb->base);
	release_mem_region(res->start, res->end - res->start + 1);
    kfree(ima_usb->d_buffer);

	platform_set_drvdata(pdev, NULL);

	if(!IS_ERR(ima_usb->clk) && ima_usb->clk) {
		clk_disable(ima_usb->clk);
		clk_put(ima_usb->clk);
		ima_usb->clk = NULL;
	}

	dev_dbg(&pdev->dev, "%s: remove ok\n", __func__);
	return 0;
}



int usb_gadget_register_driver(struct usb_gadget_driver *driver)
{
    struct ima_udc_struct *ima_usb = &controller;

	int ret;
	D_DBG( "usb_gadget_register_driver() '%s'\n",
		driver->driver.name);

	if(ima_usb->driver)
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
	ima_usb->driver = driver;
	ima_usb->gadget.dev.driver = &driver->driver;

	/* Bind the driver */
	if ((ret = device_add(&ima_usb->gadget.dev)) != 0) {
		printk(KERN_ERR "Error in device_add() : %d\n",ret);
		goto register_error;
	}

	D_DBG( "binding gadget driver '%s'\n",
		driver->driver.name);

	if ((ret = driver->bind (&ima_usb->gadget)) != 0) {
		device_del(&ima_usb->gadget.dev);
		goto register_error;
	}

	/* Enable UDC */
    udc_en(ima_usb,1);

	return 0;

register_error:
	ima_usb->driver = NULL;
	ima_usb->gadget.dev.driver = NULL;
	return ret;
}

int usb_gadget_unregister_driver(struct usb_gadget_driver *driver)
{
    struct ima_udc_struct *ima_usb = &controller;

	if (!driver || driver != ima_usb->driver || !driver->unbind)
		return -EINVAL;

	D_DBG("usb_gadget_register_driver() '%s'\n",
		driver->driver.name);

	driver->unbind(&ima_usb->gadget);
	device_del(&ima_usb->gadget.dev);
	ima_usb->driver = NULL;

	/* Disable UDC */
	udc_en(ima_usb,0);

	return 0;
}
EXPORT_SYMBOL(usb_gadget_unregister_driver);
EXPORT_SYMBOL(usb_gadget_register_driver);


#ifdef CONFIG_PM
static int ima_udc_suspend(
   struct platform_device *pdev, pm_message_t message)
{
	return 0;
}

static int ima_udc_resume(struct platform_device *pdev)
{
	return 0;
}
#else
#define ima_udc_suspend NULL
#define ima_udc_resume NULL
#endif

static struct platform_driver ima_udc_driver = {
	.driver		= {
		.name	= "imapx200_usbotg",//"ix-ima_usb",
		.owner	= THIS_MODULE,
	},
	.probe		= ima_udc_probe,
	.remove		= ima_udc_remove,
	.suspend	= ima_udc_suspend,
	.resume		= ima_udc_resume,
};

static int __init ima_udc_init(void)
{
	printk(KERN_INFO "iMAPx200 USB Device controller (c) 2009~2014\n");

	return 
		platform_driver_register(&ima_udc_driver);
}

static void __exit ima_udc_exit(void)
{
	platform_driver_unregister(&ima_udc_driver);
}

module_init(ima_udc_init);
module_exit(ima_udc_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("warits <warits.wang@infotmic.com.cn>");
MODULE_DESCRIPTION("UDC driver for iMAPx200");
MODULE_VERSION("v0");
