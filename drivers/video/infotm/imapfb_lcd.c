/***************************************************************************** 
** drivers/video/infotm/imapfb_lcd.c
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
** 1.0  09/14/2009  Feng Jiaxing
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
#include <linux/string.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <linux/wait.h>

#include <asm/io.h>
#include <asm/uaccess.h>

#include "imapfb.h"
#include "logo_new.h"
#include "logo.h"

extern void bk_power_ctrl(bool on);

#if defined(CONFIG_FB_IMAP_LCD800X480)
#define IMAPFB_HFP		40		/* front porch */
#define IMAPFB_HSW		48		/* hsync width */
#define IMAPFB_HBP		40		/* back porch */

#define IMAPFB_VFP		13		/* front porch */
#define IMAPFB_VSW		3		/* vsync width */
#define IMAPFB_VBP		29		/* back porch */

#define IMAPFB_HRES		800		/* horizon pixel x resolition */
#define IMAPFB_VRES		480		/* line cnt y resolution */

//#define IMAPFB_VFRAME_FREQ	30		/* frame rate freq */

#define IMAPFB_DIV_CFG4 0x1e16

#elif defined(CONFIG_FB_IMAP_LCD800X480_XY)
#define IMAPFB_HFP		210		/* front porch */
#define IMAPFB_HSW		20		/* hsync width */
#define IMAPFB_HBP		44		/* back porch */

#define IMAPFB_VFP		22		/* front porch */
#define IMAPFB_VSW		10		/* vsync width */
#define IMAPFB_VBP		23		/* back porch */

#define IMAPFB_HRES		800		/* horizon pixel x resolition */
#define IMAPFB_VRES		480		/* line cnt y resolution */

//#define IMAPFB_VFRAME_FREQ	30		/* frame rate freq */

#define IMAPFB_DIV_CFG4 0x1612/*0x1e16*/

#elif defined(CONFIG_FB_IMAP_LCD800X480_XY2)
#define IMAPFB_HFP		210		/* front porch */
#define IMAPFB_HSW		20		/* hsync width */
#define IMAPFB_HBP		46		/* back porch */

#define IMAPFB_VFP		22		/* front porch */
#define IMAPFB_VSW		10		/* vsync width */
#define IMAPFB_VBP		21		/* back porch */

#define IMAPFB_HRES		800		/* horizon pixel x resolition */
#define IMAPFB_VRES		480		/* line cnt y resolution */

//#define IMAPFB_VFRAME_FREQ	30		/* frame rate freq */

#define IMAPFB_DIV_CFG4 0x1612/*0x1e16*/

#elif defined(CONFIG_FB_IMAP_LCD800X600)
#define IMAPFB_HFP		210		/* front porch */
#define IMAPFB_HSW		2		/* hsync width */
#define IMAPFB_HBP		46		/* back porch */

#define IMAPFB_VFP		12		/* front porch */
#define IMAPFB_VSW		0		/* vsync width */
#define IMAPFB_VBP		23		/* back porch */

#define IMAPFB_HRES		800		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0x1612

#elif defined(CONFIG_FB_IMAP_LCD1024X576)
#define IMAPFB_HFP		15		/* front porch */
#define IMAPFB_HSW		104		/* hsync width */
#define IMAPFB_HBP		128		/* back porch */

#define IMAPFB_VFP		5		/* front porch */
#define IMAPFB_VSW		4		/* vsync width */
#define IMAPFB_VBP		21		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		576		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0x1612
#elif defined(CONFIG_FB_IMAP_LCD1024X600)
#define IMAPFB_HFP		15		/* front porch */
#define IMAPFB_HSW		104		/* hsync width */
#define IMAPFB_HBP		128		/* back porch */

#define IMAPFB_VFP		5		/* front porch */
#define IMAPFB_VSW		4		/* vsync width */
#define IMAPFB_VBP		21		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0x1612
#elif defined(CONFIG_FB_IMAP_LCD1024X600_QM)
#define IMAPFB_HFP		0		/* front porch */
#define IMAPFB_HSW		0		/* hsync width */
#define IMAPFB_HBP		160		/* back porch */

