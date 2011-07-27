/***************************************************************************** 
 ** imapx200_cam.c 
 ** 
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 ** 
 ** Description: main file of imapx200 media encode driver
 **
 ** Author:
 **     neville <haixu_fu@infotm.com>
 **      
 ** Revision History: 
 ** ­­­­­­­­­­­­­­­­­ 
 ** 1.1  06/24/2010 neville 
*******************************************************************************/

#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/slab.h>
#include <linux/poll.h>  
#include <linux/ioport.h>
#include <linux/device.h>
#include <linux/sched.h>
#include <linux/types.h>    
#include <linux/interrupt.h>
#include <linux/init.h>      
#include <linux/string.h>
#include <linux/mm.h>             
#include <linux/platform_device.h>
#include <linux/mutex.h>
#include <linux/wait.h>
#include <linux/io.h>
#include <linux/ioctl.h>
#include <linux/clk.h>
#include <linux/delay.h>
#include <linux/i2c.h>

#include <plat/clock.h>

#include <mach/imapx_base_reg.h>
#include <mach/irqs.h>
#include "imapx200_cam.h"

#include "ov2655.h"
#include "ov9650.h"
#include "ov7670.h"

//#define D50HZ	1

static struct class *camif_class;
unsigned int frame = 0;
struct clk  *imap_cam_clk;

static unsigned int camif_open_count;

static wait_queue_head_t wait_camif;
static volatile int pr_flag;
static volatile int co_flag;
static struct mutex pr_mutex;
static struct mutex co_mutex;
struct imapx200_camif_param_t  *param;
struct timeval time1, time2;

static int IIC_Read(unsigned char IICAddr, unsigned char * ByteAddr, unsigned char *Data, unsigned int Size)
{
	struct i2c_adapter *adapter;
	int ret;

	struct i2c_msg msgs[] = { 
		{
			.addr   = IICAddr,
			.flags  = 0,
			.len            = 2,
			.buf            = ByteAddr,

		},{
			.addr   = IICAddr,
			.flags  = I2C_M_RD,
			.len            = Size,
			.buf            = Data,
		}
	};
	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		//printk(KERN_ERR "[IIC_Read]: can't get i2c adapter\n");
		return -1; 
	}
 	ret = i2c_transfer(adapter, msgs, 2);
	if ( ret != 2 )
	{
		 //printk(KERN_ERR "[IIC_Read]: transfer exception with %d \n",ret);
		return -1; 
	}

	return 0;
}

static int IIC_Write(unsigned char IICAddr, unsigned char * data, enum sensor_type  sensor)
{
	struct i2c_adapter *adapter;
	unsigned int 	size = 3;
	unsigned int ret;
	
	struct i2c_msg msgs[] = { 
		{
			.addr   = IICAddr,
			.flags  = 0,
			.len            = size,
			.buf            = data,
		}
	};

	adapter = i2c_get_adapter(1);
	if (!adapter)
	{
		//printk(KERN_ERR "[IIC_Write]: can't get i2c adapter\n");
		return -1; 
	}
	ret = i2c_transfer(adapter, msgs, 1) ;
	if (ret!= 1)
	{
		//printk(KERN_ERR "transfer exception %d\n",ret);
		return -1; 
	}

	return 0;
}

static inline u32 imapx200_cam_readl(struct imapx200_camif_param_t *param,  int offset)
{
	return readl(param->ioaddr + offset);

}

static  inline void imapx200_cam_writel(struct imapx200_camif_param_t *param, int offset, u32 value)
{
	writel(value, param->ioaddr + offset);
}


static int imapx200_cam_open(struct inode *inode, struct file *file)
{
	camif_open_count++;
	return CAMIF_RET_OK;
}

static int imapx200_cam_release(struct inode *inode, struct file *file)
{
	camif_open_count--;
	return CAMIF_RET_OK;

}

#ifdef CONFIG_OV2655
static int  imapx200_cam_ov2655_switch_low_svga(void)
{
	int i, ret;	

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
		
	for(i = 0; i < ((sizeof(ov2655_svga_low_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_svga_low_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to low svga :i=%d\n",i);
		}
	}
	mdelay(30);

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int  imapx200_cam_ov2655_switch_high_svga(void)
{
	int i, ret;
	
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_svga_high_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_svga_high_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to high svga :i=%d\n",i);
		}
	}
	mdelay(30);
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int  imapx200_cam_ov2655_switch_high_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_high_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_high_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to high xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int imapx200_cam_ov2655_switch_upmid_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_upmid_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_upmid_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to mid xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int  imapx200_cam_ov2655_switch_mid_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_mid_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_mid_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to mid xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int imapx200_cam_ov2655_svga_to_xuga(void){
/********************************************\
				STOP PREVIEW
\********************************************/
	int i, ret;
	char gain = 0;

	char reg0x3013;
	char reg0x3002, reg0x3003;
	char reg0x3000;
#ifdef D50HZ
	char reg0x3070, reg0x3071;
#else
	char reg0x3072, reg0x3073;
#endif

	uint32_t shutter;
	uint32_t extra_lines;
	uint32_t preview_exposure;
	uint32_t preview_gain16;
	uint32_t preview_dummy_pixel;
	uint32_t capture_dummy_pixel;
	uint32_t capture_dummy_line;
	uint32_t preview_pclk_frequency;
	uint32_t capture_pclk_frequency;
	uint32_t capture_max_gain;
	uint32_t capture_max_gain16;
	uint32_t preview_line_width;
	uint32_t capture_line_width;
	uint32_t capture_maximum_shutter;
	uint32_t capture_exposure;
	uint32_t preview_banding_filter;
	uint32_t capture_banding_filter = 0; 
	uint32_t gain_exposure;
	uint32_t capture_gain16;
	uint32_t capture_gain;

	struct ov2655_regval_list ov2655_stop_preview[] = {
		{0x30, 0x13, 0x00},
	};
	struct ov2655_regval_list ov2655_3002[] = {
		{0x30, 0x02, 0x00},
	};
	struct ov2655_regval_list ov2655_3003[] = {
		{0x30, 0x03, 0x00},
	};
	struct ov2655_regval_list ov2655_3000[] = {
		{0x30, 0x00, 0x00},
	};

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);

	// stop AE/AG
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3013), &reg0x3013, 1);
	reg0x3013 = reg0x3013 & 0xfa;
        ov2655_stop_preview->value	= reg0x3013;	

	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_stop_preview), SENSOR_OV2655);
	if(ret)
	{
		camif_error("Failed to transfer data to i2c\n");
		return -1;
	}
	else{
		//printk("stop AE/AG\n");
	}

	// read back preview shutter
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3002), &reg0x3002, 1);
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3003), &reg0x3003, 1);
	shutter = (((uint32_t)reg0x3002)<<8) + (uint32_t)reg0x3003;
	//printk(" shutter %x\n", shutter);

	// preview exposure
	preview_exposure = shutter;
	//printk(" preview_exposure %x\n", preview_exposure);

	// read back gain for preview
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3000), &reg0x3000, 1);
	preview_gain16 = (uint32_t)(16 + (reg0x3000 & 0x0f));
	preview_gain16 = (preview_gain16 * (((reg0x3000 & 0x80)>>7)+1) * (((reg0x3000 & 0x40)>>6)+1) * (((reg0x3000 & 0x20)>>5)+1) * (((reg0x3000 & 0x10)>>4)+1));
	//printk(" preview_gain16 %x\n", preview_gain16);

