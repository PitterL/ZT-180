/***************************************************************************** 
** XXX hdmi_apilib.h XXX
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: PCB test, module hdmi.
**
** Author:
**      
**      XXX Alex XXX
** Revision History: 
** ----------------- 
** 1.1  XXX 04/30/2010 XXX	
*****************************************************************************/
/*structure for hdmi solution*/
#include <mt/common.h>
typedef unsigned long UINT32;
typedef BOOL unsigned long;

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
/*DPLLCFG*/
	UINT32 DPLLCFG;

/*DIVCFG4*/	
	UINT32 DIVCFG4;
} struct_lds_clk_param;

typedef enum
{
	LCD = 0,
	HDMI_1080P,
	HDMI_720P,
	HDMI_480P,
}LCD_TIMING;

BOOL lcd_change_timing(LCD_TIMING timing);
void lcd_config_clk(LCD_TIMING timing);
void lcd_config_controller(LCD_TIMING timing);
int pt_hdmi_cable_check(void); 
int pt_hdmi_init(void);
int pt_hdmi_release(void); 
