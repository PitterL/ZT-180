/***************************************************************************** 
** drivers/video/infotm/imapfb.c
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Implementation file of Display Controller.
**
** Author:
**     Feng Jiaxing <jiaxing_feng@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  12/08/2009  Feng Jiaxing
** 1.1	23/06/2010  Sololz		add interrupt wait
*****************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <linux/slab.h>
#include <linux/delay.h>
#include <linux/fb.h>
#include <linux/init.h>
#include <linux/dma-mapping.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/wait.h>
#include <linux/platform_device.h>
#include <linux/clk.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <asm/div64.h>
#include <asm/param.h>

#if defined(CONFIG_PM)
#include <plat/pm.h>
#endif

#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>

#include "../edid.h"
#include "imapfb.h"

static unsigned int		*overlay_reg	= NULL;	/* overlay control register added by sololz */

#ifdef CONFIG_LCD_ENABLE_IRQ
static void enable_fb_lcd_interrupt(void);
static void disable_fb_lcd_interrupt(void);

/* 
 * This mark variable is for user space call ioctl syscall to check whether 
 * an interrupt has generated. This mark will be set 0 at entry of ioctl 
 * syscall, and then ioctl will call correspond function to block thread to
 * wait next interrupt to set mark to be 1(none zero). So this mark need
 * a lock to protect it's value to avoid being operated at the same time.
 */
static volatile int		irq_mark	= 1;
/* 
 * This register base pointer is the info->io, I set a global variable to
 * store the value just want to operate interuption related registers
 */
static ids_reg_t	 	*reg_fb		= NULL;
/* This mutex lock is initialized at probe and released at remove */
static struct mutex		mark_mutex;
static wait_queue_head_t 	wait_fb;
#endif	/* CONFIG_LCD_ENABLE_IRQ */

//Imap Framebuffer Stucture Array
imapfb_info_t imapfb_info[IMAPFB_NUM];

static inline void imapfb_set_lcd_power(UINT32 to)
{
	if (imapfb_fimd.set_lcd_power)
		(imapfb_fimd.set_lcd_power)(to);
}

static inline void imapfb_set_backlight_power(UINT32 to)
{
	if (imapfb_fimd.set_backlight_power)
		(imapfb_fimd.set_backlight_power)(to);
}

static inline void imapfb_set_backlight_level(UINT32 to)
{
	if (imapfb_fimd.set_brightness)
		(imapfb_fimd.set_brightness)(to);
}

static UINT32 lcd_power_status = 0;	/* LCD Power Supply Status: On/Off */
/*****************************************************************************
** -Function:
**    imapfb_power_supply_onoff(UINT32 onoff)
**
** -Description:
**    This function implement special features. The process is,
**		1. Turn on/off Lcd power supply, not display controller output signal
**
** -Input Param
**    onoff        Turn on/off
**
** -Output Param
**    none
**
** -Return
**    none
**
*****************************************************************************/
static void imapfb_power_supply_onoff(UINT32 onoff)
{
	if (onoff)
	{
		imapfb_set_lcd_power(1);
		imapfb_set_backlight_power(1);
		imapfb_set_backlight_level(IMAPFB_DEFAULT_BACKLIGHT_LEVEL);
		lcd_power_status = 1;
	}
	else
	{
		imapfb_set_backlight_level(0);
		msleep(50);
		imapfb_set_backlight_power(0);
		imapfb_set_lcd_power(0);
		lcd_power_status = 0;
	}
}