/********************************************\
		CALCULATE CAPTURE EXPOSURE	
\********************************************/
	// dummy pixel and dummy line could be insert for capture
	preview_dummy_pixel = 0;
	capture_dummy_pixel = 0;
	capture_dummy_line = 0;
	preview_pclk_frequency = 540;  	// 	*2
	capture_pclk_frequency = 324;	//	*1
//	preview_pclk_frequency = 1;
//	capture_pclk_frequency = 1;
	
	capture_max_gain   = 1;		// from 1x to 32x
	capture_max_gain16 = capture_max_gain * 16;
	preview_line_width = 1940 + preview_dummy_pixel;
	capture_line_width = 1940 + capture_dummy_pixel;
	capture_maximum_shutter = 1236 + capture_dummy_line;
	capture_exposure = preview_exposure * capture_pclk_frequency * preview_line_width;
	//printk(" capture_exposure %x\n", capture_exposure);
	capture_exposure = capture_exposure / (preview_pclk_frequency * capture_line_width);
	//printk(" capture_exposure %x\n", capture_exposure);

	// calculate banding filter
#ifdef D50HZ
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3070), &reg0x3070, 1);
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3071), &reg0x3071, 1);
	preview_banding_filter = (((uint32_t)reg0x3071)<<8) + (uint32_t)reg0x3070;
	//printk(" preview_banding_filter %x\n", preview_banding_filter);
#else
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3072), &reg0x3072, 1);
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3073), &reg0x3073, 1);
	preview_banding_filter = (((uint32_t)reg0x3073)<<8) + (uint32_t)reg0x3072;
	//printk(" preview_banding_filter %x\n", preview_banding_filter);
#endif

//	capture_banding_filter = preview_banding_filter * capture_pclk_frequency;
//  	capture_banding_filter = capture_banding_filter * preview_line_width;
//	capture_banding_filter = capture_banding_filter / preview_pclk_frequency;
//	capture_banding_filter = capture_banding_filter / capture_line_width;
	capture_banding_filter = 84;
	//printk(" capture_banding_filter %x\n", capture_banding_filter);

	// redistribute gain and exposure
	gain_exposure = preview_gain16 * capture_exposure;
	//printk(" gain_exposure %x\n", gain_exposure);
	if( gain_exposure < capture_banding_filter * 16 ) {
		capture_exposure = gain_exposure / 16;
		capture_gain16 = (gain_exposure*2 + 1)/capture_exposure;
		capture_gain16 = capture_gain16 >> 1;
		//printk(" -------------------1\n");
	}
	else {
		if( gain_exposure > capture_maximum_shutter * 16 ) {
			capture_exposure = capture_maximum_shutter;
			//printk(" capture_exposure %x\n", capture_exposure);
			capture_gain16 = (gain_exposure*2 + 1)/capture_maximum_shutter;
			capture_gain16 = capture_gain16 >> 1;
			if( capture_gain16 > capture_max_gain16 ) {
//				capture_exposure = (gain_exposure + (gain_exposure/10))/capture_max_gain16;
//				capture_exposure = capture_exposure/16;
//				capture_exposure = capture_exposure/capture_banding_filter;
//				capture_exposure = capture_exposure*capture_banding_filter;
				capture_exposure = (gain_exposure + (gain_exposure/10)) * capture_banding_filter;
				capture_exposure = capture_exposure / (16 * capture_max_gain16 * capture_banding_filter);	
				capture_gain16 = (gain_exposure*2 + 1)/capture_exposure;
				capture_gain16 = capture_gain16 >> 1;
				//printk(" -------------------2\n");
			}
			else {
//				capture_exposure = capture_exposure/16;
//				capture_exposure = capture_exposure/capture_banding_filter;
//				capture_exposure = capture_exposure*capture_banding_filter;
				capture_exposure = capture_exposure * capture_banding_filter;
				capture_exposure = capture_exposure / capture_banding_filter;
				capture_gain16 = (gain_exposure*2 + 1)/capture_maximum_shutter;
				capture_gain16 = capture_gain16 >> 1;
				//printk(" -------------------3\n");
			}
		}
		else {
//			capture_exposure = gain_exposure/16;
//			capture_exposure = capture_exposure/capture_banding_filter;
//			capture_exposure = capture_exposure*capture_banding_filter;
			capture_exposure = gain_exposure * capture_banding_filter;
			capture_exposure = capture_exposure / (16 * capture_banding_filter);
			capture_gain16 = (gain_exposure*2 + 1)/capture_exposure;
			capture_gain16 = capture_gain16 >> 1;
			//printk(" -------------------4\n");
		}
	}
	//printk(" capture_exposure %x\n", capture_exposure);
	//printk(" capture_gain16 %x\n", capture_gain16);

