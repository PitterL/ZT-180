/***************************************************************************** 
** XXX hdmi_apilib.c XXX
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
#include "EP932api.h"
#include <mt/common.h>
#include <mt/hdmi_apilib.h>

struct_lcd_timing_param ids_timing[4] = 
{
	{1,0,3,12,0,5,480,40,2,0,6,800,250,2,0x06,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,3,12,0,35,1080,4,6,1,127,1920,109,44,0x06,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{1,0,3,12,0,20,720,5,5,0,128,1280,202,40,0x06,0,0,0,0,0,0,0,0,0,0,0,0,1},
	{2,0,3,12,0,30,480,9,6,0,60,720,16,62,0x06,0,0,0,0,0,0,0,0,0,0,0,0,1},
};

struct_lds_clk_param ids_clk[4] = 
{
	{0x13,0x1111},
	{0x24,0x0909},
	{0x24,0x1515},
	{0x1A,0x1D1D},
};

BOOL lcd_change_timing(LCD_TIMING timing)
{
	unsigned int temp;
	int i;
	
/*close lcd screen*/
	if(timing != 0)
	{
		temp = readl(GPFCON);
		temp &= ~(0x3<<12);
		temp |= 0x1<<12;
		writel(temp,GPFCON);

		temp = readl(GPFDAT);
		temp &= ~(0x1<<6);
		writel(temp,GPFDAT);
	}	

/*disable lcd output before change timing*/	
	temp = readl(IMAP_LCDCON1);
	temp &= ~(0x1<<0);
	writel(temp,IMAP_LCDCON1);

	temp = readl(IMAP_OVCW0CR);
	temp &= ~(0x1<<0);
	writel(temp,IMAP_OVCW0CR);

/*change timing*/
	lcd_config_clk(timing);
	lcd_config_controller(timing);

/*enable lcd data output*/	
	temp = readl(IMAP_LCDCON1);
	temp |= (0x1<<0);
	writel(temp,IMAP_LCDCON1);

	temp = readl(IMAP_OVCW0CR);
	temp |= (0x1<<0);
	writel(temp,IMAP_OVCW0CR);

	if(timing == 0)
	{
		temp = readl(GPFCON);
		temp &= ~(0x3<<12);
		temp |= 0x1<<12;
		writel(temp,GPFCON);

		temp = readl(GPFDAT);
		temp |= (0x1<<6);
		writel(temp,GPFDAT);
	}
}

void lcd_config_clk(LCD_TIMING timing)
{

	unsigned int temp;

	temp = readl(DPLL_CFG); 
	temp &=~(1<<31);
	writel(temp,DPLL_CFG);

	temp = readl(DPLL_CFG); 
	temp = ids_clk[timing].DPLLCFG;
	writel(temp,DPLL_CFG);

	//enable dpll	
	temp = readl(DPLL_CFG); 
	temp |=(1<<31);
	writel(temp,DPLL_CFG);

	/*wait untill dpll is locked*/
	while(!(readl(PLL_LOCKED) & 0x2));

	temp = readl(DIV_CFG4);
	temp = ids_clk[timing].DIVCFG4;
	writel(temp,DIV_CFG4);
}

void lcd_config_controller(LCD_TIMING timing)
{	
	unsigned int reg_temp[5];

	reg_temp[0] = (ids_timing[timing].VCLK <<	8) |		
				(ids_timing[timing].EACH_FRAME << 7) |		
				(ids_timing[timing].LCD_PANNEL << 5) |		
				(ids_timing[timing].BPP_MODE << 1) |
				(ids_timing[timing].LCD_OUTPUT);

	reg_temp[1] = ((ids_timing[timing].VBPD -1) << 24) |
				(((ids_timing[timing].VACTIVE -1) & ~0x400) <<14) |
				((ids_timing[timing].VFPD -1 ) << 6) |
				((ids_timing[timing].VSPW -1));

	reg_temp[2] = (ids_timing[timing].VACTIVE_HIGHBIT <<31) |
				((ids_timing[timing].HBPD -1) << 19) |
				((ids_timing[timing].HACTIVE -1) << 8) |
				((ids_timing[timing].HFPD -1));

	reg_temp[3] = ((ids_timing[timing].HSPW -1));

	reg_temp[4] = ((ids_timing[timing].COLOR_MODE) << 24)|
				((ids_timing[timing].BPP24BL) << 12)|
				((ids_timing[timing].FRM565) << 11) |
				((ids_timing[timing].INVVCLK) << 10)|
				((ids_timing[timing].INVVLINE) <<9 )|
				((ids_timing[timing].INVVFRAME) <<8)|
				((ids_timing[timing].INVVD) <<7) |
				((ids_timing[timing].INVVDEN) << 6) |
				((ids_timing[timing].INVPWREN) << 5)|
				((ids_timing[timing].INVENDLINE) << 4)|
				((ids_timing[timing].PWREN) << 3) |
				((ids_timing[timing].ENLEND) <<2 ) |
				((ids_timing[timing].BSWP) << 1) |
				((ids_timing[timing].HWSWP));
				
	writel(reg_temp[0],IMAP_LCDCON1);
	writel(reg_temp[1],IMAP_LCDCON2);
	writel(reg_temp[2],IMAP_LCDCON3);
	writel(reg_temp[3],IMAP_LCDCON4);
	writel(reg_temp[4],IMAP_LCDCON5);
}



int pt_hdmi_cable_check(void) 
{
	unsigned char Readed_EDID[256];
	memset(Readed_EDID, 0xFF, 256);
	if(!Downstream_Rx_read_EDID(Readed_EDID))
		return 1;	
	else
		return 0;
}

int pt_hdmi_init(void)
{
	lcd_change_timing(HDMI_1080P);
	udelay(100000);
	hdmi_main();
	return 0;
}

int pt_hdmi_release(void) 
{
	lcd_change_timing(LCD);
	return 0;
}
