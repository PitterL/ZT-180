/***************************************************************************** 
** drivers/usb/gadget/ima200_udc.h
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

#ifdef BIT_MASK
#undef BIT_MASK
#endif

#define   BIT_MASK(__bf)   (((1U   <<   (bw ##  __bf)) - 1) << (bs ## __bf))  
#define   SET_BITS(__dst, __bf, __val) \
  ((__dst) = ((__dst) & ~(BIT_MASK(__bf))) | \
  (((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))
#define   GET_BITS(__dst, __bf) \
      (((__dst) & (BIT_MASK(__bf)))>>(bs ## __bf))

#define Reg(x) (ima_usb->base+(x))

#define IMA_EP_COUNT	7
#define EP0_MAXPACKET 64
#define EP_MAXPACKET 512

#define SOF_WAIT_CONTROL_DATA_STAGE     (1<<0)
#define SOF_WAIT_CONFIGURE_DESCRIPTOR   (1<<1)
#define SOF_WAIT_PIPEx_TOKEN            (1<<3)

struct ima_request {
	struct usb_request			req;
	struct list_head			queue;
	unsigned int				in_use;
  #if defined(DEBUG)
    uint32_t                    jiffies_pkt_req;
    uint32_t                    jiffies_pkt_trans;
  #endif
};

enum ep0_state {
	EP0_IDLE = 0,
	EP0_SETUP = (1<<1),
	EP0_SETUP_CONFIG = (1<<2),
	EP0_IN_DATA_PHASE =(1<<3),
	EP0_OUT_DATA_PHASE = (1<<4),
	EP0_ACK = (1<<5),
	EP0_SETUP_TEST_CONFIG = (1<<6),
};

struct ima_bus_event_struct {
	//spinlock_t                  lock;	
    struct task_struct *        task;

    wait_queue_head_t           wait;

    uint32_t   event;
};


struct ima_ep_struct {
	struct usb_ep				ep;
	struct ima_udc_struct		*ima_usb;
	struct list_head			queue;
	struct completion           queue_lock;
    unsigned long               queue_timeout;
	
	struct usb_endpoint_descriptor descriptor;
	
	int         				inited;
    struct task_struct *        kepd_task;

    wait_queue_head_t           data_ready;
    struct completion           pipe_ready;

  #if defined(DEBUG)
    uint32_t                    jiffies_irq;
    uint32_t                    jiffies_req;
    uint32_t                    jiffies_token;
  #endif
};

struct ima_udc_struct {
	struct usb_gadget			gadget;
	struct usb_gadget_driver	*driver;
	struct device				*dev;

	struct ima_bus_event_struct bus_event;
	struct ima_ep_struct		ima_ep[IMA_EP_COUNT];
	struct clk				    *clk;
	struct resource				*res;
	void __iomem				*base;

    struct timer_list			timer;

    //spinlock_t                  state_lock;
	enum ep0_state				ep0state;
    uint16_t                    active_config;

    struct completion           dma_done;
    struct completion           transfer_done;

#if defined(CONFIG_USB_ANDROID_ADB)
    uint32_t                    ep2_counter;
#endif
    uint32_t                    active_ep;
					
	int							irq;
    uint8_t                     *d_buffer;
};


/* externs */
static int ima_ep_enable(struct usb_ep *ep, 
   const struct usb_endpoint_descriptor *desc);
static int ima_ep_disable(struct usb_ep *ep);
static struct usb_request *
ima_ep_alloc_request(struct usb_ep *ep, gfp_t mem_flags);
static void
ima_ep_free_request(struct usb_ep *ep, struct usb_request *req);
static int ima_ep_queue(struct usb_ep *ep, struct usb_request *req,
   gfp_t gfp_flags);
static int ima_ep_dequeue(struct usb_ep *ep, struct usb_request *req);
static int ima_set_halt(struct usb_ep *ep, int value);
static int ima_get_frame(struct usb_gadget *gadget);
static int ima_wakeup(struct usb_gadget *gadget);
static int ima_set_selfpowered(struct usb_gadget *gadget, int value);
static int ima_pullup(struct usb_gadget *gadget, int is_on);
static int ima_vbus_session(struct usb_gadget *gadget, int is_active);
static int ima_vbus_draw(struct usb_gadget *gadget, unsigned ma);