/********************************************\
			   SWITCH TO XUGA		
\********************************************/
	for(i = 0; i < ((sizeof(ov2655_xuga_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
//			//printk(KERN_ERR "to xuga:i=%x\n",i);
		}
	}

/********************************************\
			   WRITE REGISTRES 
\********************************************/
	// write exposure
	if( capture_exposure > capture_maximum_shutter ) {
		shutter = capture_maximum_shutter;
		extra_lines = capture_exposure - capture_maximum_shutter;
	}
	else {
		shutter = capture_exposure;
		extra_lines = 0;
	}
	reg0x3003 = shutter & 0x00ff;
	reg0x3002 = (shutter >>8) & 0x00ff;
	ov2655_3002->value = reg0x3002;
	ov2655_3003->value = reg0x3003;
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3002), SENSOR_OV2655);
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3003), SENSOR_OV2655);
	//printk(" ov2655_3002 %x\n ov2655_3003 %x\n", ov2655_3002->value, ov2655_3003->value);
	if(ret)
	{
		camif_error("Failed to transfer data to i2c\n");
		return -1;
	}
	else{
		//printk("write exposure\n");
	}

	// write gain
	capture_gain = capture_gain16;
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x10;
	}
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x20;
	}
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x40;
	}
	if( capture_gain16 > 16 ) {
		capture_gain16 = capture_gain16 / 2;
		gain = 0x80;
	}
	gain = gain | (char)(capture_gain - 16);
	ov2655_3000->value = gain;
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3000), SENSOR_OV2655);
	//printk(" ov2655_3000 %x\n", ov2655_3000->value);
	if(ret)
	{
		camif_error("Failed to transfer data to i2c\n");
		return -1;
	}
	else{
		//printk("write gain\n");
	}

	mdelay(30);
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);

	return 0;
}


static int imapx200_cam_ov2655_get_3008(void){
	char buf;

	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3000), &buf,1);
	//printk(" OV2655_3000 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3002), &buf,1);
	//printk(" OV2655_3002 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3003), &buf,1);
	//printk(" OV2655_3003 %x\n",buf);                                 
/*	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_301c), &buf,1);
	//printk(" OV2655_301c %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3070), &buf,1);
	//printk(" OV2655_3070 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3013), &buf,1);
	//printk(" OV2655_3013 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3014), &buf,1);
	//printk(" OV2655_3014 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3028), &buf,1);
	//printk(" OV2655_3028 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3029), &buf,1);
	//printk(" OV2655_3029 %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302a), &buf,1);
	//printk(" OV2655_302a %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302b), &buf,1);
	//printk(" OV2655_302b %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302d), &buf,1);
	//printk(" OV2655_302d %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_302e), &buf,1);
	//printk(" OV2655_302e %x\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3011), &buf,1);
	//printk(" OV2655_3011 %x\n",buf);                                 
*/	return 0;
}


static int imapx200_cam_ov2655_to_preview(void)
{
	int i, ret;
	char reg0x3013;
	struct ov2655_regval_list ov2655_3013[] = {
		{0x30, 0x13, 0x00},
	};
	
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_3013), &reg0x3013,1);
	reg0x3013 = reg0x3013 | 0x05;   
	ov2655_3013->value = reg0x3013;
	ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_3013), SENSOR_OV2655);                       

	for(i = 0; i < ((sizeof(ov2655_to_preview_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_to_preview_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to preview xuga:i=%d\n",i);
		}
	}
	return 0;
}


static int  imapx200_cam_ov2655_switch_low_xuga(void)
{
	int i, ret;

	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_before), SENSOR_OV2655);
	mdelay(5);
	for(i = 0; i < ((sizeof(ov2655_xuga_low_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_xuga_low_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to low xuga:i=%d\n",i);
		}
	}
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(ov2655_after), SENSOR_OV2655);
	return 0;
}

static int imapx200_cam_ov2655_init(void)
{
	char buf = 0;
	int i, ret;
	uint32_t tmp;
#ifdef CONFIG_IMAP_PRODUCTION_P1011A	 
	tmp = readl(rGPICON);        
	tmp &= ~(0x3 << 12);         
	tmp |= (0x1 << 12);          
	writel(tmp,rGPICON);         
	tmp = readl(rGPIDAT);        
	tmp &= ~(0x1 << 6);          
	writel(tmp,rGPIDAT);         
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P0811B
	tmp = readl(rGPPCON);
	tmp &= ~(0x3 << 20); 
	tmp |= (0x1 << 20);          
	writel(tmp,rGPPCON);         
	tmp = readl(rGPPDAT);        
	tmp &= ~(0x1 << 10);          
	writel(tmp,rGPPDAT);         
#endif

#ifdef CONFIG_IMAP_PRODUCTION_P1011B
	tmp = readl(rGPPCON);
	tmp &= ~(0x3 << 20); 
	tmp |= (0x1 << 20);          
	writel(tmp,rGPPCON);         
	tmp = readl(rGPPDAT);        
	tmp &= ~(0x1 << 10);          
	writel(tmp,rGPPDAT);         
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011C
#endif
	IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_init_regs[0]), SENSOR_OV2655);
	msleep(50);

	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_PIDH), &buf, 1);
	if(buf != 0x26)
		//printk("i2c config OV2655 wrong with %d\n",buf); 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_PIDL), &buf, 1);
	if(buf != 0x56)
		//printk("i2c config OV2655 wrong with %d\n",buf); 
