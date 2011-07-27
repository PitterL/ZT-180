/***************************************************************************** 
** drivers/video/infotm/imapfb.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** Use of Infotm's code is governed by terms and conditions 
** stated in the accompanying licensing statement. 
** 
** Description: Head file of Display Controller.
**
** Author:
**     Feng Jiaxing <jiaxing_feng@infotm.com>
**      
** Revision History: 
** ----------------- 
** 1.0  12/07/2009  Feng Jiaxing
*****************************************************************************/

#ifndef _IMAPFB_H_
#define _IMAPFB_H_

#include <linux/interrupt.h>
#include <plat/imapx.h>

/* Debug macros */
#define DEBUG 0

#if DEBUG
#define dbg_info(fmt, args...)	printk(KERN_INFO "%s: " fmt, __FUNCTION__ , ## args)
#else
#define dbg_info(fmt, args...)
#endif
#define dbg_err(fmt, args...)	printk(KERN_ERR "%s: " fmt, __FUNCTION__ , ## args)

/* Definitions */
#ifndef MHZ
#define MHZ 				(1000 * 1000)
#define PRINT_MHZ(m)	((m) / MHZ), ((m / 1000) % 1000)
#endif

/* Open/Close framebuffer window */
#define ON 	1
#define OFF	0

#define IMAPFB_MAX_NUM			4	/* max framebuffer device number */
#define IMAPFB_MAX_ALPHA_LEVEL	0xf	/* max alpha0/1 value */

#define IMAPFB_PALETTE_BUFF_CLEAR		0x80000000	/* palette entry is clear/invalid */

#define IMAPFB_COLOR_KEY_DIR_BG	0	/* match back-ground pixel in color key function */
#define IMAPFB_COLOR_KEY_DIR_FG	1	/* match fore-ground pixel in color key function */

#define IMAPFB_DEFAULT_BACKLIGHT_LEVEL	75	/* default backlight brightness: 75% */

/* Macros */
#define FB_MIN_NUM(x, y)		((x) < (y) ? (x) : (y))
#define IMAPFB_NUM			FB_MIN_NUM(IMAPFB_MAX_NUM, CONFIG_FB_IMAP_NUM)

/* IO controls */
#define IMAPFB_OSD_START			_IO  ('F', 201)
#define IMAPFB_OSD_STOP			_IO  ('F', 202)

#ifdef CONFIG_LCD_ENABLE_IRQ
/*
 * This ioctl command added by Sololz.
 * This ioctl will block till the irq mark is set.
 */
#define IMAPFB_OSD_WAIT_IRQ		_IO('F', 203)

#endif	/* CONFIG_LCD_ENABLE_IRQ */
#define IMAPFB_OSD_FLUSH_LEVEL	_IO('F', 204)	/* for flush osd level use */

#define IMAPFB_OSD_SET_POS_SIZE	_IOW ('F', 209, imapfb_win_pos_size)

#define IMAPFB_OSD_ALPHA0_SET		_IOW ('F', 210, UINT32)
#define IMAPFB_OSD_ALPHA1_SET		_IOW ('F', 211, UINT32)

#define IMAPFB_COLOR_KEY_START			_IO  ('F', 300)
#define IMAPFB_COLOR_KEY_STOP				_IO  ('F', 301)
#define IMAPFB_COLOR_KEY_ALPHA_START		_IO  ('F', 302)
#define IMAPFB_COLOR_KEY_ALPHA_STOP		_IO  ('F', 303)
#define IMAPFB_COLOR_KEY_SET_INFO			_IOW ('F', 304, imapfb_color_key_info_t)

#define IMAPFB_POWER_SUPPLY_CHANGE	_IO ('F', 324)

#define IMAPFB_GET_LCD_EDID_INFO		_IOR ('F', 327, void*)
#define IMAPFB_LCD_DETECT_CONNECT	_IO ('F', 328)
#define IMAPFB_GET_BUF_PHY_ADDR		_IOR ('F', 329, imapfb_dma_info_t)

/* Data type definition */
typedef unsigned long long	UINT64;
typedef unsigned int		UINT32;
typedef unsigned short		UINT16;
typedef unsigned char		UINT8;
typedef signed long long	SINT64;
typedef signed int			SINT32;
typedef signed short		SINT16;
typedef signed char		SINT8;

/******** Structures ********/
/* Framebuffer window position and size */
typedef struct {
	SINT32 left_x;
	SINT32 top_y;
	UINT32 width;
	UINT32 height;
} imapfb_win_pos_size;

