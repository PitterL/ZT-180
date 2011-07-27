/***************************************************************************** 
 * ** drivers/video/infotm_HDMI/imap_HDMI.h
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: Head file of Infotm HDMI.
 * **
 * ** Author:
 * **     Alex Zhang <alex.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.0  06/11/2010 Alex Zhang 
* *****************************************************************************/
#ifndef __IMAP_HDMI_H__
#define __IMAP_HDMI_H__


typedef unsigned long UINT32;
typedef unsigned long BOOL; 

typedef struct{
/*LCDCON1*/
	UINT32 VCLK;
	UINT32 EACH_FRAME;
	UINT32 LCD_PANNEL;
	UINT32 BPP_MODE;
	UINT32 LCD_OUTPUT;

/*LCDCON2*/
	UINT32 VBPD;
	UINT32 VACTIVE;
	UINT32 VFPD;
	UINT32 VSPW;

/*LCDCON3*/	
	UINT32 VACTIVE_HIGHBIT;
	UINT32 HBPD;
	UINT32 HACTIVE;
	UINT32 HFPD;

/*LCDCON4*/	
	UINT32 HSPW;

/*LCDCON5*/
	UINT32 COLOR_MODE;
	UINT32 BPP24BL;
	UINT32 FRM565;
    
	UINT32 INVVCLK;
	UINT32 INVVLINE;
	UINT32 INVVFRAME;
	UINT32 INVVD;
	UINT32 INVVDEN;
	UINT32 INVPWREN;
	UINT32 INVENDLINE;
	UINT32 PWREN;
	UINT32 ENLEND;
	UINT32 BSWP;
	UINT32 HWSWP;
} struct_lcd_timing_param;

typedef struct{
/*IMAP_TVCCR*/
	UINT32 Clock_enable;
	UINT32 TV_PCLK_mode;
	UINT32 Inv_clock;
	UINT32 clock_sel;
       	UINT32 Clock_div;

/*TVICR*/
	UINT32 tvif_enable;
	UINT32 ITU601_656n;
	UINT32 Bit16ofITU60;
	UINT32 Direct_data;
	UINT32 Bitswap;
	UINT32 Data_order;
	UINT32 Inv_vsync;
	UINT32 Inv_hsync;
	UINT32 Inv_href;
	UINT32 Inv_field;
	UINT32 Begin_with_EAV;

/*TVCMCR*/
	UINT32 Matrix_mode;
	UINT32 Passby;
	UINT32 Inv_MSB_in;
	UINT32 Inv_MSB_out;
	UINT32 Matrix_oft_b;
	UINT32 Matrix_oft_a;

/*TVUBA1*/
	UINT32 UBA1_LEN;

/*TVUNBA*/	
	UINT32 UNBA_LEN;

/*TVUBA2*/
	UINT32 UNBA2_LEN;

/*TVLBA1*/
	UINT32 LBA1_LEN;

/*TVLNBA*/
	UINT32 LNBA_LEN;

/*TVLBA2*/
	UINT32 LBA2_LEN;

/*TVBLEN*/
	UINT32 BLANK_LEN;

/*TVVLEN*/	
	UINT32 VIDEO_LEN;

/*TVHSCR*/
	UINT32 Hsync_VB1_ctrl;
	UINT32 Hsync_delay;
	UINT32 Hsync_extend;

/*TVVSHCR*/
	UINT32 Vsync_delay_upper;
	UINT32 Vsync_extend_upper;

/*TVVSLCR*/
	UINT32 Vsync_delay_lower;
	UINT32 Vsync_extend_lower;

/*TVXSIZE*/
	UINT32 DISP_XSIZE;

/*TVYSIZE*/
	UINT32 DISP_YSIZE;
	
}struct_tvif_timing_param;

typedef struct{
/*DPLLCFG*/
	UINT32 DPLLCFG;

/*DIVCFG4*/	
	UINT32 DIVCFG4;
} struct_lds_clk_param;

typedef enum
{
	LCD = 0,
	HDMI_1080P,
//	HDMI_1080I,
	HDMI_720P,
	HDMI_480P_16_9,
	HDMI_480P_4_3,
//	HDMI_480I_16_9,
//	HDMI_480I_4_3,
	HDMI_576P_16_9,
	HDMI_576P_4_3,
//	HDMI_576I_16_9,
//	HDMI_576I_4_3,
	HDMI_640_480,	
	HDMI_1080P_TV,
//	HDMI_1080I,
	HDMI_720P_TV,
	HDMI_480P_16_9_TV,
	HDMI_480P_4_3_TV,
//	HDMI_480I_16_9,
//	HDMI_480I_4_3,
	HDMI_576P_16_9_TV,
	HDMI_576P_4_3_TV,
//	HDMI_576I_16_9,
//	HDMI_576I_4_3,
	HDMI_640_480_TV,	
}LCD_TIMING;

/*
struct imap_HDMI_info {
	void (* ClearPending)(void);
	void (* HotplugMonitor)(void);
	struct tasklet_struct my_tasklet;
	struct i2c_client *client
};
*/

BOOL lcd_change_timing(LCD_TIMING timing, BOOL tv_IF);
void lcd_config_clk(LCD_TIMING timing);
void lcd_config_controller(LCD_TIMING timing);
void tvif_config_controller(LCD_TIMING timing);

#define HDMI_CHECK_HOTPLUG	_IOR('H', 301, UINT32)
#define HDMI_SET_NOTMAL_TIMING	_IO('H', 302)
#define HDMI_SET_VIDEO_TIMING	_IOW('H', 303, UINT32)
#define HDMI_QUERY_APP		_IO('H', 304)
#define HDMI_QUERY_RENDER	_IO('H', 305)
#define HDMI_CHECK_MENUSWITCH	_IOR('H', 306, UINT32)
#define HDMI_CHECK		_IOR('H', 307, UINT32)
#define HDMI_QUERY_MONITOR	_IO('H', 308)
#define HDMI_TV_SUPPORT_V_MODE  _IOR('H', 310, UINT32)

#endif