/*
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_AEC_time1), &buf,1);
	//printk("INIT-Sensor AEC_3002 %d\n",buf);                                 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_AEC_time2), &buf,1);
	//printk("INIT-Sensor AEC_3003 %d\n",buf);                                 
*/

	for(i = 0; i < ((sizeof(ov2655_init_regs) / 3) -1); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_init_regs[i+1]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk("i=%d\n",i);
		}
	}
/*
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_28), &buf, 1);
		//printk("i2c config OV2655_28 wrong with %d\n",buf); 
	IIC_Read(OV2655_I2C_RADDR, (unsigned char *)(&OV2655_29), &buf, 1);
		//printk("i2c config OV2655_29 wrong with %d\n",buf); 
*/
	return 0;

}

static int imapx200_cam_ov2655_close(void)
{
	int i, ret;
	uint32_t tmp;
	
	for(i = 0; i < ((sizeof(ov2655_stop_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char *)(&ov2655_stop_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to xuga:i=%d\n",i);
		}
	}

// PULL PWDN TO DODD(hign)
#ifdef CONFIG_IMAP_PRODUCTION_P1011A
	tmp = readl(rGPICON);        
	tmp &= ~(0x3 << 12);         
	tmp |= (0x1 << 12);          
	writel(tmp,rGPICON);         
	tmp = readl(rGPIDAT);        
	tmp |= (0x1 << 6);          
	writel(tmp,rGPIDAT);         
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011B
	tmp = readl(rGPPCON);
	tmp &= ~(0x3 << 20); 
	tmp |= (0x1 << 20);          
	writel(tmp,rGPPCON);         
	tmp = readl(rGPPDAT);        
	tmp |= (0x1 << 10);          
	writel(tmp,rGPPDAT);         
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P0811B
	tmp = readl(rGPPCON);
	tmp &= ~(0x3 << 20); 
	tmp |= (0x1 << 20);          
	writel(tmp,rGPPCON);         
	tmp = readl(rGPPDAT);        
	tmp |= (0x1 << 10);          
	writel(tmp,rGPPDAT);         
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011C
#endif
	
	return 0; 
} 

/*****************************************************\
 *************** Special Effects *********************
\*****************************************************/ 

static int imapx200_cam_ov2655_sepia(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sepia_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sepia_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sepia:i=%d\n",i);
		}
	} 
	
	return 0;
}  

static int imapx200_cam_ov2655_bluish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_bluish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_bluish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to bluish:i=%d\n",i);
		}
	} 
	
	return 0;
} 

static int imapx200_cam_ov2655_greenish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_greenish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_greenish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to greenish:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_reddish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_reddish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_reddish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to reddish:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_yellowish(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_yellowish_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_yellowish_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to yellowish:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_bandw(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_bandw_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_bandw_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to bandw:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_negative(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_negative_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_negative_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to negative:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_normal(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_normal_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_normal_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to normal:i=%d\n",i);
		}
	} 
	
	return 0;
}

/*****************************************************\
 *************** Light Mode **************************
\*****************************************************/ 

static int imapx200_cam_ov2655_auto(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_auto_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_auto_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to auto:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_sunny(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sunny_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sunny_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sunny:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_cloudy(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_cloudy_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_cloudy_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to cloudy:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_office(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_office_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_office_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to office:i=%d\n",i);
		}
	} 
	
	return 0;
}

static int imapx200_cam_ov2655_home(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_home_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_home_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to home:i=%d\n",i);
		}
	} 
	
	return 0;
}

/*****************************************************\
 *************** Color Saturation ********************
\*****************************************************/ 

static int imapx200_cam_ov2655_saturation_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation +2:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int imapx200_cam_ov2655_saturation_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation +1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int imapx200_cam_ov2655_saturation_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation +0:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_saturation_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation -1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_saturation_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_saturation_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_saturation_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to saturation -2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Brightness **************************
\*****************************************************/ 

static int imapx200_cam_ov2655_brightness_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +3:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int imapx200_cam_ov2655_brightness_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int imapx200_cam_ov2655_brightness_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_brightness_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness +0:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_brightness_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness -1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_brightness_5(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_5_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_5_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness -2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_brightness_6(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_brightness_6_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_brightness_6_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to brightness -3:=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Contrast ****************************
\*****************************************************/
		
static int imapx200_cam_ov2655_contrast_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +3:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int imapx200_cam_ov2655_contrast_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int imapx200_cam_ov2655_contrast_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_contrast_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast +0:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_contrast_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast -1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_contrast_5(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_5_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_5_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast -2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_contrast_6(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_contrast_6_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_contrast_6_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to contrast -3:=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Sharpness ***************************
\*****************************************************/
                                                        
static int imapx200_cam_ov2655_sharpness_0(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_0_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_0_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 0:i=%d\n",i);
		}
	} 
	
	return 0 ;
} 

static int imapx200_cam_ov2655_sharpness_1(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_1_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_1_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 1:i=%d\n",i);
		}
	} 
	
	return 0 ;
}
 
static int imapx200_cam_ov2655_sharpness_2(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_2_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_2_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 2:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_sharpness_3(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_3_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_3_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 3:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_sharpness_4(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_4_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_4_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness 4:i=%d\n",i);
		}
	} 
	
	return 0 ;
}

static int imapx200_cam_ov2655_sharpness_auto(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_sharpness_auto_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_sharpness_auto_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to sharpness auto:=%d\n",i);
		}
	} 
	
	return 0 ;
}

/*****************************************************\
 *************** Night Mode **************************
\*****************************************************/
                                                        
static int imapx200_cam_ov2655_night_mode_on(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_night_mode_on_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_night_mode_on_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to night mode on:=%d\n",i);
		}
	} 
	
	return 0 ;
}
                                                        