/* Color key function info. */
typedef struct {
	UINT32 direction;
	UINT32 colval;
} imapfb_color_key_info_t;

/* Framebuffer window video memory physical addr (4 buffers) */
typedef struct {
	dma_addr_t map_dma_f1;
	dma_addr_t map_dma_f2;
	dma_addr_t map_dma_f3;
	dma_addr_t map_dma_f4;
} imapfb_dma_info_t;

/* RGB and transparency bit field assignment */
typedef struct {
	struct fb_bitfield red;
	struct fb_bitfield green;
	struct fb_bitfield blue;
	struct fb_bitfield transp;
} imapfb_rgb_t;

const static imapfb_rgb_t imapfb_a1rgb232_8 = {
	.red		= {.offset = 5, .length = 2,},
	.green	= {.offset = 2, .length = 3,},
	.blue	= {.offset = 0, .length = 2,},
	.transp	= {.offset = 7, .length = 1,},
};

const static imapfb_rgb_t imapfb_rgb565_16 = {
	.red		= {.offset = 11, .length = 5,},
	.green	= {.offset = 5, .length = 6,},
	.blue	= {.offset = 0, .length = 5,},
	.transp	= {.offset = 0, .length = 0,},
};

const static imapfb_rgb_t imapfb_rgb666_18 = {
	.red		= {.offset = 12, .length = 6,},
	.green	= {.offset = 6, .length = 6,},
	.blue	= {.offset = 0, .length = 6,},
	.transp	= {.offset = 0, .length = 0,},
};

const static imapfb_rgb_t imapfb_a1rgb666_19 = {
	.red		= {.offset = 12, .length = 6,},
	.green	= {.offset = 6, .length = 6,},
	.blue	= {.offset = 0, .length = 6,},
	.transp	= {.offset = 18, .length = 1,},
};

const static imapfb_rgb_t imapfb_rgb888_24 = {
	.red		= {.offset = 16, .length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 0, .length = 0,},
};

const static imapfb_rgb_t imapfb_a1rgb888_25 = {
	.red		= {.offset = 16, .length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 24, .length = 1,},
};

const static imapfb_rgb_t imapfb_a4rgb888_28 = {
	.red		= {.offset = 16,.length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 24, .length = 4,},
};

const static imapfb_rgb_t imapfb_a8rgb888_32 = {
	.red		= {.offset = 16, .length = 8,},
	.green	= {.offset = 8, .length = 8,},
	.blue	= {.offset = 0, .length = 8,},
	.transp	= {.offset = 24, .length = 8,},
};

/* Imap framebuffer struct */
typedef struct {
	struct fb_info		fb;		/* linux framebuffer struct */
	struct device		*dev;	/* linux framebuffer device */

	struct clk		*clk1;	/* clock resource for imapx200 display controller subsystem (IDS) */
	struct clk		*clk2;	/* clock resource for imapx200 display controller subsystem (IDS) */

	struct resource	*mem;	/* memory resource for IDS mmio */
	void __iomem		*io;		/* mmio for IDS */

	UINT32			win_id;	/* framebuffer window number */

	/* buf0 raw memory addresses */
	dma_addr_t		map_dma_f1;	/* physical */
	u_char *			map_cpu_f1;	/* virtual */
	UINT32			map_size_f1;	/* size */

	/* buf1 raw memory addresses */
	dma_addr_t		map_dma_f2;	/* physical */
	u_char *			map_cpu_f2;	/* virtual */
	unsigned int		map_size_f2;	/* size */

	/* buf2 raw memory addresses */
	dma_addr_t		map_dma_f3;	/* physical */
	u_char *			map_cpu_f3;	/* virtual */
	UINT32			map_size_f3;	/* size */

	/* buf3 raw memory addresses */
	dma_addr_t		map_dma_f4;	/* physical */
	u_char *			map_cpu_f4;	/* virtual */
	UINT32			map_size_f4;	/* size */

	/* keep these registers in case we need to re-write palette */
	UINT32			palette_buffer[256];	/* real palette buffer */
	UINT32			pseudo_pal[256];		/* pseudo palette buffer */
} imapfb_info_t;