#define IMAPFB_VFP		0		/* front porch */
#define IMAPFB_VSW		0		/* vsync width */
#define IMAPFB_VBP		19		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0x1612

#elif defined(CONFIG_FB_IMAP_LCD1024X600_XY)
#define IMAPFB_HFP		100		/* front porch */
#define IMAPFB_HSW		60		/* hsync width */
#define IMAPFB_HBP		160		/* back porch */

#define IMAPFB_VFP		6		/* front porch */
#define IMAPFB_VSW		0		/* vsync width */
#define IMAPFB_VBP		19		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0x120e

#elif defined(CONFIG_FB_IMAP_LCD1024X600_7INCH)
#define IMAPFB_HFP		160		/* front porch */
#define IMAPFB_HSW		0		/* hsync width */
#define IMAPFB_HBP		160		/* back porch */

#define IMAPFB_VFP		12		/* front porch */
#define IMAPFB_VSW		0		/* vsync width */
#define IMAPFB_VBP		23		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		600		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0xe0a

#elif defined(CONFIG_FB_IMAP_LCD1024X768_8INCH)
#define IMAPFB_HFP		100		/* front porch */
#define IMAPFB_HSW		40		/* hsync width */
#define IMAPFB_HBP		180		/* back porch */

#define IMAPFB_VFP		15		/* front porch */
#define IMAPFB_VSW		0		/* vsync width */
#define IMAPFB_VBP		23		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		768		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0xe0a

#elif defined(CONFIG_FB_IMAP_LCD1024X768_9INCH)
#define IMAPFB_HFP		100		/* front porch */
#define IMAPFB_HSW		40		/* hsync width */
#define IMAPFB_HBP		180		/* back porch */

#define IMAPFB_VFP		15		/* front porch */
#define IMAPFB_VSW		0		/* vsync width */
#define IMAPFB_VBP		23		/* back porch */

#define IMAPFB_HRES		1024		/* horizon pixel x resolition */
#define IMAPFB_VRES		768		/* line cnt y resolution */

#define IMAPFB_DIV_CFG4 0xe0a

#endif

#define IMAPFB_HRES_OSD	IMAPFB_HRES		/* horizon pixel x resolition */
#define IMAPFB_VRES_OSD	IMAPFB_VRES		/* line cnt y resolution */

#define IMAPFB_HRES_OSD_VIRTUAL	IMAPFB_HRES		/* horizon pixel x resolition */
#define IMAPFB_VRES_OSD_VIRTUAL	IMAPFB_VRES		/* line cnt y resolution */
#define IMAPFB_VFRAME_FREQ	60		/* frame rate freq */


#define IMAPFB_PIXEL_CLOCK	(IMAPFB_VFRAME_FREQ * (IMAPFB_HFP + IMAPFB_HSW + IMAPFB_HBP + IMAPFB_HRES) * (IMAPFB_VFP + IMAPFB_VSW + IMAPFB_VBP + IMAPFB_VRES))