static int imapx200_cam_ov2655_night_mode_off(void)
{ 
	int i, ret; 
	
	for(i = 0; i< ((sizeof(ov2655_night_mode_off_regs) / 3)); i++)
	{
		ret = IIC_Write(OV2655_I2C_WADDR, (unsigned char*)(&ov2655_night_mode_off_regs[i]), SENSOR_OV2655);
		if(ret)
		{
			camif_error("Failed to transfer data to i2c\n");
			return -1;
		}
		else{
		//	//printk(KERN_ERR "to night mode off:=%d\n",i);
		}
	} 
	
	return 0 ;
}

#endif

#ifdef CONFIG_OV7670
static int  imapx200_cam_ov7670_switch_svga(void)
{
	return 0;
}
static int  imapx200_cam_ov7670_switch_xuga(void)
{
	return 0;
}
#endif

#ifdef CONFIG_OV9650
static int  imapx200_cam_ov9650_switch_svga(void)
{
	return 0;
}
static int  imapx200_cam_ov9650_switch_xuga(void)
{
	return 0;
}
#endif 

static int imapx200_cam_ioctl(struct inode *inode, struct file *file, unsigned int cmd, unsigned long arg) 
{
	int value;
	int ret;
	switch(cmd)
	{
		case PR_WAIT_OK:
			mutex_lock(&pr_mutex);
			pr_flag = 1;
			mutex_unlock(&pr_mutex);
			wait_event_interruptible_timeout(wait_camif, pr_flag == 0, HZ/4);
//			imapx200_cam_ov2655_get_3008();
			break;
		case CO_WAIT_OK:
			mutex_lock(&co_mutex);
			co_flag = 1;
			mutex_unlock(&co_mutex);
			wait_event_interruptible_timeout(wait_camif, co_flag == 0, HZ/ 4);
			imapx200_cam_ov2655_get_3008();
			break;
		case SENSOR_SET_MODE:
			if(copy_from_user(&value, (int *)arg, sizeof(int)))
			{
				ret = -EFAULT;
				return ret;
			}

			switch(value)
			{
				case INIT_SENSOR:
#ifdef CONFIG_OV2655
					imapx200_cam_ov2655_init();
#endif
#ifdef CONFIG_OV7670
					imapx200_cam_ov7670_init();
#endif			
#ifdef CONFIG_OV9650
					imapx200_cam_ov9650_init();
#endif			
					break;

				case SWITCH_SENSOR_TO_HIGH_SVGA:
#ifdef CONFIG_OV2655
					imapx200_cam_ov2655_switch_high_svga();
#endif
#ifdef CONFIG_OV7670
					imapx200_cam_ov7670_switch_svga();
#endif			
#ifdef CONFIG_OV9650
					imapx200_cam_ov9650_switch_svga();
#endif			
					break;
				case SWITCH_SENSOR_TO_LOW_SVGA:
					imapx200_cam_ov2655_switch_low_svga();
					break;
				case SWITCH_SENSOR_TO_HIGH_XUGA:
					imapx200_cam_ov2655_svga_to_xuga();
					imapx200_cam_ov2655_switch_high_xuga();
					break;

				case SWITCH_SENSOR_TO_UPMID_XUGA:
					imapx200_cam_ov2655_svga_to_xuga();
					imapx200_cam_ov2655_switch_upmid_xuga();
					break;

				case SWITCH_SENSOR_TO_MID_XUGA:
					imapx200_cam_ov2655_svga_to_xuga();
					imapx200_cam_ov2655_switch_mid_xuga();
					break;

				case SWITCH_SENSOR_TO_LOW_XUGA:
					imapx200_cam_ov2655_svga_to_xuga();
					imapx200_cam_ov2655_switch_low_xuga();
					break;

				case SENSOR_TO_PREVIEW:
					//printk(" back to preview\n");
					//imapx200_cam_ov2655_get_3008();
					imapx200_cam_ov2655_to_preview();

					break;
				case CLOSE_SENSOR:
#ifdef CONFIG_OV2655
					//printk(KERN_ERR "SENSOR_CLOSE  KERNAL IOCTL\n");
					imapx200_cam_ov2655_close();
#endif
#ifdef CONFIG_OV7670
					imapx200_cam_ov7670_close();
#endif			
#ifdef CONFIG_OV9650
					imapx200_cam_ov9650_close();
#endif
					break;
				default: 
					break;

			}
			break;

		case SENSOR_SET_WHITE_BALANCE:
			if(copy_from_user(&value, (int *)arg, sizeof(int)))
			{
				ret = -EFAULT;
				return ret;
			}

			switch(value)
			{
				case SENSOR_WB_AUTO:
					imapx200_cam_ov2655_auto();
					break;
				case SENSOR_WB_CUSTOM:
					break;
				case SENSOR_WB_INCANDESCENT:
					imapx200_cam_ov2655_home();
					break;
				case SENSOR_WB_FLUORESCENT:
					imapx200_cam_ov2655_office();
					break;
				case SENSOR_WB_DAYLIGHT:
					imapx200_cam_ov2655_sunny();
					break;
				case SENSOR_WB_CLOUDY:
					imapx200_cam_ov2655_cloudy();
					break;
				case SENSOR_WB_TWILIGHT:
					break;
				case SENSOR_WB_SHADE:
					break;
				default:
					break;
			}
			break;
		case SENSOR_SET_COLOR_EFECT:
			if(copy_from_user(&value, (int *)arg, sizeof(int)))
			{
				ret = -EFAULT;
				return ret;
			}
			switch(value)
			{
				case SENSOR_EFFECT_OFF:
					imapx200_cam_ov2655_normal();
					break;
				case SENSOR_EFFECT_MONO:
					imapx200_cam_ov2655_bandw();
					break;
				case SENSOR_EFFECT_NEGATIVE:
					imapx200_cam_ov2655_negative();
					break;
				case SENSOR_EFFECT_SOLARIZE:
					imapx200_cam_ov2655_yellowish();
					break;
				case SENSOR_EFFECT_PASTEL:
					imapx200_cam_ov2655_reddish();
					break;
				case SENSOR_EFFECT_MOSAIC:
					break;
				case SENSOR_EFFECT_RESIZE:
					break;
				case SENSOR_EFFECT_SEPIA:
					imapx200_cam_ov2655_sepia();
					break;
				case SENSOR_EFFECT_POSTERIZE:
					imapx200_cam_ov2655_bluish();
					break;
				case SENSOR_EFFECT_WHITEBOARD:
					break;
				case SENSOR_EFFECT_BLACKBOARD:
					break;
				case SNESOR_EFFECT_AQUA:	
					imapx200_cam_ov2655_greenish();
					break;
				default:
					break;
			}
			break;
		case SENSOR_SET_ANTIBANDING:
			break;
		case SENSOR_SET_BRIGHTNESS:
			break;
		case SENSOR_SET_NIGHT_MODE:
			if(copy_from_user(&value, (int *)arg, sizeof(int)))
			{
				ret = -EFAULT;
				return ret;
			}
			switch(value)
			{
				case SENSOR_SCENCE_AUTO:
//					imapx200_cam_ov2655_night_mode_off();
					break;
				case SENSOR_SCENCE_NIGHT:
//					imapx200_cam_ov2655_night_mode_on();
					break;
				default:
					break;
			}
			break;
		default:
			break;

	}

	return CAMIF_RET_OK;
}