typedef struct {
	/* Screen physical resolution */
	UINT32 xres;
	UINT32 yres;

	/* OSD Screen info */
	UINT32 osd_xres;			/* Visual OSD x resolution */
	UINT32 osd_yres;			/* Visual OSD y resolution */
	UINT32 osd_xres_virtual;	/* Virtual OSD x resolution */
	UINT32 osd_yres_virtual;	/* Virtual OSD y resolution */
	UINT32 osd_xoffset;		/* Visual to Virtual OSD x offset */
	UINT32 osd_yoffset;		/* Visual to Virtual OSD y offset */

	UINT32 bpp;				/* Bits per pixel */
	UINT32 bytes_per_pixel;	/* Bytes per pixel for true color */
	UINT32 pixclock;			/* Pixel clock */

	UINT32 hsync_len;		/* Horizontal sync length */
	UINT32 left_margin;		/* Horizontal back porch */
	UINT32 right_margin;		/* Horizontal front porch */
	UINT32 vsync_len;		/* Vertical sync length */
	UINT32 upper_margin;	/* Vertical back porch */
	UINT32 lower_margin;	/* Vertical front porch */
	UINT32 sync;				/* whether sync signal is used or not */

	UINT32 cmap_grayscale:1;
	UINT32 cmap_inverse:1;
	UINT32 cmap_static:1;
	UINT32 unused:29;

	/* LCD control registers */
	UINT32 lcdcon1;		/* LCD Control 1 Register */
	UINT32 lcdcon2;		/* LCD Control 2 Register */
	UINT32 lcdcon3;		/* LCD Control 3 Register */
	UINT32 lcdcon4;		/* LCD Control 4 Register */
	UINT32 lcdcon5;		/* LCD Control 5 Register */
	UINT32 lcdvclkfsr;	/* LCD VCLK Frequency Status Register */

	/* IDS interrupt registers */
	UINT32 idsintpnd;		/* IDS Interrupt Pending Register */
	UINT32 idssrcpnd;	/* IDS Source Pending Register */
	UINT32 idsintmsk;	/* IDS Interrupt Mask Register */

	/* Overlay control registers */
	UINT32 ovcdcr;		/* Overlay Controller Display Control Register */
	UINT32 ovcpcr;		/* Overlay Controller Palette Control Register */
	UINT32 ovcbkcolor;	/* Overlay Controller Background Color Register */
	
	UINT32 ovcw0cr;		/* Overlay Controller Window 0 Control Register */
	UINT32 ovcw0pcar;	/* Overlay Controller Window 0 Position Control A Register */
	UINT32 ovcw0pcbr;	/* Overlay Controller Window 0 Position Control B Register */
	UINT32 ovcw0b0sar;	/* Overlay Controller Window 0 Buffer 0 Start Address Register */
	UINT32 ovcw0b1sar;	/* Overlay Controller Window 0 Buffer 1 Start Address Register */
	UINT32 ovcw0vssr;	/* Overlay Controller Window 0 Virtual Screen Size Register */
	UINT32 ovcw0cmr;	/* Overlay Controller Window 0 Color Map Register */
	UINT32 ovcw0b2sar;	/* Overlay Controller Window 0 Buffer 2 Start Address Register */
	UINT32 ovcw0b3sar;	/* Overlay Controller Window 0 Buffer 3 Start Address Register */
	
	UINT32 ovcw1cr;		/* Overlay Controller Window 1 Control Register */
	UINT32 ovcw1pcar;	/* Overlay Controller Window 1 Position Control A Register */
	UINT32 ovcw1pcbr;	/* Overlay Controller Window 1 Position Control B Register */
	UINT32 ovcw1pccr;	/* Overlay Controller Window 1 Position Control C Register */
	UINT32 ovcw1b0sar;	/* Overlay Controller Window 1 Buffer 0 Start Address Register */
	UINT32 ovcw1b1sar;	/* Overlay Controller Window 1 Buffer 1 Start Address Register */
	UINT32 ovcw1vssr;	/* Overlay Controller Window 1 Virtual Screen Size Register */
	UINT32 ovcw1ckcr;	/* Overlay Controller Window 1 Color Key Control Register */
	UINT32 ovcw1ckr;	/* Overlay Controller Window 1 Color Key Register */
	UINT32 ovcw1cmr;	/* Overlay Controller Window 1 Color Map Register */
	UINT32 ovcw1b2sar;	/* Overlay Controller Window 1 Buffer 2 Start Address Register */
	UINT32 ovcw1b3sar;	/* Overlay Controller Window 1 Buffer 3 Start Address Register */
	
	UINT32 ovcw2cr;		/* Overlay Controller Window 2 Control Register */
	UINT32 ovcw2pcar;	/* Overlay Controller Window 2 Position Control A Register */
	UINT32 ovcw2pcbr;	/* Overlay Controller Window 2 Position Control B Register */
	UINT32 ovcw2pccr;	/* Overlay Controller Window 2 Position Control C Register */
	UINT32 ovcw2b0sar;	/* Overlay Controller Window 2 Buffer 0 Start Address Register */
	UINT32 ovcw2b1sar;	/* Overlay Controller Window 2 Buffer 1 Start Address Register */
	UINT32 ovcw2vssr;	/* Overlay Controller Window 2 Virtual Screen Size Register */
	UINT32 ovcw2ckcr;	/* Overlay Controller Window 2 Color Key Control Register */
	UINT32 ovcw2ckr;	/* Overlay Controller Window 2 Color Key Register */
	UINT32 ovcw2cmr;	/* Overlay Controller Window 2 Color Map Register */
	UINT32 ovcw2b2sar;	/* Overlay Controller Window 2 Buffer 2 Start Address Register */
	UINT32 ovcw2b3sar;	/* Overlay Controller Window 2 Buffer 3 Start Address Register */
	
	UINT32 ovcw3cr;		/* Overlay Controller Window 3 Control Register */
	UINT32 ovcw3pcar;	/* Overlay Controller Window 3 Position Control A Register */
	UINT32 ovcw3pcbr;	/* Overlay Controller Window 3 Position Control B Register */
	UINT32 ovcw3pccr;	/* Overlay Controller Window 3 Position Control C Register */
	UINT32 ovcw3bsar;	/* Overlay Controller Window 3 Buffer Start Address Register */
	UINT32 ovcw3vssr;	/* Overlay Controller Window 3 Virtual Screen Size Register */
	UINT32 ovcw3ckcr;	/* Overlay Controller Window 3 Color Key Control Register */
	UINT32 ovcw3ckr;	/* Overlay Controller Window 3 Color Key Register */
	UINT32 ovcw3cmr;	/* Overlay Controller Window 3 Color Map Register */
	UINT32 ovcw3sabsar;	/* Overlay Controller Window 3 Cursor And Bitmap Buffer Start Address Register */

	UINT32 ovcbrb0sar;	/* Overlay Controller CbCr Buffer 0 Start Address Register */
	UINT32 ovcbrb1sar;	/* Overlay Controller CbCr Buffer 1 Start Address Register*/
	
	UINT32 ovcoef11;		/* Overlay Controller Color Matrix Coefficient11 Register */
	UINT32 ovcoef12;		/* Overlay Controller Color Matrix Coefficient12 Register */
	UINT32 ovcoef13;		/* Overlay Controller Color Matrix Coefficient13 Register */
	UINT32 ovcoef21;		/* Overlay Controller Color Matrix Coefficient21 Register */
	UINT32 ovcoef22;		/* Overlay Controller Color Matrix Coefficient22 Register */
	UINT32 ovcoef23;		/* Overlay Controller Color Matrix Coefficient23 Register */
	UINT32 ovcoef31;		/* Overlay Controller Color Matrix Coefficient31 Register */
	UINT32 ovcoef32;		/* Overlay Controller Color Matrix Coefficient32 Register */
	UINT32 ovcoef33;		/* Overlay Controller Color Matrix Coefficient33 Register */

	UINT32 ovcomc;		/* Overlay Controller Color Matrix Configure Register */
	UINT32 ovcbrb2sar;	/* Overlay Controller CbCr Buffer 2 Start Address Register */
	UINT32 ovcbrb3sar;	/* Overlay Controller CbCr Buffer 3 Start Address Register */

	/* Utility Functions */
	void (*set_backlight_power)(UINT32);
	void (*set_lcd_power)(UINT32);
	void (*set_brightness)(UINT32);
	int (*lcd_check_var)(struct fb_var_screeninfo *);
}imapfb_fimd_info_t;

#ifdef CONFIG_LCD_ENABLE_IRQ
/* I notice that above structure for register is something problem. */
typedef struct
{
	volatile unsigned int dmp[21];
	volatile unsigned int pending;	/* Interrupt register of pending */
	volatile unsigned int source;	/* Interrupt register of source */
	volatile unsigned int mask;	/* Interrupt register of mask */
}ids_reg_t;
#endif	/* CONFIG_LCD_ENABLE_IRQ */

extern imapfb_fimd_info_t imapfb_fimd;

#if defined (CONFIG_FB_IMAP_KERNEL_LOGO)
void imapfb_kernel_logo(void*);
#endif
void imapfb_lcd_power_supply(UINT32);
void imapfb_backlight_power_supply(UINT32);
void imapfb_set_brightness(UINT32);
void imapfb_set_gpio(void);
void imapfb_set_clk(void);

#endif