imapfb_fimd_info_t imapfb_fimd = {
    .lcdcon1 = (1<<	8) |		//<R/W> "CLKVAL" VCLK = HCLK / ((CLKVAL + 1) * 2) -> About 7 Mhz  // ;;; SHL
			   (0	<<	7) |		// <R/W> must be always set to 0
			   (3	<<	5) |		// <R>Reserved : default = 3
			   (12	<<	1) |		// <R> Reserved : default = 0
			   (0	<<	0),		   // <R/W> "ENVID" lcd video output and the logic enable/disable 
			                       // 0 = Disable the video output and the LCD control signal
			                       // 1 = Enable the video output and the LCD control signal
	
	.lcdcon2 = (IMAPFB_VBP << 24) |		//<R/W> "VBPD"  Vertiacl back porch is the number of inactive lines at the start of a frame,after vertical synchronization period
			   ((IMAPFB_VRES -1) << 14) |	//<R/W> "LINEVAL" these bits determine the vertical size of LCD panel. Note: In the field, only low 10 bit is set, LINEVAL[10] is set in bit 31 of register LCDCON3
			   (IMAPFB_VFP << 6) |		//<R/W> "VFPD"  vertical front porch is the number of inactive lines at the end of a frame, before vertical synchronization period
			   (IMAPFB_VSW << 0),		//<R/W> "VSPW"  vertical sync pulse width determines the VSYNC pulse's high level width by counting the number of inactive lines
	
	.lcdcon3 = (IMAPFB_HBP << 19) |		// <R/W> "HBPD" Horizontal back porch is the number of VCLK periods between the falling edge of HSYNC and the start of active data
			   ((IMAPFB_HRES - 1) << 8) |	// <R/W> "HOZVAL" these bits determine the horizontal size of LCD panel
			   (IMAPFB_HFP << 0) ,		// <R/W> "HFPD" Horizontal front porch is the number of VCLK periods between the end of active data and the rising edge of HSYNC
	
	.lcdcon4 = (IMAPFB_HSW << 0) ,		// <R/W> "HSPW" 

#if (defined(CONFIG_FB_IMAP_LCD800X480)||defined(CONFIG_FB_IMAP_LCD800X480_XY)||defined(CONFIG_FB_IMAP_LCD800X480_XY2)||defined(CONFIG_FB_IMAP_LCD800X600))
	.lcdcon5 = (((0<<4)|(0x1<<2)|(2))<<24) |  //<R/W> "RGBORD" RGB output order of TFT LCD display interface 
			   (0 << 11) |		// <R/W> "DSPTYPE" [12:11] = 2b' 00:TFT LCD display  01:i80 interface 10:TV display
			   (0 << 10) |		// <R/W> "INVVCLK"       : VCLK Falling Edge
			   (0 << 9) |		// <R/W> "INVVLINE"      : Inverted Polarity
			   (0 << 8) |		// <R/W> INVVFRAME     : Inverted Polarity
			   (0 << 7) |		// <R/W> INVVD         : Normal
			   (0 << 6) |		// <R/W> INVVDEN       : Normal
			   (0 << 5) |		// <R/W> INVPWREN      : Normal
			   (0 << 4) |		// <R/W> INVENDLINE    : Normal
			   (0 << 3) |		// <R/W> PWREN         : Disable PWREN
			   (0 << 0) ,		// RESERVED

#elif defined(CONFIG_FB_IMAP_LCD1024X576)	
	.lcdcon5 = (((0<<4)|(0x1<<2)|(2))<<24) |  //<R/W> "RGBORD" RGB output order of TFT LCD display interface 
			   (0 << 11) |		// <R/W> "DSPTYPE" [12:11] = 2b' 00:TFT LCD display  01:i80 interface 10:TV display
			   (1 << 10) |		// <R/W> "INVVCLK"       : VCLK Falling Edge
			   (0 << 9) |		// <R/W> "INVVLINE"      : Inverted Polarity
			   (0 << 8) |		// <R/W> INVVFRAME     : Inverted Polarity
			   (0 << 7) |		// <R/W> INVVD         : Normal
			   (0 << 6) |		// <R/W> INVVDEN       : Normal
			   (0 << 5) |		// <R/W> INVPWREN      : Normal
			   (0 << 4) |		// <R/W> INVENDLINE    : Normal
			   (0 << 3) |		// <R/W> PWREN         : Disable PWREN
			   (0 << 0) ,		// RESERVED
#elif defined(CONFIG_FB_IMAP_LCD1024X600)||defined(CONFIG_FB_IMAP_LCD1024X600_QM)||defined(CONFIG_FB_IMAP_LCD1024X600_XY)
	.lcdcon5 = (((0<<4)|(0x1<<2)|(2))<<24) |  //<R/W> "RGBORD" RGB output order of TFT LCD display interface 
			   (0 << 11) |		// <R/W> "DSPTYPE" [12:11] = 2b' 00:TFT LCD display  01:i80 interface 10:TV display
			   (1 << 10) |		// <R/W> "INVVCLK"       : VCLK Falling Edge
			   (0 << 9) |		// <R/W> "INVVLINE"      : Inverted Polarity
			   (0 << 8) |		// <R/W> INVVFRAME     : Inverted Polarity
			   (0 << 7) |		// <R/W> INVVD         : Normal
			   (0 << 6) |		// <R/W> INVVDEN       : Normal
			   (0 << 5) |		// <R/W> INVPWREN      : Normal
			   (0 << 4) |		// <R/W> INVENDLINE    : Normal
			   (0 << 3) |		// <R/W> PWREN         : Disable PWREN
			   (0 << 0) ,		// RESERVED	
#elif (defined(CONFIG_FB_IMAP_LCD1024X600_7INCH))
    .lcdcon5 = (((0<<4)|(0x1<<2)|(2))<<24) |  //<R/W> "RGBORD" RGB output order of TFT LCD display interface 
               (0 << 11) |      // <R/W> "DSPTYPE" [12:11] = 2b' 00:TFT LCD display  01:i80 interface 10:TV display
               (0 << 10) |      // <R/W> "INVVCLK"       : VCLK Falling Edge
               (0 << 9) |       // <R/W> "INVVLINE"      : Inverted Polarity
               (0 << 8) |       // <R/W> INVVFRAME     : Inverted Polarity
               (0 << 7) |       // <R/W> INVVD         : Normal
               (0 << 6) |       // <R/W> INVVDEN       : Normal
               (0 << 5) |       // <R/W> INVPWREN      : Normal
               (0 << 4) |       // <R/W> INVENDLINE    : Normal
               (0 << 3) |       // <R/W> PWREN         : Disable PWREN
               (0 << 0) ,       // RESERVED

#elif (defined(CONFIG_FB_IMAP_LCD1024X768_8INCH))
    .lcdcon5 = (((0<<4)|(0x1<<2)|(2))<<24) |  //<R/W> "RGBORD" RGB output order of TFT LCD display interface 
               (0 << 11) |      // <R/W> "DSPTYPE" [12:11] = 2b' 00:TFT LCD display  01:i80 interface 10:TV display
               (0 << 10) |      // <R/W> "INVVCLK"       : VCLK Falling Edge
               (0 << 9) |       // <R/W> "INVVLINE"      : Inverted Polarity
               (0 << 8) |       // <R/W> INVVFRAME     : Inverted Polarity
               (0 << 7) |       // <R/W> INVVD         : Normal
               (0 << 6) |       // <R/W> INVVDEN       : Normal
               (0 << 5) |       // <R/W> INVPWREN      : Normal
               (0 << 4) |       // <R/W> INVENDLINE    : Normal
               (0 << 3) |       // <R/W> PWREN         : Disable PWREN
               (0 << 0) ,       // RESERVED

#elif (defined(CONFIG_FB_IMAP_LCD1024X768_9INCH))
    .lcdcon5 = (((0<<4)|(0x1<<2)|(2))<<24) |  //<R/W> "RGBORD" RGB output order of TFT LCD display interface 
               (0 << 11) |      // <R/W> "DSPTYPE" [12:11] = 2b' 00:TFT LCD display  01:i80 interface 10:TV display
               (0 << 10) |      // <R/W> "INVVCLK"       : VCLK Falling Edge
               (0 << 9) |       // <R/W> "INVVLINE"      : Inverted Polarity
               (0 << 8) |       // <R/W> INVVFRAME     : Inverted Polarity
               (0 << 7) |       // <R/W> INVVD         : Normal
               (0 << 6) |       // <R/W> INVVDEN       : Normal
               (0 << 5) |       // <R/W> INVPWREN      : Normal
               (0 << 4) |       // <R/W> INVENDLINE    : Normal
               (0 << 3) |       // <R/W> PWREN         : Disable PWREN
               (0 << 0) ,       // RESERVED

#endif

	.ovcdcr = IMAP_OVCDCR_IFTYPE_RGB,

#if defined (CONFIG_FB_IMAP_BPP8)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_8BPP_ARGB232 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 8,
	.bytes_per_pixel = 1,
	
#elif defined (CONFIG_FB_IMAP_BPP16)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_16BPP_RGB565 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 16,
	.bytes_per_pixel = 2,
	
#elif defined (CONFIG_FB_IMAP_BPP18)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_18BPP_RGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 18,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP19)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_19BPP_ARGB666 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 19,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP24)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_24BPP_RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 24,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP25)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_25BPP_ARGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 25,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP28)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_28BPP_A4RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 28,
	.bytes_per_pixel = 4,

#elif defined (CONFIG_FB_IMAP_BPP32)
	.ovcw0cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE
		| IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw1cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw2cr = IMAP_OVCWxCR_BUFSEL_BUF0 | IMAP_OVCWxCR_BUFAUTOEN_DISABLE | IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE
		| IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE | IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 
		| IMAP_OVCWxCR_BLD_PIX_PLANE | IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.ovcw3cr = IMAP_OVCWxCR_BITSWP_DISABLE | IMAP_OVCWxCR_BIT2SWP_DISABLE | IMAP_OVCWxCR_BIT4SWP_DISABLE | IMAP_OVCWxCR_BYTSWP_DISABLE
		| IMAP_OVCWxCR_HAWSWP_DISABLE | IMAP_OVCWxCR_ALPHA_SEL_1 | IMAP_OVCWxCR_BLD_PIX_PLANE
		| IMAP_OVCWxCR_BPPMODE_32BPP_A8RGB888 | IMAP_OVCWxCR_ENWIN_DISABLE,
	.bpp = 32,
	.bytes_per_pixel = 4,

#endif

	.ovcw0pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw0pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw0cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,

#if (CONFIG_FB_IMAP_NUM > 1)
	.ovcw1pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw1pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw1pccr = IMAP_OVCWxPCCR_ALPHA0_R(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA0_G(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA0_B(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_R(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA1_G(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_B(IMAPFB_MAX_ALPHA_LEVEL),
	.ovcw1cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,
#endif

#if (CONFIG_FB_IMAP_NUM > 2)	
	.ovcw2pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw2pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw2pccr = IMAP_OVCWxPCCR_ALPHA0_R(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA0_G(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA0_B(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_R(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA1_G(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_B(IMAPFB_MAX_ALPHA_LEVEL),
	.ovcw2cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,
#endif

#if (CONFIG_FB_IMAP_NUM > 3)
	.ovcw3pcar = IMAP_OVCWxPCAR_LEFTTOPX(0) | IMAP_OVCWxPCAR_LEFTTOPY(0),
	.ovcw3pcbr = IMAP_OVCWxPCBR_RIGHTBOTX(IMAPFB_HRES_OSD - 1) | IMAP_OVCWxPCBR_RIGHTBOTY(IMAPFB_VRES_OSD - 1),
	.ovcw3pccr = IMAP_OVCWxPCCR_ALPHA0_R(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA0_G(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA0_B(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_R(IMAPFB_MAX_ALPHA_LEVEL)
		| IMAP_OVCWxPCCR_ALPHA1_G(IMAPFB_MAX_ALPHA_LEVEL) | IMAP_OVCWxPCCR_ALPHA1_B(IMAPFB_MAX_ALPHA_LEVEL),
	.ovcw3cmr = IMAP_OVCWxCMR_MAPCOLEN_DISABLE,
#endif	

	.sync = 0,
	.cmap_static = 1,

	.xres = IMAPFB_HRES,
	.yres = IMAPFB_VRES,

	.osd_xres = IMAPFB_HRES_OSD,
	.osd_yres = IMAPFB_VRES_OSD,

	.osd_xres_virtual = IMAPFB_HRES_OSD_VIRTUAL,
	.osd_yres_virtual = IMAPFB_VRES_OSD_VIRTUAL,

	.osd_xoffset = 0,
	.osd_yoffset = 0,

	.pixclock = IMAPFB_PIXEL_CLOCK,

	.hsync_len = IMAPFB_HSW,
	.vsync_len = IMAPFB_VSW,
	.left_margin = IMAPFB_HBP,
	.upper_margin = IMAPFB_VBP,
	.right_margin = IMAPFB_HFP,
	.lower_margin = IMAPFB_VFP,
	.set_lcd_power = imapfb_lcd_power_supply,
	//.set_backlight_power= imapfb_backlight_power_supply,
	//.set_brightness = imapfb_set_brightness,
};

void imapfb_lcd_power_supply(UINT32 on_off)
{
    bk_power_ctrl(!!on_off);
}


void imapfb_backlight_power_supply(UINT32 on_off)
{

}

void imapfb_set_brightness(UINT32 val)
{
	printk(KERN_INFO "Set Lcd Backlight Brightness %d\n", val);
}

void imapfb_set_gpio(void)
{
	printk(KERN_INFO "LCD TYPE :: will be initialized\n");

	//Set RGB IF Data Line and Control Singal Line
	writel(0xaaaaaaaa, rGPMCON);
	writel(0x2aaaaaa, rGPNCON);
	writel(0, rGPMPUD);
	writel(0, rGPNPUD);
}

void imapfb_set_clk(void)
{
//	printk(KERN_INFO "IDS Clock Source Setting\n");

	//Set IF Clk Source and OSD Clk Source
	//writel((7 << 10) | (2 << 8) | (7 << 2) | (2 << 0), rDIV_CFG4);
	//writel(0x161e, rDIV_CFG4);
    writel(IMAPFB_DIV_CFG4,rDIV_CFG4);
}

#if defined (CONFIG_FB_IMAP_KERNEL_LOGO)
void imapfb_kernel_logo(void *buf)
{
	int i, nX, nY;
	UINT8 *pDB;
	UINT8 *pFB;
	pFB = (UINT8 *)buf;
	
#if defined (CONFIG_FB_IMAP_BPP16)
	pDB = (UINT8 *)logo_300x120;

	memset(pFB, 0xff, IMAPFB_HRES * IMAPFB_VRES * 2);
	for (i = 0; i < IMAPFB_HRES * IMAPFB_VRES; i++)
	{
		nX = i % IMAPFB_HRES;
		nY = i / IMAPFB_HRES;
		if((nX >= ((IMAPFB_HRES - 300) / 2)) && (nX < ((IMAPFB_HRES + 300) / 2)) && (nY >= ((IMAPFB_VRES - 120) / 2)) && (nY < ((IMAPFB_VRES + 120) / 2)))
		{
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
		}
		else
		{
			pFB++;
			pFB++;
		}
	}
#elif defined (CONFIG_FB_IMAP_BPP32)
	pDB = (UINT8 *)gImage_logo;

	memset(pFB, 0xff, IMAPFB_HRES * IMAPFB_VRES * 4);
	for (i = 0; i < IMAPFB_HRES * IMAPFB_VRES; i++)
	{
		nX = i % IMAPFB_HRES;
		nY = i / IMAPFB_HRES;
		if((nX >= ((IMAPFB_HRES - 320) / 2)) && (nX < ((IMAPFB_HRES + 320) / 2)) && (nY >= ((IMAPFB_VRES - 240) / 2)) && (nY < ((IMAPFB_VRES + 240) / 2)))
		{
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
			*pFB++ = *pDB++;
		}
		else
		{
			pFB++;
			pFB++;
			pFB++;
			pFB++;
		}
	}
#endif
}
#endif