static struct file_operations imapx200_cam_fops = 
{
		owner:		THIS_MODULE,
		open:		imapx200_cam_open,
		release:	imapx200_cam_release,
		ioctl:		imapx200_cam_ioctl,
};

static irqreturn_t imapx200_cam_irq_handle(int irq ,void *dev_id)
{
	u32 intmask;
	unsigned int clear_int = 0;
	irqreturn_t ret;
	intmask = readl(param->ioaddr+IMAP_CICPTSTATUS);
	////printk(KERN_ERR "get irq 0x%08x\n",intmask);
	
	if(!intmask || intmask == 0xFFFFFFFF)
	{
		ret = IRQ_NONE;
		return ret;
	}
	if(intmask & CAMIF_OVERFLOW)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= (intmask & CAMIF_OVERFLOW);
	}
	if(intmask & CAMIF_UNDERFLOW)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= (intmask & CAMIF_UNDERFLOW);
	}

	if(intmask & PRFIFO_DIRTY)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= PRFIFO_DIRTY;
	}
	if(intmask & COFIFO_DIRTY)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= COFIFO_DIRTY;
	}
	if(intmask & CERR656)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= CERR656;
	}

	if(intmask & PR_LEISURE)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= PR_LEISURE;
	}

	if(intmask & CO_LEISURE)
	{
	//	camif_error("get IRQ %08x\n",intmask);
		clear_int |= CO_LEISURE;
	}
	
	if(intmask & PR_DMA_SUCCESS)
	{
		mutex_lock(&pr_mutex);
		/*	
		if(frame == 0)
		{
			do_gettimeofday(&time1);
		}
		if(frame == 29)
		{
			do_gettimeofday(&time2);
			//printk(KERN_ERR "30frame cost %ds,%ld us\n",(time2.tv_sec - time1.tv_sec) , (time2.tv_usec - time1.tv_usec));
			time1.tv_sec = time2.tv_sec = time2.tv_usec = time1.tv_usec = 0;
		}
		if( frame!=29)
			frame++;
		else frame = 0;
		*/
		if(pr_flag == 1)
		{
			wake_up_interruptible(&wait_camif);
		}
		pr_flag = 0;
		mutex_unlock(&pr_mutex);
		clear_int |= PR_DMA_SUCCESS;
	}

	if(intmask & CO_DMA_SUCCESS)
	{
		mutex_lock(&co_mutex);
		if(co_flag == 1)
		{
			wake_up_interruptible(&wait_camif);
		}
		co_flag = 0;
		mutex_unlock(&co_mutex);
		clear_int |= CO_DMA_SUCCESS;
	}

	//imapx200_cam_writel(param, IMAP_CICPTSTATUS, (intmask | clear_int));
	writel(intmask, param->ioaddr+IMAP_CICPTSTATUS);
	
	return IRQ_HANDLED;
}

//init camif ,config sensor
static void  imapx200_cam_gpio_init(void)
{
	u32 tmp;

	/*for p1011*/
		
	tmp = readl(rDIV_CFG1);
	tmp &= ~((3 << 16) | (0x1f << 18));
	tmp |= ((2<<16) | (19<<18));
	writel(tmp, rDIV_CFG1);
	
	tmp = readl(rWP_MASK);
	tmp &= ~(0x1<15);
	writel(tmp, rWP_MASK);

	// config it to p1011a to enable 1.8v 2.8v
#ifdef CONFIG_IMAP_PRODUCTION_P1011A
	tmp = readl(rGPACON);
	tmp &= ~(0x3 << 8);
	tmp |=  (0x1 << 8);
	writel(tmp,rGPACON);
	tmp = readl(rGPADAT);
	tmp |= (0x1 << 4);
	writel(tmp,rGPADAT);
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011B
	tmp = readl(rGPACON);
	tmp &= ~(0x3 << 8);
	tmp |=  (0x1 << 8);
	writel(tmp,rGPACON);
	tmp = readl(rGPADAT);
	tmp |= (0x1 << 4);
	writel(tmp,rGPADAT);
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011C
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P0811B
	tmp = readl(rGPACON);
	tmp &= ~(0x3 << 8);
	tmp |=  (0x1 << 8);
	writel(tmp,rGPACON);
	tmp = readl(rGPADAT);
	tmp |= (0x1 << 4);
	writel(tmp,rGPADAT);
#endif
/*
	tmp = readl(rGPICON);        
	tmp &= ~(0x3 << 12);         
	tmp |= (0x1 << 12);          
	writel(tmp,rGPICON);         
	tmp = readl(rGPIDAT);        
	tmp &= ~(0x1 << 6);          
	writel(tmp,rGPIDAT);         
	mdelay(50);
*/
    /*
	tmp = readl(rGPCCON);
	tmp &= ~(0x3<<6 | 0x3<<4 | 0x3<<2 | 0x3<<0);
	tmp |= (0x2<<6 | 0x2<<4 | 0x2<<2 |0x2 << 0);
	writel(tmp, rGPCCON);
    */

    /*
	tmp = readl(rGPFCON);
	tmp &=~(0x3 << 18);
	tmp |= 0x1<<18;
	writel(tmp, rGPFCON);
    */

    /*
	tmp = readl(rGPFDAT);
	tmp &= ~(0x1 << 9);
	writel(tmp, rGPFDAT);
    */
    
	writel(0x2AAAAAA, rGPLCON);

    //here is a bug for board e3
    /*
	tmp = readl(rGPBCON);
	tmp &= ~(0x3<<8);
	writel(tmp, rGPBCON);
    */
}