/*****************************************************************************
** -Function:
**    imapfb_map_video_memory(imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**		1. Allocate framebuffer memory for input framebuffer window.
**		2. Save physical address, virtual address and size of framebuffer memory.
**		3. If input framebuffer window is win0 and kernel logo is configured, then show this logo.
**
** -Input Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Output Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Return
**    -ENOMEM	: Failure
**	0			: Success
**
*****************************************************************************/
static int imapfb_map_video_memory(imapfb_info_t *fbi)
{
#if defined(CONFIG_FB_IMAP_DOUBLE_BUFFER)
	fbi->fb.fix.smem_len *= 2;
#endif
	//Allocate framebuffer memory and save physical address, virtual address and size
	fbi->map_size_f1 = PAGE_ALIGN(fbi->fb.fix.smem_len);
	fbi->map_cpu_f1 = dma_alloc_writecombine(fbi->dev, fbi->map_size_f1, &fbi->map_dma_f1, GFP_KERNEL);
	fbi->map_size_f1 = fbi->fb.fix.smem_len;

	//If succeed in allocating framebuffer memory, then init the memory with some color or kernel logo 
	if (fbi->map_cpu_f1)
	{
//		printk("Window[%d] - FB1: map_video_memory: clear %p:%08x\n",
//			fbi->win_id, fbi->map_cpu_f1, fbi->map_size_f1);

		//Show kernel logo
#if defined (CONFIG_FB_IMAP_KERNEL_LOGO)
		/*
		 *Added for INFOTM LOGO
		 */
		if (!fbi->win_id)
			imapfb_kernel_logo(fbi->map_cpu_f1); 
#else
		memset(fbi->map_cpu_f1, 0x0, fbi->map_size_f1);
#endif

		//Set physical and virtual address for future use
		fbi->fb.screen_base = fbi->map_cpu_f1;
		fbi->fb.fix.smem_start = fbi->map_dma_f1;

//		printk("            FB1: map_video_memory: dma=%08x cpu=%p size=%08x\n",
//			fbi->map_dma_f1, fbi->map_cpu_f1, fbi->fb.fix.smem_len);
	}
	else
	{
		printk(KERN_ERR "[imapfb_map_video_memory]: win%d fail to allocate framebuffer memory\n", fbi->win_id);
		return -ENOMEM;
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_unmap_video_memory(imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**		1. Free framebuffer memory for input framebuffer window.
**
** -Input Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Output Param
**    none
**
** -Return
**    none
**
*****************************************************************************/
static void imapfb_unmap_video_memory(imapfb_info_t *fbi)
{
	dma_free_writecombine(fbi->dev, fbi->map_size_f1, fbi->map_cpu_f1,  fbi->map_dma_f1);
}

/*****************************************************************************
** -Function:
**    imapfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**		1. Check input video params of 'var'. If a value doesn't fit, round it up. If it's too big,
**		    return -EINVAL.
**
** -Input Param
**    *var	Variable Screen Info. Structure Pointer
**    *info	Framebuffer Structure Pointer
**
** -Output Param
**    *var	Variable Screen Info. Structure Pointer
**
** -Return
**    none
**
*****************************************************************************/ 
static int imapfb_check_var(struct fb_var_screeninfo *var, struct fb_info *info)
{
	if (!var || !info)
	{
		printk(KERN_ERR "[imapfb_check_var]: input argument null\n");
		return -EINVAL;
	}
	
	switch (var->bits_per_pixel)
	{			
		case 8:
			var->red = imapfb_a1rgb232_8.red;
			var->green = imapfb_a1rgb232_8.green;
			var->blue = imapfb_a1rgb232_8.blue;
			var->transp = imapfb_a1rgb232_8.transp;
			imapfb_fimd.bpp = 8;
			imapfb_fimd.bytes_per_pixel = 1;
			break;

		case 16:
			var->red = imapfb_rgb565_16.red;
			var->green = imapfb_rgb565_16.green;
			var->blue = imapfb_rgb565_16.blue;
			var->transp = imapfb_rgb565_16.transp;
			imapfb_fimd.bpp = 16;
			imapfb_fimd.bytes_per_pixel = 2;
			break;
		
		case 18:
			var->red = imapfb_rgb666_18.red;
			var->green = imapfb_rgb666_18.green;
			var->blue = imapfb_rgb666_18.blue;
			var->transp = imapfb_rgb666_18.transp;
			imapfb_fimd.bpp = 18;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 19:
			var->red = imapfb_a1rgb666_19.red;
			var->green = imapfb_a1rgb666_19.green;
			var->blue = imapfb_a1rgb666_19.blue;
			var->transp = imapfb_a1rgb666_19.transp;
			imapfb_fimd.bpp = 19;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 24:
			var->red = imapfb_rgb888_24.red;
			var->green = imapfb_rgb888_24.green;
			var->blue = imapfb_rgb888_24.blue;
			var->transp = imapfb_rgb888_24.transp;
			imapfb_fimd.bpp = 24;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 25:
			var->red = imapfb_a1rgb888_25.red;
			var->green = imapfb_a1rgb888_25.green;
			var->blue = imapfb_a1rgb888_25.blue;
			var->transp = imapfb_a1rgb888_25.transp;
			imapfb_fimd.bpp = 25;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 28:
			var->red = imapfb_a4rgb888_28.red;
			var->green = imapfb_a4rgb888_28.green;
			var->blue = imapfb_a4rgb888_28.blue;
			var->transp = imapfb_a4rgb888_28.transp;
			imapfb_fimd.bpp = 28;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		case 32:
			var->red = imapfb_a8rgb888_32.red;
			var->green = imapfb_a8rgb888_32.green;
			var->blue = imapfb_a8rgb888_32.blue;
			var->transp = imapfb_a8rgb888_32.transp;
			imapfb_fimd.bpp = 32;
			imapfb_fimd.bytes_per_pixel = 4;
			break;

		default:
			printk(KERN_ERR "[imapfb_check_var]: input bits_per_pixel of var invalid\n");
			return -EINVAL;
	}

	info->fix.line_length = imapfb_fimd.bytes_per_pixel * info->var.xres;
		
	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_activate_var(imapfb_info_t *fbi, struct fb_var_screeninfo *var)
**
** -Description:
**    This function implement special features. The process is,
**		1. According to input bits_per_pixel, set new configuration to window control registers.
**
** -Input Param
**    *var	Variable Screen Info. Structure Pointer
**    *fbi		Imap Framebuffer Structure Pointer
**
** -Output Param
**    none
**
** -Return
**    none
**
*****************************************************************************/ 
static void imapfb_activate_var(const struct fb_var_screeninfo *var, imapfb_info_t *fbi)
{
	UINT32 val;

	val = readl(IMAP_OVCW0CR + 0x80 * fbi->win_id);
	val &= IMAP_OVCWxCR_BPPMODE_MSK;
	switch (var->bits_per_pixel)
	{
		case 8:
			val |= IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232;
			break;

		case 16:
			val |= IMAP_OVCWxCR_BPPMODE_16BPP_RGB565;
			break;
			
		case 18:
			val |= IMAP_OVCWxCR_BPPMODE_18BPP_RGB666;
			break;

		case 19:
			val |= IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666;
			break;

		case 24:
			val |= IMAP_OVCWxCR_BPPMODE_24BPP_RGB888;
			break;

		case 25:
			val |= IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888;
			break;

		case 28:
			val |= IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888;
			break;

		case 32:			
			val |= IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888;
			break;

		default:
			printk(KERN_ERR "[imapfb_activate_var]: input bits_per_pixel %d invalid\n", var->bits_per_pixel);
			return;
	}

	switch (fbi->win_id)
	{
		case 0:
			imapfb_fimd.ovcw0cr = val;
			writel(imapfb_fimd.ovcw0cr, IMAP_OVCW0CR);
			break;
		case 1:
			imapfb_fimd.ovcw1cr = val;
			writel(imapfb_fimd.ovcw1cr, IMAP_OVCW1CR);
			break;
		case 2:
			imapfb_fimd.ovcw2cr = val;
			writel(imapfb_fimd.ovcw2cr, IMAP_OVCW2CR);
			break;
		case 3:
			imapfb_fimd.ovcw3cr = val;
			writel(imapfb_fimd.ovcw3cr, IMAP_OVCW3CR);
			break;
		default:
			printk(KERN_ERR "[imapfb_activate_var]: input win id %d invalid\n", fbi->win_id);
			return;
	}
}

/*****************************************************************************
** -Function:
**    imapfb_set_par(struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**		1. According to input framebuffer struct, set new var struct value and set these new
**		    values to relevant registers.
**
** -Input Param
**    *info	Framebuffer Structure Pointer
**
** -Output Param
**	*info	Framebuffer Structure Pointer
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/ 
static int imapfb_set_par(struct fb_info *info)
{	
	struct fb_var_screeninfo *var = NULL;
	imapfb_info_t *fbi = (imapfb_info_t*) info;
	int ret = 0;

	if (!info)
	{
		printk(KERN_ERR "[imapfb_set_par]: input argument null\n");
		return -EINVAL;
	}
	
	var = &info->var;
	
	//Set Visual Color Type
	if (var->bits_per_pixel == 8 || var->bits_per_pixel == 16 || var->bits_per_pixel == 18 || var->bits_per_pixel == 19
		|| var->bits_per_pixel == 24 || var->bits_per_pixel == 25 || var->bits_per_pixel == 28 || var->bits_per_pixel == 32)
		fbi->fb.fix.visual = FB_VISUAL_TRUECOLOR;
	else if (var->bits_per_pixel == 1 || var->bits_per_pixel == 2 || var->bits_per_pixel == 4)
		fbi->fb.fix.visual = FB_VISUAL_PSEUDOCOLOR;
	else
	{
		printk(KERN_ERR "[imapfb_set_par]: input bits_per_pixel invalid\n");
		ret = -EINVAL;
		goto out;
	}

	//Check Input Params
	ret = imapfb_check_var(var, info);
	if (ret)
	{
		printk(KERN_ERR "[imapfb_set_par]: fail to check var\n");
		ret = -EINVAL;
		goto out;
	}	

	//Activate New Configuration
	imapfb_activate_var(var, fbi);

out:
	return ret;
}

/*****************************************************************************
** -Function:
**    imapfb_set_fb_addr(const imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**        1. According to virtual offset in both direction, set new start address of input
**		window in framebuffer ram which equals framebuffer start address plus offset.
**
** -Input Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Output Param
**    none
**
** -Return
**    none
**
*****************************************************************************/
static void imapfb_set_fb_addr(const imapfb_info_t *fbi)
{
	UINT32 video_phy_temp_f1 = fbi->map_dma_f1;	/* framebuffer start address */
	UINT32 start_address;						/* new start address */
	UINT32 start_offset;									/* virtual offset */

	//Get Start Address Offset
	start_offset = (fbi->fb.var.xres_virtual * fbi->fb.var.yoffset + fbi->fb.var.xoffset) * imapfb_fimd.bytes_per_pixel;

	//New Start Address with Offset
	start_address = video_phy_temp_f1 + start_offset;

	//Set New Start Address for Input Framebuffer Window
	switch (fbi->win_id)
	{
		case 0:
			imapfb_fimd.ovcw0b0sar = start_address;
			writel(imapfb_fimd.ovcw0b0sar, IMAP_OVCW0B0SAR);
	        	break;

		case 1:
			imapfb_fimd.ovcw1b0sar = start_address;
			writel(imapfb_fimd.ovcw1b0sar, IMAP_OVCW1B0SAR);
			break;

		case 2:
			imapfb_fimd.ovcw2b0sar = start_address;
			writel(imapfb_fimd.ovcw2b0sar, IMAP_OVCW2B0SAR);
		        break;

		case 3:
			imapfb_fimd.ovcw3bsar = start_address;
			writel(imapfb_fimd.ovcw3bsar, IMAP_OVCW3BSAR);
			break;
			
		default:
			printk(KERN_ERR "[imapfb_set_fb_addr]: input win id %d invalid\n", fbi->win_id);
			break;
	}
}

/*****************************************************************************
** -Function:
**    imapfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**        1. Pan the display using 'xoffset' and 'yoffset' fields of input var structure.
**
** -Input Param
**	*var		Variable Screen Parameter Structure
**	*info	Framebuffer Structure
**
** -Output Param
**    *info	Framebuffer Structure
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_pan_display(struct fb_var_screeninfo *var, struct fb_info *info)
{
	imapfb_info_t *fbi = (imapfb_info_t*)info;
	int ret;

	if (!var || !info)
	{
		printk(KERN_ERR "[imapfb_pan_display]: input arguments null\n");
		return -EINVAL;
	}

	if (var->xoffset + info->var.xres > info->var.xres_virtual)
	{
		printk(KERN_ERR "[imapfb_pan_display]: pan display out of range in horizontal direction\n");
		return -EINVAL;
	}

	if (var->yoffset + info->var.yres > info->var.yres_virtual)
	{
		printk(KERN_ERR "[imapfb_pan_display]: pan display out of range in vertical direction\n");
		return -EINVAL;
	}

	fbi->fb.var.xoffset = var->xoffset;
	fbi->fb.var.yoffset = var->yoffset;

	//Check Input Params
	ret = imapfb_check_var(&fbi->fb.var, info);
	if (ret)
	{
		printk(KERN_ERR "[imapfb_pan_display]: fail to check var\n");
		return -EINVAL;
	}

	imapfb_set_fb_addr(fbi);
	/* FIXME:Delay for LCD frame synchronization */
	udelay(300);

#ifdef CONFIG_LCD_ENABLE_IRQ
	if (var->activate & FB_ACTIVATE_VBL)
	{
		irq_mark = 0;
		wait_event_interruptible_timeout(wait_fb, \
				irq_mark == 1, \
				HZ / 40);
	}
#endif
	return 0;
}

#ifdef CONFIG_LCD_ENABLE_IRQ
/*
 * LCD framebuffer interrupt handle function, it's really not a good idea
 * to setup a interrupt handle function for framebuffer driver. But for 
 * better use in user space OSD lib delay for interrupt to proctect buffer
 * to be displayed. In this interrupt handle function, I first wake up 
 * wait queue, then clear pending bits just to disable interrupt reload.
 */
static irqreturn_t imapfb_irq_handle(int irq, void *dev_id)
{
	volatile unsigned int ival = 0;	/* Interrupt pending register value */
	volatile unsigned int sval = 0;	/* Source pending register value */

	/* printk(KERN_ALERT "@@@ [LCD DEBUG] Get int framebuffer interrupt @@@\n"); */

	ival = reg_fb->pending;
	sval = reg_fb->source;

	if(((ival & 0x01) == 1) || ((sval & 0x01) == 1))
	{
		/* Set irq mark to be 1 */
		if(irq_mark == 0)
		{
			/*
			 * Only in correspond ioctl command, irq_mark will be set
			 * 0, and wait there, this will wake up the ioctl.
			 */
			irq_mark = 1;
			wake_up_interruptible(&wait_fb);
		}

		ival |= 0x01;
		sval |= 0x01;
		reg_fb->pending = ival;
		reg_fb->source = sval;
	}
	else 
	{
		/* This should be an unexpected case, clear all interrupt and source pending bits */
		printk(KERN_ERR "[LCD ERR] Unexpected interrupt generated\n");

		/* Clear all pending bits */
		ival |= 0xfe;
		sval |= 0xfe;
		reg_fb->pending = ival;
		reg_fb->source = sval;
	}

	return IRQ_HANDLED;
}
#endif	/* CONFIG_LCD_ENABLE_IRQ */

/*****************************************************************************
** -Function:
**    imapfb_blank(int blank_mode, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**        1. According to input blank mode, Lcd display blank mode in screen.
**		a. No Blank
**		b. Vsync Suspend
**		c. Hsync Suspend
**		d. Power Down
**
** -Input Param
**	blank_mode	Blank Mode
**	*info		Framebuffer Structure
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_blank(int blank_mode, struct fb_info *info)
{
	switch (blank_mode)
	{
		/* Lcd on, Backlight on */
		case VESA_NO_BLANKING:
			imapfb_set_lcd_power(1);
			imapfb_set_backlight_power(1);
			break;

		/* Lcd on, Backlight off */
		case VESA_VSYNC_SUSPEND:
		case VESA_HSYNC_SUSPEND:
			break;

		/* Lcd and Backlight off */
		case VESA_POWERDOWN:
			imapfb_set_lcd_power(0);
			imapfb_set_backlight_power(0);
			break;

		default:
			printk(KERN_ERR "[imapfb_blank]: input blank mode %d invalid\n", blank_mode);
			return -EINVAL;
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_onoff_win(const imapfb_info_t *fbi, UINT32 onoff)
**
** -Description:
**    This function implement special features. The process is,
**		1. Open/close one window
**
** -Input Param
**    *fbi		Imap Framebuffer Structure Pointer
**    onoff	Open/Close
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static inline int imapfb_onoff_win(const imapfb_info_t *fbi, UINT32 onoff)
{
	UINT32 win_num =  fbi->win_id;

	if (win_num >= IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_onoff_win]: input win id %d invalid\n", fbi->win_id);
		return -EINVAL;
	}

	if (onoff)
		writel(readl(IMAP_OVCW0CR + (0x80 * win_num)) | IMAP_OVCWxCR_ENWIN_ENABLE, IMAP_OVCW0CR + (0x80 * win_num));
	else
		writel(readl(IMAP_OVCW0CR + (0x80 * win_num)) & ~IMAP_OVCWxCR_ENWIN_ENABLE, IMAP_OVCW0CR + (0x80 * win_num));

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_stop_lcd(void)
**
** -Description:
**    This function implement special features. The process is,
**		1. Display controller stop outputting video signal
**
** -Input Param
**    none
**
** -Output Param
**    none
**
** -Return
**	none
**
*****************************************************************************/
static inline void imapfb_stop_lcd(void)
{
	unsigned long flags;
	UINT32 tmp;

	local_irq_save(flags);

	//Disable Video Output
	tmp = readl(IMAP_LCDCON1)&IMAP_LCDCON1_SAVED_MASK;
	writel(tmp & ~IMAP_LCDCON1_ENVID_ENABLE, IMAP_LCDCON1);

	local_irq_restore(flags);
}

/*****************************************************************************
** -Function:
**    imapfb_start_lcd(void)
**
** -Description:
**    This function implement special features. The process is,
**		1. Display controller start outputting video signal
**
** -Input Param
**    none
**
** -Output Param
**    none
**
** -Return
**	none
**
*****************************************************************************/
static inline void imapfb_start_lcd(void)
{
	unsigned long flags;
	UINT32 tmp;

	local_irq_save(flags);

	//Enable Video Output
	tmp = readl(IMAP_LCDCON1)&IMAP_LCDCON1_SAVED_MASK;
	writel(tmp | IMAP_LCDCON1_ENVID_ENABLE, IMAP_LCDCON1);

	local_irq_restore(flags);
}

/*****************************************************************************
** -Function:
**    imapfb_update_palette(imapfb_info_t *fbi, UINT32 regno, UINT32 val)
**
** -Description:
**    This function implement special features. The process is,
**        1. Update real palette
**		a. 256 entries in paletter of win 0 and 1
**		b. only 16 entries in paletter of win 2 and 3
**
** -Input Param
**    *fbi		Imap Framebuffer Structure Pointer
**    regno	Number of Palette Register
**    val		Color Value of Palette Register
**
** -Output Param
**    *fbi		Imap Framebuffer Structure Pointer
**
** -Return
**	none
**
*****************************************************************************/
static void imapfb_update_palette(imapfb_info_t *fbi, UINT32 regno, UINT32 val)
{
	unsigned long flags;
	UINT32 win_num;

	if (!fbi)
	{
		printk(KERN_ERR "[imapfb_update_palette]: input fbi null\n");
		return;
	}

	win_num = fbi->win_id;

	if ((2 == win_num || 3 == win_num) && regno >= 16)
	{
		printk(KERN_ERR "[imapfb_update_palette]: input register number out of range.\n");
		return;
	}

	local_irq_save(flags);

	fbi->palette_buffer[regno] = val;
	writel(val, IMAP_OVCW0PAL + 0x400 * win_num + 0x4 * regno);

	local_irq_restore(flags);
}

static inline UINT32 imapfb_chan_to_field(UINT32 chan, struct fb_bitfield bf)
{
#if defined (CONFIG_FB_IMAP_BPP16)
	chan &= 0xffff;
	chan >>= 16 - bf.length;
#elif defined (CONFIG_FB_IMAP_BPP32)
	chan &= 0xffffffff;
	chan >>= 32 - bf.length;
#endif

	return chan << bf.offset;
}

/*****************************************************************************
** -Function:
**    imapfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
**		unsigned int blue, unsigned int transp, struct fb_info *info)
**
** -Description:
**    This function implement special features. The process is,
**        1. Set color register
**		a. Fake palette for true color: for some special use
**		b. Real palette for paletter color: need to modify registers
**
** -Input Param
**	regno	Number of Color Register
**	red		Red Part of input Color
**	green	Green Part of input Color
**	blue		Blue Part of input Color
**	transp	Transparency Part of input Color
**	*info	Framebuffer Structure
**
** -Output Param
**    *info	Framebuffer Structure
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_setcolreg(unsigned int regno, unsigned int red, unsigned int green,
	unsigned int blue, unsigned int transp, struct fb_info *info)
{
	imapfb_info_t *fbi = (imapfb_info_t*)info;
	UINT32 val;

	if (!fbi)
	{
		printk(KERN_ERR "[imapfb_setcolreg]: input info null\n");
		return -EINVAL;
	}

	switch (fbi->fb.fix.visual)
	{
		/* Modify Fake Palette of 16 Colors */ 
		case FB_VISUAL_TRUECOLOR:
			//if (regno < 16)
			if (regno < 256)	/* Modify Fake Palette of 256 Colors */ 
			{				
				unsigned int *pal = fbi->fb.pseudo_palette;

				val = imapfb_chan_to_field(red, fbi->fb.var.red);
				val |= imapfb_chan_to_field(green, fbi->fb.var.green);
				val |= imapfb_chan_to_field(blue, fbi->fb.var.blue);
				val |= imapfb_chan_to_field(transp, fbi->fb.var.transp);			

				pal[regno] = val;
			}
			else
			{
				printk(KERN_ERR "[imapfb_setcolreg]: input register number %d invalid\n", regno);
				return -EINVAL;
			}
			break;

		/* Modify Real Palette of 256 Colors */
		case FB_VISUAL_PSEUDOCOLOR:
			if (regno < 256)
			{
				/* Only Support RGB565 */
				val = ((red >> 0) & 0xf800);
				val |= ((green >> 5) & 0x07e0);
				val |= ((blue >> 11) & 0x001f);

				imapfb_update_palette(fbi, regno, val);
			}
			else
			{
				printk(KERN_ERR "[imapfb_setcolreg]: input register number %d invalid\n", regno);
				return -EINVAL;
			}			
			break;

		default:
			printk(KERN_ERR "[imapfb_setcolreg]: unknown color type\n");
			return -EINVAL;
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_set_win_coordinate(imapfb_info_t *fbi, UINT32 left_x, UINT32 top_y,
**		UINT32 width, UINT32 height)
**
** -Description:
**    This function implement special features. The process is,
**        1. Set visual window coordinate and size
**        2. Modify visual window offset relative to virtual framebuffer
**
** -Input Param
**	*fbi		Imap Framebuffer Structure Pointer
**	left_x	Coordinate in x-axis of visual window left top corner (unit in pixel)
**	top_y	Coordinate in y-axis of visual window left top corner (unit in pixel)
**	width	Width in x-axis of visual window (unit in pixel)
**	height	Height in y-axis of visual window (unit in pixel)
**
** -Output Param
**    *fbi		Imap Framebuffer Structure Pointer
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_set_win_coordinate(imapfb_info_t *fbi, UINT32 left_x, UINT32 top_y, UINT32 width, UINT32 height)
{
	struct fb_var_screeninfo *var= &fbi->fb.var;	
	UINT32 win_num = fbi->win_id;

	if (win_num >= IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_onoff_win]: input win id %d invalid\n", fbi->win_id);
		return -EINVAL;
	}

	writel(IMAP_OVCWxPCAR_LEFTTOPX(left_x) | IMAP_OVCWxPCAR_LEFTTOPY(top_y), IMAP_OVCW0PCAR + 0x80 * win_num);
	writel(IMAP_OVCWxPCBR_RIGHTBOTX(left_x + width - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(top_y + height - 1), IMAP_OVCW0PCBR + 0x80 * win_num);

	var->xoffset += left_x;
	var->yoffset += top_y;
	var->xres = width;
	var->yres = height;

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_set_win_pos_size(imapfb_info_t *fbi, SINT32 left_x, SINT32 top_y,
**		UINT32 width, UINT32 height)
**
** -Description:
**    This function implement special features. The process is,
**        1. Adjust visual window coordinate and size to proper value and set them
**        2. Modify visual window offset relative to virtual framebuffer
**        3. Modify start address of visual window in framebuffer memory
**
** -Input Param
**	*fbi		Imap Framebuffer Structure Pointer
**	left_x	Coordinate in x-axis of visual window left top corner (unit in pixel)
**	top_y	Coordinate in y-axis of visual window left top corner (unit in pixel)
**	width	Width in x-axis of visual window (unit in pixel)
**	height	Height in y-axis of visual window (unit in pixel)
**
** -Output Param
**    *fbi		Imap Framebuffer Structure Pointer
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_set_win_pos_size(imapfb_info_t *fbi, SINT32 left_x, SINT32 top_y, UINT32 width, UINT32 height)
{
	UINT32 xoffset, yoffset;
	int ret = 0;
	
	if (left_x >= imapfb_fimd.xres)
	{
		printk(KERN_ERR "[imapfb_set_win_pos_size]: input left_x invalid, beyond screen width\n");
		return -EINVAL;
	}

	if (top_y >= imapfb_fimd.yres)
	{
		printk(KERN_ERR "[imapfb_set_win_pos_size]: input top_y invalid, beyond screen height\n");
		return -EINVAL;
	}

	if (left_x + width <= 0)
	{
		printk(KERN_ERR "[imapfb_set_win_pos_size]: input left_x and width too small, out of screen width\n");
		return -EINVAL;
	}

	if (top_y + height <= 0)
	{
		printk(KERN_ERR "[imapfb_set_win_pos_size]: input top_y and height too small, out of screen height\n");
		return -EINVAL;
	}
	
	if (left_x < 0)
	{		
		width = left_x + width;
		xoffset = -left_x;
		left_x = 0;
	}
	
	if (top_y < 0)
	{		
		height = top_y + height;
		yoffset = -top_y;
		top_y = 0;
	}
	
	if (left_x + width > imapfb_fimd.xres)
	{
		width = imapfb_fimd.xres - left_x;
	}
	
	if (top_y + height > imapfb_fimd.yres)
	{
		height = imapfb_fimd.yres - top_y;
	}

	ret = imapfb_set_win_coordinate(fbi, left_x, top_y, width, height);
	if (ret)
	{
		printk(KERN_ERR "[imapfb_set_win_pos_size]: fail to set win%d coordinate\n", fbi->win_id);
		return ret;
	}

	//Check Input Params
	ret = imapfb_check_var(&fbi->fb.var, &fbi->fb);
	if (ret)
	{
		printk(KERN_ERR "[imapfb_set_win_pos_size]: fail to check var\n");
		return ret;
	}
	
	imapfb_set_fb_addr(fbi);

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_set_alpha_level(const imapfb_info_t *fbi, UINT32 level, UINT32 alpha_index)
**
** -Description:
**    This function implement special features. The process is,
**        1. According to input alpha index, set alpha value of red, green, blue three colors separately
**
** -Input Param
**	*fbi			Imap Framebuffer Structure Pointer
**	level			Alpha Value (0~15)
**	alpha_index	Alpha Index (0/1)
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_set_alpha_level(const imapfb_info_t *fbi, UINT32 level, UINT32 alpha_index)
{
	UINT32 alpha_val;
	UINT32 win_num = fbi->win_id;

	if (!win_num || win_num > IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_set_alpha_level]: input win id %d invalid\n", win_num);
		return -EINVAL;
	}

	alpha_val = readl(IMAP_OVCW1PCCR + 0x80 * (win_num - 1));

	if (alpha_index)
	{
		alpha_val &= IMAP_OVCWxPCCR_ALPHA1_R_MSK;
		alpha_val &= IMAP_OVCWxPCCR_ALPHA1_G_MSK;
		alpha_val &= IMAP_OVCWxPCCR_ALPHA1_B_MSK;
		alpha_val |= IMAP_OVCWxPCCR_ALPHA1_R(level) | IMAP_OVCWxPCCR_ALPHA1_G(level) | IMAP_OVCWxPCCR_ALPHA1_B(level);
	}
	else
	{
		alpha_val &= IMAP_OVCWxPCCR_ALPHA0_R_MSK;
		alpha_val &= IMAP_OVCWxPCCR_ALPHA0_G_MSK;
		alpha_val &= IMAP_OVCWxPCCR_ALPHA0_B_MSK;
		alpha_val |= IMAP_OVCWxPCCR_ALPHA0_R(level) | IMAP_OVCWxPCCR_ALPHA0_G(level) | IMAP_OVCWxPCCR_ALPHA0_B(level);
	}

	writel(alpha_val, IMAP_OVCW1PCCR + 0x80 * (win_num - 1));

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_onoff_color_key(const imapfb_info_t *fbi, UINT32 onoff)
**
** -Description:
**    This function implement special features. The process is,
**        1. Enable/disable color key function of input window
**
** -Input Param
**	*fbi		Imap Framebuffer Structure Pointer
**	onoff	Enable/disable
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_onoff_color_key(const imapfb_info_t *fbi, UINT32 onoff)
{
	UINT32 win_num =  fbi->win_id;
	UINT32 v1, v2;

	if (!win_num || win_num > IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_onoff_color_key]: input win id %d invalid\n", win_num);
		return -EINVAL;
	}

	if (win_num != 3)
		v1 = readl(IMAP_OVCW1CKCR + 0x80 * (win_num - 1));
	else
		v1 = readl(IMAP_OVCW3CKCR);

	v2 = readl(IMAP_OVCW0CR + 0x80 * win_num);

	if (onoff)
	{
		v1 |= IMAP_OVCWxCKCR_KEYEN_ENABLE;
		v2 &= ~IMAP_OVCWxCR_ALPHA_SEL_1;
		v2 |= IMAP_OVCWxCR_BLD_PIX_PIXEL;
	}
	else
	{
		v1 &= ~IMAP_OVCWxCKCR_KEYEN_ENABLE;
		v2 &= ~IMAP_OVCWxCR_BLD_PIX_PIXEL;
	}

	writel(v2, IMAP_OVCW0CR + 0x80 * win_num);

	if (win_num != 3)
		writel(v1, IMAP_OVCW1CKCR + 0x80 * (win_num - 1));
	else
		writel(v1, IMAP_OVCW3CKCR);

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_onoff_color_key_alpha_blending(imapfb_info_t *fbi, UINT32 onoff)
**
** -Description:
**    This function implement special features. The process is,
**        1. Start/stop alpha blending in color key function
**
** -Input Param
**	*fbi		Imap Framebuffer Structure Pointer
**	onoff	Start/stop alpha blending
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_onoff_color_key_alpha_blending(const imapfb_info_t *fbi, UINT32 onoff)
{
	UINT32 win_num =  fbi->win_id;
	UINT32 val;

	if (!win_num || win_num > IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_onoff_color_key_alpha_blending]: input win id %d invalid\n", win_num);
		return -EINVAL;
	}

	if (win_num != 3)
		val = readl(IMAP_OVCW1CKCR + 0x80 * (win_num - 1));
	else
		val = readl(IMAP_OVCW3CKCR);

	if (onoff)
		val |= IMAP_OVCWxCKCR_KEYBLEN_ENABLE;
	else
		val &= ~IMAP_OVCWxCKCR_KEYBLEN_ENABLE;

	if (win_num != 3)
		writel(val, IMAP_OVCW1CKCR + 0x80 * (win_num - 1));
	else
		writel(val, IMAP_OVCW3CKCR);

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_set_color_key_info(const imapfb_info_t *fbi, imapfb_color_key_info_t *colkey_info)
**
** -Description:
**    This function implement special features. The process is,
**        1. Set all the info. which color key function need, including compare key, color value,
**		and match fore-ground pixel or back-ground pixel
**
** -Input Param
**	*fbi			Imap Framebuffer Structure Pointer
**	*colkey_info	Color Key Info. Structure Pointer
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_set_color_key_info(const imapfb_info_t *fbi, const imapfb_color_key_info_t *colkey_info)
{
	UINT32 compkey = 0, colval = 0;
	UINT32 win_num =  fbi->win_id;
	UINT32 val;

	if (!win_num || win_num > IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_set_color_key_info]: input win id %d invalid\n", win_num);
		return -EINVAL;
	}

	colval = colkey_info->colval;
	switch (fbi->fb.var.bits_per_pixel)
	{
		case 16:	/* RGB565 */
			compkey = 0x70307;
			break;

		case 18:	/* RGB666 */
			compkey = 0x30303;
			break;

		case 24:	/* RGB888 */
			compkey = 0;
			break;

		default:
			printk(KERN_ERR "[imapfb_set_color_key_info]: input bpp mode %d invalid\n", fbi->fb.var.bits_per_pixel);
			return -EINVAL;
	}

	if (IMAPFB_COLOR_KEY_DIR_BG == colkey_info->direction)
		val = IMAP_OVCWxCKCR_COMPKEY(compkey) | IMAP_OVCWxCKCR_DIRCON_BACKGROUND;
	else if (IMAPFB_COLOR_KEY_DIR_FG == colkey_info->direction)
		val = IMAP_OVCWxCKCR_COMPKEY(compkey) | IMAP_OVCWxCKCR_DIRCON_FOREGROUND;
	else
	{
		printk(KERN_ERR "[imapfb_set_color_key_info]: color key direction is not correct\n");
		return -EINVAL;
	}

	if (win_num != 3)
	{
		writel(val, IMAP_OVCW1CKCR + 0x80 * (win_num - 1));
		writel(colval, IMAP_OVCW1CKR + 0x80 * (win_num - 1));
	}
	else
	{
		writel(val, IMAP_OVCW3CKCR);
		writel(colval, IMAP_OVCW3CKR);
	}

	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_do_ddc_edid(struct i2c_adapter *adapter)
**
** -Description:
**    This function implement special features. The process is,
**        1. Get edid info. through input i2c adapter
**
** -Input Param
**	*adapter		I2C Adapter (which denote an i2c bus)
**
** -Output Param
**    none
**
** -Return
**	buf[UINT8*]	Buffer which store edid info.
**	NULL		Failure
**
*****************************************************************************/
#define DDC_ADDR	0x50

static UINT8 *imapfb_do_ddc_edid(struct i2c_adapter *adapter)
{
	UINT8 start = 0x0;
	UINT8 *buf = kmalloc(EDID_LENGTH, GFP_KERNEL);
	struct i2c_msg msgs[] = {
		{
			.addr	= DDC_ADDR,
			.flags	= 0,
			.len		= 1,
			.buf		= &start,
		}, {
			.addr	= DDC_ADDR,
			.flags	= I2C_M_RD,
			.len		= EDID_LENGTH,
			.buf		= buf,
		}
	};

	if (!buf)
	{
		printk(KERN_ERR "[imapfb_do_ddc_edid]: unable to allocate memory for EDID.\n");
		return NULL;
	}

	if (i2c_transfer(adapter, msgs, 2) == 2)
		return buf;

	printk(KERN_INFO "[imapfb_do_ddc_edid]: unable to read EDID block.\n");

	kfree(buf);
	
	return NULL;
}

/*****************************************************************************
** -Function:
**    imapfb_edid_checksum(UINT8 *edid)
**
** -Description:
**    This function implement special features. The process is,
**        1. Calculate crc check sum of 128-byte edid info.
**
** -Input Param
**	*edid		Edid Buffer Pointer
**
** -Output Param
**    none
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_edid_checksum(UINT8 *edid)
{
	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_get_lcd_edid_info(UINT8 *edid_info)
**
** -Description:
**    This function implement special features. The process is,
**		1. Get lcd edid info. through i2c bus.
**
** -Input Param
**    *edid_info	Edid Info. Pointer
**
** -Output Param
**    *edid_info	Edid Info. Pointer
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_get_lcd_edid_info(UINT8 *edid_info)
{
	struct i2c_adapter *adapter;
	UINT8 *edid = NULL;
	UINT32 retries = 3;

	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		printk(KERN_ERR "[imapfb_get_lcd_edid_info]: can't get i2c adapter\n");
		return -1;
	}

retry_edid:	
	edid = imapfb_do_ddc_edid(adapter);

	if (!edid)
	{
		printk(KERN_ERR "[imapfb_get_lcd_edid_info]: can't get display edid info.\n");
		i2c_put_adapter(adapter);
		return -1;
	}

	//Check checksum, if right, then continue, otherwise get EDID again.
	if (imapfb_edid_checksum(edid))
	{
		printk(KERN_WARNING "[imapfb_get_lcd_edid_info]: EDID checksum fail and retry to get EDID info.\n");
		if (retries > 0)
		{
			retries--;
			goto retry_edid;
		}
		else
		{
			printk(KERN_ERR "[imapfb_get_lcd_edid_info]: display edid info check summary fail.\n");
			i2c_put_adapter(adapter);
			return -1;
		}
	}

	memcpy(edid_info, edid, EDID_LENGTH);

	i2c_put_adapter(adapter);
	return 0;
}

/*****************************************************************************
** -Function:
**    imapfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
**
** -Description:
**    This function implement special features. The process is,
**        1. According input command code, perform some operations
**		a. Set the position and size of some window
**		b. Open/close some window
**		c. Set value of alpha0/1 of some window
**		d. Start/stop color key function of some window
**		e. Start/stop alpha blending in color key function of some window
**		f. Set color key info. of some window
**		g. Power up/down lcd screen
**		h. Get edid info. of lcd screen
**		i. Detect lcd connection
**		j. Get framebuffer physical address info. of some window
**
** -Input Param
**    *info	Framebuffer Structure
**    cmd		Command Code
**    arg		Command Argument
**
** -Output Param
**    *info	Framebuffer Structure
**    arg		Command Argument
**
** -Return
**	0		Success
**	others	Failure
**
*****************************************************************************/
static int imapfb_ioctl(struct fb_info *info, unsigned int cmd, unsigned long arg)
{
	imapfb_info_t *fbi = (imapfb_info_t*)info;
	imapfb_win_pos_size win_info;
	UINT32 alpha_level;
	imapfb_color_key_info_t colkey_info;
	UINT8 edid_info[EDID_LENGTH];
	imapfb_dma_info_t fb_dma_addr_info;
	int ret = 0;
	unsigned int osdlevel	= 0;	/* added by sololz, get osd level to be flushed passed by user space ioctl */
	unsigned int *tmp_reg	= NULL;
	unsigned int tmp_val	= 0;

	if (!fbi)
	{
		printk(KERN_ERR "[imapfb_ioctl]: input info null\n");
		return -EINVAL;
	}
		
	switch(cmd)
	{
#ifdef CONFIG_LCD_ENABLE_IRQ	/* this ioctl added by sololz */
		case IMAPFB_OSD_WAIT_IRQ:

			mutex_lock(&mark_mutex);
			irq_mark = 0;
			mutex_unlock(&mark_mutex);

			/* 
			 * jiffies is set to be (HZ / 50), it's supposed to be 20ms.
			 * This timeout parameter is jiffies, what HZ means that jiffies
			 * increase count in one second.
			 */
			wait_event_interruptible_timeout(wait_fb, \
					irq_mark == 1, \
					HZ / 50);

			break;
#endif	/* CONFIG_LCD_ENABLE_IRQ */

		case IMAPFB_OSD_FLUSH_LEVEL:
			/*
			 * this ioctl command added by sololz, and i suppose caller is sure that
			 * the level which to be flushed and if any other is still using correspond
			 * osd level, he will certainly get errors. so be aware.
			 */

			if(copy_from_user(&osdlevel, (unsigned int *)arg, 4))
			{
				printk(KERN_ERR "[imapfb_ioctl]: copy win%d osd level info. from user space error\n", fbi->win_id);
				return -EFAULT;
			}

			if(osdlevel > 3)
			{
				printk(KERN_ERR "[imapfb_ioctl]: input overlay number error, %d\n", osdlevel);
				return -EINVAL;
			}

			/* reset osd level */
			tmp_reg = (unsigned int *)((unsigned int)overlay_reg + (0x80 * (osdlevel + 1)));
			tmp_val = tmp_reg[0];
			tmp_reg[0] = tmp_val & (~0x00000001);

			break;

		case IMAPFB_OSD_SET_POS_SIZE:
			if (copy_from_user(&win_info, (imapfb_win_pos_size *)arg, sizeof(imapfb_win_pos_size)))
			{
				printk(KERN_ERR "[imapfb_ioctl]: copy win%d position and size info. from user space error\n", fbi->win_id);
				return -EFAULT;
			}

			ret = imapfb_set_win_pos_size(fbi, win_info.left_x, win_info.top_y, win_info.width, win_info.height);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: set win%d position and size error\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;
		case IMAPFB_OSD_START:
			ret = imapfb_onoff_win(fbi, 1);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: fail to start win%d\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;
		case IMAPFB_OSD_STOP:
			ret = imapfb_onoff_win(fbi, 0);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: fail to stop win%d\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;
		case IMAPFB_OSD_ALPHA0_SET:
			alpha_level = (UINT32)arg;

			if (alpha_level > IMAPFB_MAX_ALPHA_LEVEL)
				alpha_level = IMAPFB_MAX_ALPHA_LEVEL;

			ret = imapfb_set_alpha_level(fbi, alpha_level, 0);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: fail to set win%d alpha0\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;

		case IMAPFB_OSD_ALPHA1_SET:
			alpha_level = (UINT32)arg;

			if (alpha_level > IMAPFB_MAX_ALPHA_LEVEL)
				alpha_level = IMAPFB_MAX_ALPHA_LEVEL;

			ret = imapfb_set_alpha_level(fbi, alpha_level, 1);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: fail to set win%d alpha1\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;
		case IMAPFB_COLOR_KEY_START:
			ret = imapfb_onoff_color_key(fbi, ON);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: win%d color key start error\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;

		case IMAPFB_COLOR_KEY_STOP:
			ret = imapfb_onoff_color_key(fbi, OFF);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: win%d color key stop error\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;

		case IMAPFB_COLOR_KEY_ALPHA_START:
			ret = imapfb_onoff_color_key_alpha_blending(fbi, ON);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: win%d color key alpha blending start error\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;

		case IMAPFB_COLOR_KEY_ALPHA_STOP:
			ret = imapfb_onoff_color_key_alpha_blending(fbi, OFF);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: win%d color key alpha blending stop error\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;
	
		case IMAPFB_COLOR_KEY_SET_INFO:
			if (copy_from_user(&colkey_info, (imapfb_color_key_info_t *) arg, sizeof(imapfb_color_key_info_t)))
			{
				printk(KERN_ERR "[imapfb_ioctl]: copy win%d color key info. from user space error\n", fbi->win_id);
				return -EFAULT;
			}

			ret = imapfb_set_color_key_info(fbi, &colkey_info);
			if (ret)
			{
				printk(KERN_ERR "[imapfb_ioctl]: win%d set color key info. error\n", fbi->win_id);
				ret = -EFAULT;
			}
			break;

		case IMAPFB_POWER_SUPPLY_CHANGE:
			if(lcd_power_status)
				imapfb_power_supply_onoff(0);
			else
				imapfb_power_supply_onoff(1);
			break;

		case IMAPFB_GET_LCD_EDID_INFO:
			if (-1 == imapfb_get_lcd_edid_info(edid_info))
			{
				printk(KERN_ERR "[imapfb_ioctl]: can't get lcd screen edid info.\n");
				return -EFAULT;
			}

			if(copy_to_user((void*)arg, (const void*)edid_info, EDID_LENGTH))
			{
				printk(KERN_ERR "[imapfb_ioctl]: copy lcd screen info. to user space error\n");
				return -EFAULT;
			}
			break;

		case IMAPFB_LCD_DETECT_CONNECT:
			break;

		case IMAPFB_GET_BUF_PHY_ADDR:
			fb_dma_addr_info.map_dma_f1 = fbi->map_dma_f1;
			fb_dma_addr_info.map_dma_f2 = fbi->map_dma_f2;
			fb_dma_addr_info.map_dma_f3 = fbi->map_dma_f3;
			fb_dma_addr_info.map_dma_f4 = fbi->map_dma_f4;

			if (copy_to_user((void *) arg, (const void *)&fb_dma_addr_info, sizeof(imapfb_dma_info_t)))
			{
				printk(KERN_ERR "[imapfb_ioctl]: copy win%d framebuffer physical address info. to user space error\n", fbi->win_id);
				return -EFAULT;
			}

			break;

		default:
			printk(KERN_ERR "[imapfb_ioctl]: unknown command type\n");
			return -EFAULT;
	}

	return 0;
}

/* Framebuffer operations structure */
struct fb_ops imapfb_ops = {
	.owner			= THIS_MODULE,
	.fb_check_var		= imapfb_check_var,
	.fb_set_par		= imapfb_set_par,
	.fb_blank		= imapfb_blank,
	.fb_pan_display		= imapfb_pan_display,
	.fb_setcolreg		= imapfb_setcolreg,
	.fb_fillrect		= cfb_fillrect,
	.fb_copyarea		= cfb_copyarea,
	.fb_imageblit		= cfb_imageblit,
	.fb_ioctl		= imapfb_ioctl,
};

/*****************************************************************************
** -Function:
**    imapfb_init_fbinfo(imapfb_info_t *finfo, char *drv_name, int index)
**
** -Description:
**    This function implement special features. The process is,
**		1. Init framebuffer struct of input imap framebuffer struct, including fix and var struct.
**		2. If input index is 0, then init hardware setting.
**
** -Input Param
**    *fbi        		Imap Framebuffer Structure Pointer
**	drv_name	Driver Name
**	index		Window ID
**
** -Output Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Return
**	none
**
*****************************************************************************/
static void imapfb_init_fbinfo(imapfb_info_t *finfo, UINT8 *drv_name, UINT32 index)
{
	int i = 0;

	if (!finfo)
	{
		printk(KERN_ERR "[imapfb_init_fbinfo]: input finfo null\n");
		return;
	}

	if (index >= IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_init_fbinfo]: input index invalid\n");
		return;
	}

	if (index == 0)
		imapfb_set_gpio();

	strcpy(finfo->fb.fix.id, drv_name);

	finfo->win_id = index;
	finfo->fb.fix.type = FB_TYPE_PACKED_PIXELS;
	finfo->fb.fix.type_aux = 0;
	finfo->fb.fix.xpanstep = 0;
	finfo->fb.fix.ypanstep = 1;
	finfo->fb.fix.ywrapstep = 0;
	finfo->fb.fix.accel = FB_ACCEL_NONE;
	finfo->fb.fix.mmio_start = LCD_BASE_REG_PA;
	finfo->fb.fix.mmio_len = SZ_16K;

	finfo->fb.fbops = &imapfb_ops;
	finfo->fb.flags = FBINFO_FLAG_DEFAULT;

	finfo->fb.pseudo_palette = &finfo->pseudo_pal;

	finfo->fb.var.nonstd = 0;
	finfo->fb.var.activate = FB_ACTIVATE_NOW;
	finfo->fb.var.accel_flags = 0;
	finfo->fb.var.vmode = FB_VMODE_NONINTERLACED;

	finfo->fb.var.xres = imapfb_fimd.osd_xres;
	finfo->fb.var.yres = imapfb_fimd.osd_yres;
	finfo->fb.var.xres_virtual = imapfb_fimd.osd_xres_virtual;
	finfo->fb.var.yres_virtual = imapfb_fimd.osd_yres_virtual * 2;
	finfo->fb.var.xoffset = imapfb_fimd.osd_xoffset;
	finfo->fb.var.yoffset = imapfb_fimd.osd_yoffset;
	
	finfo->fb.var.bits_per_pixel = imapfb_fimd.bpp;
	finfo->fb.var.pixclock = imapfb_fimd.pixclock;
	finfo->fb.var.hsync_len = imapfb_fimd.hsync_len;
	finfo->fb.var.left_margin = imapfb_fimd.left_margin;
	finfo->fb.var.right_margin = imapfb_fimd.right_margin;
	finfo->fb.var.vsync_len = imapfb_fimd.vsync_len;
	finfo->fb.var.upper_margin = imapfb_fimd.upper_margin;
	finfo->fb.var.lower_margin = imapfb_fimd.lower_margin;
	finfo->fb.var.sync = imapfb_fimd.sync;
	finfo->fb.var.grayscale = imapfb_fimd.cmap_grayscale;

	finfo->fb.fix.smem_len = finfo->fb.var.xres_virtual * finfo->fb.var.yres_virtual * imapfb_fimd.bytes_per_pixel;
	finfo->fb.fix.line_length = finfo->fb.var.xres * imapfb_fimd.bytes_per_pixel;

	for (i = 0; i < 256; i++)
		finfo->palette_buffer[i] = IMAPFB_PALETTE_BUFF_CLEAR;
}

/*****************************************************************************
** -Function:
**    imapfb_init_registers(imapfb_info_t *fbi)
**
** -Description:
**    This function implement special features. The process is,
**		1. If input window id is 0, then set clock source divider of display controller to register.
**		2. Set start address of framebuffer of input window to register.
**		3. Set all other registers according to input window id.
**
** -Input Param
**    *fbi        		Imap Framebuffer Structure Pointer
**
** -Output Param
**    *fbi        Imap Framebuffer Structure Pointer
**
** -Return
**    none
**
*****************************************************************************/
static void imapfb_init_registers(const imapfb_info_t *fbi)
{	
	struct fb_var_screeninfo *var = NULL;
	unsigned long flags;
	UINT32 win_num;

	if (!fbi)
	{
		printk(KERN_ERR "[imapfb_init_registers]: input argument null\n");
		return;
	}
	else
	{
		var = (struct fb_var_screeninfo *)&(fbi->fb.var);
		win_num =  fbi->win_id;
	}

	if (win_num >= IMAPFB_NUM)
	{
		printk(KERN_ERR "[imapfb_init_registers]: input win id %d invalid\n", win_num);
		return;
	}	

	local_irq_save(flags);

	if (0 == win_num)
	{
		imapfb_fimd.lcdcon1 = imapfb_fimd.lcdcon1 & ~IMAP_LCDCON1_ENVID_ENABLE;
		writel(imapfb_fimd.lcdcon1, IMAP_LCDCON1);
		//imapfb_fimd.lcdcon1 &= IMAP_LCDCON1_CLKVAL_MSK;
		//imapfb_fimd.lcdcon1 |= IMAP_LCDCON1_CLKVAL(clk_get_rate(fbi->clk) / (2 * imapfb_fimd.pixclock) - 1);
		//imapfb_fimd.lcdcon1 |= IMAP_LCDCON1_CLKVAL(1);
 	}

	switch (win_num)
	{
		case 0:
			imapfb_fimd.ovcw0b0sar =  fbi->map_dma_f1;
			imapfb_fimd.ovcw0vssr = IMAP_OVCWxVSSR_VW_WIDTH(var->xres_virtual);

			writel(imapfb_fimd.lcdcon1, IMAP_LCDCON1);
			writel(imapfb_fimd.lcdcon2, IMAP_LCDCON2);
			writel(imapfb_fimd.lcdcon3, IMAP_LCDCON3);
			writel(imapfb_fimd.lcdcon4, IMAP_LCDCON4);
			writel(imapfb_fimd.lcdcon5, IMAP_LCDCON5);
			writel(imapfb_fimd.ovcdcr, IMAP_OVCDCR);
			writel(imapfb_fimd.ovcpcr, IMAP_OVCPCR);
			writel(imapfb_fimd.ovcw0cr, IMAP_OVCW0CR);
			writel(imapfb_fimd.ovcw0pcar, IMAP_OVCW0PCAR);
			writel(imapfb_fimd.ovcw0pcbr, IMAP_OVCW0PCBR);
			writel(imapfb_fimd.ovcw0b0sar, IMAP_OVCW0B0SAR);
			writel(imapfb_fimd.ovcw0vssr, IMAP_OVCW0VSSR);
			writel(imapfb_fimd.ovcw0cmr, IMAP_OVCW0CMR);
 
			imapfb_onoff_win(fbi, ON);
			break;

		case 1:
			imapfb_fimd.ovcw1b0sar =  fbi->map_dma_f1;
			imapfb_fimd.ovcw1vssr = IMAP_OVCWxVSSR_VW_WIDTH(var->xres_virtual);
			
			writel(imapfb_fimd.ovcw1cr, IMAP_OVCW1CR);
			writel(imapfb_fimd.ovcw1pcar, IMAP_OVCW1PCAR);
			writel(imapfb_fimd.ovcw1pcbr, IMAP_OVCW1PCBR);
			writel(imapfb_fimd.ovcw1pccr, IMAP_OVCW1PCCR);
			writel(imapfb_fimd.ovcw1b0sar, IMAP_OVCW1B0SAR);
			writel(imapfb_fimd.ovcw1vssr, IMAP_OVCW1VSSR);
			writel(imapfb_fimd.ovcw1cmr, IMAP_OVCW1CMR);

			imapfb_onoff_win(fbi, OFF);
			break;

		case 2:
			imapfb_fimd.ovcw2b0sar =  fbi->map_dma_f1;
			imapfb_fimd.ovcw2vssr = IMAP_OVCWxVSSR_VW_WIDTH(var->xres_virtual);
			
			writel(imapfb_fimd.ovcw2cr, IMAP_OVCW2CR);
			writel(imapfb_fimd.ovcw2pcar, IMAP_OVCW2PCAR);
			writel(imapfb_fimd.ovcw2pcbr, IMAP_OVCW2PCBR);
			writel(imapfb_fimd.ovcw2pccr, IMAP_OVCW2PCCR);
			writel(imapfb_fimd.ovcw2b0sar, IMAP_OVCW2B0SAR);
			writel(imapfb_fimd.ovcw2vssr, IMAP_OVCW2VSSR);
			writel(imapfb_fimd.ovcw2cmr, IMAP_OVCW2CMR);

			imapfb_onoff_win(fbi, OFF);
			break;

		case 3:
			imapfb_fimd.ovcw3bsar =  fbi->map_dma_f1;
			imapfb_fimd.ovcw3vssr = IMAP_OVCWxVSSR_VW_WIDTH(var->xres_virtual);
			
			writel(imapfb_fimd.ovcw3cr, IMAP_OVCW3CR);
			writel(imapfb_fimd.ovcw3pcar, IMAP_OVCW3PCAR);
			writel(imapfb_fimd.ovcw3pcbr, IMAP_OVCW3PCBR);
			writel(imapfb_fimd.ovcw3pccr, IMAP_OVCW3PCCR);
			writel(imapfb_fimd.ovcw3bsar, IMAP_OVCW3BSAR);
			writel(imapfb_fimd.ovcw3vssr, IMAP_OVCW3VSSR);
			writel(imapfb_fimd.ovcw3cmr, IMAP_OVCW3CMR);

			imapfb_onoff_win(fbi, OFF);
			break;
	}

	local_irq_restore(flags);
}

static int __init imapfb_probe(struct platform_device *pdev)
{
	struct resource *res;
	struct fb_info *fbinfo;
	imapfb_info_t *info;
	UINT8 driver_name[] = "imapfb";
	UINT32 index, size;
	int ret;

	//Set backlight enable GPIO register to sleep mode
/*	__raw_writel((__raw_readl(rSLP_GPACON) & (~(0x1<<6))), rSLP_GPACON);
	__raw_writel((__raw_readl(rSLP_GPADAT) | (0x1<<6)), rSLP_GPADAT);
	__raw_writel((__raw_readl(rGPA_SLP_CTRL) | (0x1<<6)), rGPA_SLP_CTRL);*/

	printk(KERN_INFO "Imap Framebuffer Driver Initialization Start!\n");
	
	//Check Input Argument
	if (!pdev)
	{
		printk(KERN_ERR "[imapfb_probe]: input argument null\n");
		return -EINVAL;
	}

	//Allocate one Framebuffer Structure
	fbinfo = framebuffer_alloc(sizeof(imapfb_info_t), &pdev->dev);
	if (!fbinfo)
	{
		printk(KERN_ERR "[imapfb_probe]: fail to allocate framebuffer\n");
		return -ENOMEM;
	}

	platform_set_drvdata(pdev, fbinfo);
	info = fbinfo->par;
	info->dev = &pdev->dev;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if (!res)
	{
		printk(KERN_ERR "[imapfb_probe]: fail to get io registers\n");
		ret = -ENXIO;
		goto dealloc_fb;
	}
	size = (res->end - res->start) + 1;
	info->mem = request_mem_region(res->start, size, pdev->name);
	if (!info->mem)
	{
		printk(KERN_ERR "[imapfb_probe]: fail to get memory region\n");
		ret = -ENOENT;
		goto dealloc_fb;
	}
	info->io = ioremap(res->start, size);
	if (!info->io)
	{
		printk(KERN_ERR "[imapfb_probe]: ioremap of registers fail\n");
		ret = -ENXIO;
		goto release_mem;
	}

	imapfb_set_clk();

	info->clk1 = clk_get(NULL, "lcd");
	if (!info->clk1 || IS_ERR(info->clk1))
	{
		printk(KERN_ERR "[imapfb_probe]: fail to get display lcd controller clock source\n");
		ret =  -ENOENT;
		goto release_io;
	}
	clk_enable(info->clk1);
//	printk(KERN_INFO "IMAP_LCD clock got enabled :: %ld.%03ld Mhz\n", PRINT_MHZ(clk_get_rate(info->clk1)));

	info->clk2 = clk_get(NULL, "osd");
	if (!info->clk2 || IS_ERR(info->clk2))
	{
		printk(KERN_ERR "[imapfb_probe]: fail to get display osd controller clock source\n");
		ret =  -ENOENT;
		goto release_clock1;
	}
	clk_enable(info->clk2);
//	printk(KERN_INFO "IMAP_LCD clock got enabled :: %ld.%03ld Mhz\n", PRINT_MHZ(clk_get_rate(info->clk2)));

//	imapfb_set_backlight_level(0);
	for (index = 0; index < IMAPFB_NUM; index++)
	{
		imapfb_info[index].mem = info->mem;
		imapfb_info[index].io = info->io;
		imapfb_info[index].clk1 = info->clk1;
		imapfb_info[index].clk2 = info->clk2;

		imapfb_init_fbinfo(&imapfb_info[index], driver_name, index);

		/* Initialize video memory */
		ret = imapfb_map_video_memory(&imapfb_info[index]);
		if (ret)
		{
			printk(KERN_ERR "[imapfb_probe]: win %d fail to allocate framebuffer video ram\n", index);
			ret = -ENOMEM;
			goto release_clock2;
		}

		ret = imapfb_check_var(&imapfb_info[index].fb.var, &imapfb_info[index].fb);
		if (ret)
		{
			printk(KERN_ERR "[imapfb_probe]: win %d fail to check var\n", index);
			ret = -EINVAL;
			goto free_video_memory;
		}
		
		imapfb_init_registers(&imapfb_info[index]);
		
		if (index < 2)
		{
			if (fb_alloc_cmap(&imapfb_info[index].fb.cmap, 256, 0) < 0)
			{
				printk(KERN_ERR "[imapfb_probe]: win %d fail to allocate color map\n", index);
				goto free_video_memory;
			}
		}
		else
		{
			if (fb_alloc_cmap(&imapfb_info[index].fb.cmap, 16, 0) < 0)
			{
				printk(KERN_ERR "[imapfb_probe]: win %d fail to allocate color map\n", index);
				goto free_video_memory;
			}
		}
		
		ret = register_framebuffer(&imapfb_info[index].fb);
		if (ret < 0)
		{
			printk(KERN_ERR "[imapfb_probe]: failed to register framebuffer device %d\n", ret);
			goto dealloc_cmap;
		}

		printk(KERN_INFO "fb%d: %s frame buffer device\n", imapfb_info[index].fb.node, imapfb_info[index].fb.fix.id);
	}
	
	imapfb_start_lcd();
    imapfb_set_lcd_power(1);
	imapfb_set_backlight_power(1);
	imapfb_set_backlight_level(IMAPFB_DEFAULT_BACKLIGHT_LEVEL);
	lcd_power_status = 1;

/* ######################################################################### */

	/* map overlay control registers added by sololz */
	overlay_reg = (unsigned int *)ioremap_nocache(0x20cd1000, 4 * 1024);
	if(overlay_reg == NULL)
	{
		printk(KERN_ERR "Map overlay registers base address error\n");
		goto destroy_mutex;
	}

#ifdef CONFIG_LCD_ENABLE_IRQ
	/* Initialize mutex */
	mutex_init(&mark_mutex);

	/* Initialize a wait queue head for wait */
	init_waitqueue_head(&wait_fb);

	/* Asign OSD controller register base address, added by sololz */
	reg_fb = (ids_reg_t *)(info->io);

	/* Register an irq for framebuffer */
	if(request_irq(IRQ_IDS, imapfb_irq_handle, IRQF_DISABLED, \
				pdev->name, pdev) != 0)
	{
		printk(KERN_ERR "Create framebuffer irq handle function error\n");
		goto unmap_osdreg;
	}

	/* Enable lcd interrupt */
	enable_fb_lcd_interrupt();

	printk(KERN_ALERT "%s, 0x%x, 0x%x, 0x%x\n", pdev->name, reg_fb->pending, reg_fb->source, reg_fb->mask);

#endif	/* CONFIG_LCD_ENABLE_IRQ */

/* ######################################################################### */

	printk(KERN_INFO "Imap Framebuffer Driver Initialization OK!\n");

	return 0;

#ifdef CONFIG_LCD_ENABLE_IRQ
unmap_osdreg:
	if(overlay_reg != NULL)
		iounmap(overlay_reg);
#endif	/* CONFIG_LCD_ENABLE_IRQ */

destroy_mutex:
	mutex_destroy(&mark_mutex);

dealloc_cmap:
	fb_dealloc_cmap(&imapfb_info[index].fb.cmap);

free_video_memory:
	imapfb_unmap_video_memory(&imapfb_info[index]);

release_clock2:
	clk_disable(info->clk2);
	clk_put(info->clk2);

release_clock1:
	clk_disable(info->clk1);
	clk_put(info->clk1);

release_io:
	iounmap(info->io);

release_mem:
	release_resource(info->mem);
	kfree(info->mem);

dealloc_fb:
	framebuffer_release(fbinfo);
	
	return ret;
}

static int imapfb_remove(struct platform_device *pdev)
{
	struct fb_info *fbinfo = platform_get_drvdata(pdev);
	imapfb_info_t *info = fbinfo->par;
	UINT32 index = 0;

	imapfb_set_backlight_level(0);
	msleep(50);	/* Sleep 50ms for LCD Power Down Timing */
	imapfb_set_backlight_power(0);
	imapfb_stop_lcd();	/* Stop Output of Display Controller */
	imapfb_set_lcd_power(0);

	msleep(1);

	//Free Framebuffer Memory and Unregister Framebuffer Device
	for (index = 0; index < IMAPFB_NUM; index++)
	{
		imapfb_unmap_video_memory((imapfb_info_t*)&imapfb_info[index]);
		unregister_framebuffer(&imapfb_info[index].fb);
	}

	//Disable Clock for IMAP Display Controller and Free Clock Structure
	if (info->clk1)
	{
		clk_disable(info->clk1);
		clk_put(info->clk1);
	 	info->clk1 = NULL;
	}
	if (info->clk2)
	{
		clk_disable(info->clk2);
		clk_put(info->clk2);
	 	info->clk2 = NULL;
	}

	//Unmap IO Memory
	iounmap(info->io);

	//Free IO Memory Resource
	release_resource(info->mem);
	kfree(info->mem);

	//Release Framebuffer Structure
	framebuffer_release(fbinfo);	

#ifdef CONFIG_LCD_ENABLE_IRQ
	/* Release mark mutex */
	mutex_destroy(&mark_mutex);

	/* Free irq handle thread */
	free_irq(IRQ_IDS, pdev);
#endif	/* CONFIG_LCD_ENABLE_IRQ */

	if(overlay_reg != NULL)
		iounmap(overlay_reg);

	return 0;
}

#if defined(CONFIG_PM)

static struct sleep_save imap_fb_save[] = {
	SAVE_ITEM(IMAP_LCDCON1),
	SAVE_ITEM(IMAP_LCDCON2),
	SAVE_ITEM(IMAP_LCDCON3),
	SAVE_ITEM(IMAP_LCDCON4),
	SAVE_ITEM(IMAP_LCDCON5),

	SAVE_ITEM(IMAP_LCDVCLKFSR),
	
	SAVE_ITEM(IMAP_IDSINTPND),
	SAVE_ITEM(IMAP_IDSSRCPND),
	SAVE_ITEM(IMAP_IDSINTMSK),

	SAVE_ITEM(IMAP_OVCDCR),
	SAVE_ITEM(IMAP_OVCPCR),
	SAVE_ITEM(IMAP_OVCBKCOLOR),
	
	SAVE_ITEM(IMAP_OVCW0CR),
	SAVE_ITEM(IMAP_OVCW0PCAR),
	SAVE_ITEM(IMAP_OVCW0PCBR),
	SAVE_ITEM(IMAP_OVCW0B0SAR),
	SAVE_ITEM(IMAP_OVCW0B1SAR),
	SAVE_ITEM(IMAP_OVCW0VSSR),
	SAVE_ITEM(IMAP_OVCW0CMR),
	SAVE_ITEM(IMAP_OVCW0B2SAR),
	SAVE_ITEM(IMAP_OVCW0B3SAR),

	SAVE_ITEM(IMAP_OVCW1CR),
	SAVE_ITEM(IMAP_OVCW1PCAR),
	SAVE_ITEM(IMAP_OVCW1PCBR),
	SAVE_ITEM(IMAP_OVCW1PCCR),
	SAVE_ITEM(IMAP_OVCW1B0SAR),
	SAVE_ITEM(IMAP_OVCW1B1SAR),
	SAVE_ITEM(IMAP_OVCW1VSSR),
	SAVE_ITEM(IMAP_OVCW1CKCR),
	SAVE_ITEM(IMAP_OVCW1CKR),
	SAVE_ITEM(IMAP_OVCW1CMR),
	SAVE_ITEM(IMAP_OVCW1B2SAR),
	SAVE_ITEM(IMAP_OVCW1B3SAR),

	SAVE_ITEM(IMAP_OVCW2CR),
	SAVE_ITEM(IMAP_OVCW2PCAR),
	SAVE_ITEM(IMAP_OVCW2PCBR),
	SAVE_ITEM(IMAP_OVCW2PCCR),
	SAVE_ITEM(IMAP_OVCW2B0SAR),
	SAVE_ITEM(IMAP_OVCW2B1SAR),
	SAVE_ITEM(IMAP_OVCW2VSSR),
	SAVE_ITEM(IMAP_OVCW2CKCR),
	SAVE_ITEM(IMAP_OVCW2CKR),
	SAVE_ITEM(IMAP_OVCW2CMR),
	SAVE_ITEM(IMAP_OVCW2B2SAR),
	SAVE_ITEM(IMAP_OVCW2B3SAR),

	SAVE_ITEM(IMAP_OVCW3CR),
	SAVE_ITEM(IMAP_OVCW3PCAR),
	SAVE_ITEM(IMAP_OVCW3PCBR),
	SAVE_ITEM(IMAP_OVCW3PCCR),
	SAVE_ITEM(IMAP_OVCW3BSAR),
	SAVE_ITEM(IMAP_OVCW3VSSR),
	SAVE_ITEM(IMAP_OVCW3CKCR),
	SAVE_ITEM(IMAP_OVCW3CKR),
	SAVE_ITEM(IMAP_OVCW3CMR),
	SAVE_ITEM(IMAP_OVCW3SABSAR),

	SAVE_ITEM(IMAP_OVCBRB0SAR),
	SAVE_ITEM(IMAP_OVCBRB1SAR),
	SAVE_ITEM(IMAP_OVCOEF11),
	SAVE_ITEM(IMAP_OVCOEF12),
	SAVE_ITEM(IMAP_OVCOEF13),
	SAVE_ITEM(IMAP_OVCOEF21),
	SAVE_ITEM(IMAP_OVCOEF22),
	SAVE_ITEM(IMAP_OVCOEF23),
	SAVE_ITEM(IMAP_OVCOEF31),
	SAVE_ITEM(IMAP_OVCOEF32),
	SAVE_ITEM(IMAP_OVCOEF33),
	SAVE_ITEM(IMAP_OVCOMC),
	SAVE_ITEM(IMAP_OVCBRB2SAR),
	SAVE_ITEM(IMAP_OVCBRB3SAR),
	
	SAVE_ITEM(IMAP_OVCW0PAL),
	SAVE_ITEM(IMAP_OVCW1PAL),
	SAVE_ITEM(IMAP_OVCW2PAL),
	SAVE_ITEM(IMAP_OVCW3PAL),
};

int imapfb_suspend(struct platform_device *dev, pm_message_t state)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	imapfb_info_t *info = fbinfo->par;
	unsigned int temp;

	imapfb_set_backlight_level(0);
	msleep(50);	/* Sleep 50ms for LCD Power Down Timing */
	imapfb_set_backlight_power(0);
	imapfb_stop_lcd();	/* Stop Output of Display Controller */
	imapfb_set_lcd_power(0);

	//Save all Registers
	imapx200_pm_do_save(imap_fb_save, ARRAY_SIZE(imap_fb_save));

	msleep(1);
	//Disable Clock for IMAP Display Controller
	clk_disable(info->clk1);
	clk_disable(info->clk2);

	return 0;
}

int imapfb_resume(struct platform_device *dev)
{
	struct fb_info *fbinfo = platform_get_drvdata(dev);
	imapfb_info_t *info = fbinfo->par;

	//Enable Clock for IMAP Display Controller
	clk_enable(info->clk1);
	clk_enable(info->clk2);
	//Restore all Registers
	imapx200_pm_do_restore(imap_fb_save, ARRAY_SIZE(imap_fb_save));

	imapfb_set_gpio();	/* Set GPIO Configuration for Output Data and Control Signal */
	imapfb_set_lcd_power(1);
	imapfb_start_lcd();	/* Start Output of Display Controller */
	imapfb_set_backlight_power(1);
	imapfb_set_backlight_level(IMAPFB_DEFAULT_BACKLIGHT_LEVEL);

	return 0;
}

#else

int imapfb_suspend(struct platform_device *dev, pm_message_t state)
{
	return 0;
}

int imapfb_resume(struct platform_device *dev)
{
	return 0;
}

#endif

static struct platform_driver imapfb_driver = {
	.probe		= imapfb_probe,
	.remove		= imapfb_remove,
	.suspend		= imapfb_suspend,
	.resume		= imapfb_resume,
	.driver		= {
		.name	= "imap-fb",
		.owner	= THIS_MODULE,
	},
};

int __devinit imapfb_init(void)
{
	return platform_driver_register(&imapfb_driver);
}
static void __exit imapfb_cleanup(void)
{
	platform_driver_unregister(&imapfb_driver);
}

module_init(imapfb_init);
module_exit(imapfb_cleanup);

MODULE_AUTHOR("Feng Jiaxing");
MODULE_DESCRIPTION("IMAP Framebuffer Driver");
MODULE_LICENSE("GPL");

#ifdef CONFIG_LCD_ENABLE_IRQ
/*****************************************************************************
** -Function:
**    enable_fb_lcd_interrupt(void)
**
** -Description: Only enable lcd interrupt, unmask it.
**	Enable OSD lcd interrupt. Only enable lcd interrupt, unmask it.
*****************************************************************************/
void enable_fb_lcd_interrupt(void)
{
	volatile unsigned int val = 0;
	volatile unsigned int ival = 0;
	volatile unsigned int sval = 0;

	ival = reg_fb->pending;
	sval = reg_fb->source;
	ival |= 0x01;
	sval |= 0x01;
	reg_fb->pending = ival;
	reg_fb->source = sval;

	val = reg_fb->mask;
	val &= (~0x01);
	reg_fb->mask = val;
}

void disable_fb_lcd_interrupt(void)
{
	volatile unsigned int val = 0;

	val = reg_fb->mask;
	val |= (0x01);
	reg_fb->mask = val;
}
#endif	/* CONFIG_LCD_ENABLE_IRQ */