#define EP0_BUFFER_SEL      0
#define EPx_IN_BUFFER_SEL   1
#define EPx_OUT_BUFFER_SEL  2


#ifdef DEBUG
#undef dev_dbg
#define dev_dbg(dev, format, arg...)		\
	printk(format , ## arg)

 //#define DEBUG_DBG
 //#define DEBUG_REQ 
 //#define DEBUG_TRX 
 //#define DEBUG_INIT 
 #define DEBUG_EP0 
 //#define DEBUG_EPX 
 //#define DEBUG_IRQ 
 //#define DEBUG_EPIRQ 
 //#define DEBUG_DUMP 
 #define DEBUG_ERR 

#define D_TRACE(args...)	dev_dbg(dev, ## args)

#ifdef DEBUG_DBG
	#define D_DBG(args...)	dev_dbg(dev, ## args)
#else
	#define D_DBG(args...)	do {} while (0)
#endif /* DEBUG_DBG */

#ifdef DEBUG_REQ
	#define D_REQ(dev, args...)	dev_dbg(dev, ## args)
#else
	#define D_REQ(dev, args...)	do {} while (0)
#endif /* DEBUG_REQ */

#ifdef DEBUG_TRX
	#define D_TRX(dev, args...)	dev_dbg(dev, ## args)
#else
	#define D_TRX(dev, args...)	do {} while (0)
#endif /* DEBUG_TRX */

#ifdef DEBUG_INIT
	#define D_INI(dev, args...)	dev_dbg(dev, ## args)
#else
	#define D_INI(dev, args...)	do {} while (0)
#endif /* DEBUG_INIT */

#ifdef DEBUG_EP0
	#define D_EP0(dev, args...)	dev_dbg(dev, ## args)
#else
	#define D_EP0(dev, args...)	do {} while (0)
#endif /* DEBUG_EP0 */

#ifdef DEBUG_EPX
	#define D_EPX(dev, args...)	dev_dbg(dev, ## args)
#else
	#define D_EPX(dev, args...)	do {} while (0)
#endif /* DEBUG_EP0 */


#ifdef DEBUG_DUMP
	static void dump_usb_stat(const char *label,
						struct ima_udc_struct *ima_usb)
	{

	}

	static void dump_ep_stat(const char *label,
						struct ima_ep_struct *ima_ep)
	{

	}

	static void dump_req(const char *label, struct ima_ep_struct *ima_ep,
							struct usb_request *req)
	{

	}
	static void dump_regs(const char *label,
						struct ima_udc_struct *ima_usb)
	{
        int i;
        if(label)
            dev_dbg(ima_usb->dev,"<%s> request dump <\n", label);
        for(i = USB_ACR;i<=USB_EDR6;i+=4)
            printk("%04x 0x%08x\n",i,readl(Reg(i)));
        if(label)
            printk(">\n");

    }
#else
	#define dump_ep_stat(x, y)		do {} while (0)
	#define dump_usb_stat(x, y)		do {} while (0)
	#define dump_req(x, y, z)		do {} while (0)
#endif /* DEBUG_DUMP */

#ifdef DEBUG_ERR
	#define D_ERR(dev, args...)	dev_dbg(dev, ## args)
#else
	#define D_ERR(dev, args...)	do {} while (0)
#endif


#else
    #define D_TRACE(args...)        do {} while (0)
    #define D_DBG(args...)		    do {} while (0)
	#define D_REQ(dev, args...)		do {} while (0)
	#define D_TRX(dev, args...)		do {} while (0)
	#define D_INI(dev, args...)		do {} while (0)
	#define D_EP0(dev, args...)		do {} while (0)
	#define D_EPX(dev, args...)		do {} while (0)
	#define dump_ep_intr(x, y, z, i)	do {} while (0)
	#define dump_intr(x, y, z)		do {} while (0)
	#define dump_ep_stat(x, y)		do {} while (0)
	#define dump_usb_stat(x, y)		do {} while (0)
	#define dump_req(x, y, z)		do {} while (0)
	#define D_ERR(dev, args...)		do {} while (0)

    #define DEBUG_NORMAL 0
    #define DEBUG_VERBOSE 0

    static int dprintk(int level, const char *fmt, ...)
    {
        return 0;
    }	
#endif /* DEBUG */


#endif /* __IMAP_UDC_H__ */