void imapx200_cam_reset(struct imapx200_camif_param_t *param, enum sensor_type sensor)
{
	u32 tmp;

	switch(sensor) {
		case SENSOR_OV9650:
		case SENSOR_OV7670:
		case SENSOR_OV2655:
			writel(0x0, param->ioaddr + IMAP_CIGCTRL);

			tmp = readl(param->ioaddr+IMAP_CIGCTRL);
			tmp |= (0x1 << 1);
			writel(tmp, param->ioaddr+IMAP_CIGCTRL);
			mdelay(100);

			tmp = readl(param->ioaddr+IMAP_CIGCTRL);
			tmp &= ~(0x1 << 1);
			writel(tmp, param->ioaddr+IMAP_CIGCTRL);
			mdelay(100);

			tmp = readl(param->ioaddr+IMAP_CIGCTRL);
			tmp |= (0x1 << 1);
			writel(tmp, param->ioaddr+IMAP_CIGCTRL);
			tmp = readl(param->ioaddr+IMAP_CIGCTRL);
			//printk(KERN_ERR "++++0X%08xtmp\n",tmp);
			break;
		default:
			break;
	}
}


#ifdef CONFIG_OV7670
static int imapx200_cam_ov7670_init(void) {

	int i, j;
	int ret;
	//char buf;

	/*check senser and i2c*/
	/*
	IIC_Read(OVC7670_I2C_ADDR, REG_MIDH , &buf, 1);
	if(buf!= 0x7f)
		//printk(KERN_ERR "i2c config 0 ov7670 wrong with value %d\n",buf);

	IIC_Read(OVC7670_I2C_ADDR, REG_MIDL, &buf, 1);
	if(buf!= 0xa2)
		//printk(KERN_ERR "i2c config 1 ov7670 wrong with value %d\n",buf);

	IIC_Read(OVC7670_I2C_ADDR, REG_PID, &buf, 1);
	if(buf!= 0x76)
		//printk(KERN_ERR "i2c config 2 ov7670 wrong with value %d\n",buf);

	IIC_Read(OVC7670_I2C_ADDR, REG_VER, &buf, 1);
	if(buf!= 0x73)
		//printk(KERN_ERR "i2c config 3 ov7670 wrong with value %d\n",buf);
		*/
	/*Write data to ov7670 senser*/

	for(i = 0; i < (sizeof(ov7670_regs) / 2); i++)
	{
		if(ov7670_regs[i].reg_num == 0xFF)
		{
			for(j=0; j < 100000;j++);
		}else{
			ret = IIC_Write(OVC7670_I2C_ADDR, (unsigned char *)(&ov7670_regs[i]), SENSOR_OV7670);

			if(ret)
			{
				camif_error("Failed to transfer data to i2c\n");
				return -1;
			}
		}	
			
	}

	
	 return 0;

}
#endif

#ifdef  CONFIG_OV9650
static int imapx200_cam_ov9650_init(void)
{
	//char buf;
	int i,j,ret;

	//printk(KERN_ERR "+++++++++OV9650 INIT+++++++++++++++++");
	

	for(i = 0; i < (sizeof(ov9650_regs) / 2); i++)
	{
		if(ov7670_regs[i].reg_num == 0xFF)
		{
			for(j=0; j < 100000;j++);
		}else{
			ret = IIC_Write(OVC9650_I2C_ADDR, (unsigned char *)( &ov9650_regs[i]),SENSOR_OV9650);

			if(ret)
			{
				camif_error("Failed to transfer data to i2c\n");
				return -1;
			}
		}	
			
	}

	return 0;
}

#endif


static int imapx200_cam_probe(struct platform_device *pdev)
{
	int ret;
	struct resource *res;
	uint32_t tmp;

	imap_cam_clk = NULL;
	camif_open_count = 0;

	/*initualize wait queue*/
	init_waitqueue_head(&wait_camif);
	pr_flag = co_flag = 0;
	//printk(KERN_ERR "iMAP camera interface driver probe\n");
	camif_debug("iMAP camera driver probe\n");

	param = kzalloc(sizeof(struct imapx200_camif_param_t), GFP_KERNEL);
	if(!param) {
		camif_error("alloc buffer failed!\n");
		return  -ENOMEM;
	}

	ret = register_chrdev(CAMIF_DEFAULT_MAJOR, "imapx200_camif", &imapx200_cam_fops);
	if(ret)
	{
		camif_error("imapx register chardev error!\n");
		goto out;
	}

	camif_class = class_create(THIS_MODULE, "imapx200_camif");
	param->dev_id = MKDEV(CAMIF_DEFAULT_MAJOR, CAMIF_DEFAULT_MINOR);
	device_create(camif_class , NULL,  param->dev_id, NULL, "imapx200-camif");
	
	
	param->dev = &pdev->dev;
	
	/*get clock srouce*/
	param->hclk = clk_get(&pdev->dev, "camif");
	if( IS_ERR (param->hclk)) {
		camif_error("failed to get clock\n");
		ret = -ENOENT;
		goto err_io_noclk;
	}
	clk_enable(param->hclk);

	/*get irq number*/
	param->irq = platform_get_irq(pdev,0);
	if(param->irq < 0) {
		camif_error("no irq specified\n");
		ret = param->irq;
		goto err_io_clk;
	}

	ret = request_irq(param->irq, imapx200_cam_irq_handle, IRQF_DISABLED,
			dev_name(&pdev->dev),param);
	if(ret)
		goto err_io_clk;



	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	if(!res) {
		camif_error("no memory specified\n");
		ret = -ENOENT;
		goto err_io_clk;
	}

	param->phy_start = res->start;
	param->phy_size = resource_size(res);
	param->res = request_mem_region(res->start, resource_size(res),
			pdev->name);

	if(!param->res) {
		camif_error("cannot request IO\n");
		ret = -ENXIO;
		goto err_io_clk;
	}
	

	//printk(KERN_ERR " start = %08x,size = %d\n",res->start,resource_size(res));
	param->ioaddr = ioremap_nocache(res->start, resource_size(res));
	if(!param->ioaddr) {
		camif_error("cannot map IO\n");
		ret = -ENXIO;
		goto err_io_mem;
	}

	
	 imapx200_cam_gpio_init();
	 
#ifdef CONFIG_OV2655
	 imapx200_cam_reset(param,  SENSOR_OV2655);
	 //imapx200_cam_ov2655_init();
#endif
#ifdef CONFIG_OV9650
	 imapx200_cam_reset(param,  SENSOR_OV9650);
//	 imapx200_cam_ov9650_init();
#endif
#ifdef CONFIG_OV7670
	 imapx200_cam_reset(param, SENSOR_OV7670);
//	 imapx200_cam_ov7670_init();
#endif
	 if(ret)
	 {
		 camif_error("set sensor failed\n");
		 goto err_io_mem;
	 }

	 /* Initialize mutex */
	 memset(&pr_mutex, 0x00, sizeof(struct mutex));
	 memset(&co_mutex, 0x00, sizeof(struct mutex));
	 mutex_init(&pr_mutex);
	 mutex_init(&co_mutex);
	 tmp = readl(param->ioaddr+IMAP_CIGCTRL);

	return CAMIF_RET_OK;		
err_io_mem:
	release_resource(param->res);
err_io_clk:
	clk_disable(param->hclk);
err_io_noclk:
	kfree(param);
out:
	camif_error("driver probe failed with err %d\n",ret);
	return ret;
}


static int imapx200_cam_remove(struct platform_device *pdev) 
{
	mutex_destroy(&pr_mutex);
	mutex_destroy(&co_mutex);

	iounmap((void *)(param->ioaddr));		
	release_mem_region(param->phy_start, param->phy_size);
	if(param->res)
	{
		release_resource(param->res);
		kfree(param->res);
		param->res = NULL;
	}

	free_irq(param->irq, pdev);
	device_destroy(camif_class, param->dev_id);
	class_destroy(camif_class);
	unregister_chrdev(CAMIF_DEFAULT_MAJOR, "imapx200-camif");
	clk_disable(param->hclk);


	return CAMIF_RET_OK;
}

#ifdef CONFIG_PM
static int imapx200_cam_suspend(struct platform_device *pdev, pm_message_t state)
{
	//imapx200_cam_ov2655_close();
	//printk(KERN_INFO "imapx_cam_suspend -- \n");
	return CAMIF_RET_OK;
}

static int imapx200_cam_resume(struct platform_device *pdev)
{
	uint32_t tmp;
	//printk(KERN_INFO "imapx_cam_resume ++\n");

#ifdef CONFIG_IMAP_PRODUCTION_P1011A
	tmp = readl(rGPACON);
	tmp &= ~(0x3 << 8);
	tmp |=  (0x1 << 8);
	writel(tmp,rGPACON);
	tmp = readl(rGPADAT);
	tmp |= (0x1 << 4);
	writel(tmp,rGPADAT);
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011B
	tmp = readl(rGPACON);
	tmp &= ~(0x3 << 8);
	tmp |=  (0x1 << 8);
	writel(tmp,rGPACON);
	tmp = readl(rGPADAT);
	tmp |= (0x1 << 4);
	writel(tmp,rGPADAT);
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P0811B
	tmp = readl(rGPACON);
	tmp &= ~(0x3 << 8);
	tmp |=  (0x1 << 8);
	writel(tmp,rGPACON);
	tmp = readl(rGPADAT);
	tmp |= (0x1 << 4);
	writel(tmp,rGPADAT);
#endif
#ifdef CONFIG_IMAP_PRODUCTION_P1011C
#endif
	imapx200_cam_reset(param,  SENSOR_OV2655);
	return CAMIF_RET_OK;
}
#else 

#define	imapx200_cam_suspend NULL
#define	imapx200_cam_resume  NULL
#endif


static struct platform_driver imapx200_cam_driver =
{
	.probe			= imapx200_cam_probe,
	.remove			= imapx200_cam_remove,
#ifdef CONFIG_PM
	.suspend		= imapx200_cam_suspend,
	.resume			= imapx200_cam_resume,
#endif	
	.driver			=
	{
		.owner		= THIS_MODULE,
		.name 		= "imapx200_camif",
	},
};



/*
 * init && exit
 */
static int __init imapx200_cam_init(void)
{
	if(platform_driver_register(&imapx200_cam_driver))
	{
		camif_error("Failed to register IMAPX200 CAMIF driver\n");
		return -EPERM;
	}
	camif_debug("IMAPX200 CAMIF driver register OK!\n");
	
	return CAMIF_RET_OK;
}


static void __exit imapx200_cam_exit(void)
{
	platform_driver_unregister(&imapx200_cam_driver);
	camif_debug("IMAPX200 CAMIF driver unregister OK!\n");
}



module_init(imapx200_cam_init);
module_exit(imapx200_cam_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("neville of infoTM");
MODULE_DESCRIPTION("IMAPX200 CAMIF DRIVER");
