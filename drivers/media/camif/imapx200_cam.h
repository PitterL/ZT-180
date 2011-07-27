/***************************************************************************** 
 ** imapx200_cam.h 
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

#ifndef  __IMAPX200_CAM_H__
#define	__IMAPX200_CAM_H__
#endif


#define	CAMIF_RET_OK	0
#define CAMIF_DEFAULT_MAJOR   119
#define	CAMIF_DEFAULT_MINOR   119

/*debugs marco*/
#ifdef CONFIG_IMAP_CAMIF_DEBUG
#define CAMIF_DEBUG(debug, ...)         \
		printk(KERN_DEBUG "%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)
#define CAMIF_ALERT(alert, ...)         \
		printk(KERN_ALERT "%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)
#else
#define CAMIF_DEBUG(debug, ...)	do{}while(0)
#define CAMIF_ALERT(alert, ...)	do{}while(0)
#endif 


#define CAMIF_ERROR(error, ...)         \
		printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#define camif_debug(debug, ...)         CAMIF_DEBUG(debug, ##__VA_ARGS__)
#define camif_alert(alert, ...)         CAMIF_ALERT(alert, ##__VA_ARGS__)
#define camif_error(error, ...)         CAMIF_ERROR(error, ##__VA_ARGS__)


#define PR_WAIT_OK 	 		0
#define	CO_WAIT_OK  			1
#define SENSOR_SET_MODE			3	
#define	SENSOR_SET_WHITE_BALANCE        4
#define	SENSOR_SET_COLOR_EFECT		5
#define SENSOR_SET_ANTIBANDING          6
#define	SENSOR_SET_BRIGHTNESS		7
#define SENSOR_SET_NIGHT_MODE		8
#define	GET_FLAG_PHY			9
#define	SET_FLAG_DIRTY			10


#define	INIT_SENSOR 			1
#define SWITCH_SENSOR_TO_HIGH_SVGA   	2
#define SWITCH_SENSOR_TO_LOW_SVGA 	3	 
#define	SWITCH_SENSOR_TO_HIGH_XUGA   	4
#define SWITCH_SENSOR_TO_UPMID_XUGA     5
#define	SWITCH_SENSOR_TO_MID_XUGA   	6
#define	SWITCH_SENSOR_TO_LOW_XUGA   	7
#define	SENSOR_TO_HIGH_PREVIEW		8
#define	SENSOR_TO_LOW_PREVIEW		9
#define	CLOSE_SENSOR			10


#define SENSOR_WB_AUTO 			1
#define SENSOR_WB_CUSTOM		2
#define SENSOR_WB_INCANDESCENT		3
#define	SENSOR_WB_FLUORESCENT		4
#define	SENSOR_WB_DAYLIGHT		5
#define	SENSOR_WB_WARM_FLUORECENT	6
#define	SENSOR_WB_CLOUDY	 	7
#define	SENSOR_WB_TWILIGHT		8
#define	SENSOR_WB_SHADE			9

#define	SENSOR_EFFECT_OFF  		1
#define SENSOR_EFFECT_MONO		2
#define	SENSOR_EFFECT_NEGATIVE 		3
#define	SENSOR_EFFECT_SOLARIZE		4
#define	SENSOR_EFFECT_PASTEL		5
#define	SENSOR_EFFECT_MOSAIC		6
#define	SENSOR_EFFECT_RESIZE		7
#define	SENSOR_EFFECT_SEPIA		8
#define	SENSOR_EFFECT_POSTERIZE		9
#define	SENSOR_EFFECT_WHITEBOARD 	10
#define	SENSOR_EFFECT_BLACKBOARD	11
#define	SNESOR_EFFECT_AQUA		12

#define	SENSOR_BRIGHTNESS_0		1
#define	SENSOR_BRIGHTNESS_1		2
#define	SENSOR_BRIGHTNESS_2		3
#define	SENSOR_BRIGHTNESS_3		4
#define	SENSOR_BRIGHTNESS_4		5
#define	SENSOR_BRIGHTNESS_5		6
#define	SENSOR_BRIGHTNESS_6		7

#define SENSOR_SCENCE_AUTO		1
#define SENSOR_SCENCE_ACTION		2
#define	SENSOR_SCENCE_PORTRAIT		3
#define	SENSOR_SCENCE_LANDSCAPE		4
#define	SENSOR_SCENCE_NIGHT		5



struct imapx200_camif_param_t {
	dev_t			dev_id;
	struct device 		*dev;
	unsigned int 		irq;
	struct clk		*hclk;
	struct resource		*res;
	unsigned int		phy_start;
	unsigned int 		phy_size;
	void __iomem		*ioaddr;
};

enum sensor_type {
	SENSOR_OV2655 = 0,
	SENSOR_OV9650,
	SENSOR_OV7670,
};


// irq status
#define	OVFICH4_PR		( 1 << 31)
#define	OVFICH3_PR		( 1 << 30)
#define	OVFICH2_CO		( 1 << 29)
#define	OVFICH1_CO		( 1 << 28)
#define	OVFICH0_CO		( 1 << 27)
#define UNFICH4_PR		( 1 << 26)
#define UNFICH3_PR		( 1 << 25)
#define UNFICH2_CO		( 1 << 24)
#define UNFICH1_CO		( 1 << 23)
#define UNFICH0_CO		( 1 << 22)
#define		CAMIF_OVERFLOW		(OVFICH4_PR | OVFICH3_PR | \
						OVFICH2_CO | OVFICH1_CO | \
						OVFICH0_CO)
#define		CAMIF_UNDERFLOW		(UNFICH4_PR | UNFICH3_PR | \
						UNFICH2_CO | UNFICH1_CO | \
						UNFICH0_CO)

#define	PRFIFO_DIRTY		( 1 << 21)
#define	COFIFO_DIRTY		( 1 << 20)
#define	CERR656			( 1 << 19)
#define PR_LEISURE		( 1 << 18)	
#define CO_LEISURE		( 1 << 17)	

#define	DMACH4ONCE		( 1 << 16)
#define	DMACH3ONCE		( 1 << 15)
#define	DMACH2ONCE		( 1 << 14)
#define	DMACH1ONCE		( 1 << 13)
#define	DMACH0ONCE		( 1 << 12)

#define	DMACH4TWICE		( 1 << 11)
#define	DMACH3TWICE		( 1 << 10)
#define	DMACH2TWICE		( 1 << 9)
#define	DMACH1TWICE		( 1 << 8)
#define	DMACH0TWICE		( 1 << 7)

#define	SMART_PR		(1 << 6)
#define	SMART_CO		(1 << 5)
#define	FRAME_PR		(1 << 4)
#define	FRAME_CO		(1 << 3)

#define	PR_DMA_SUCCESS		(1 << 2)
#define	CO_DMA_SUCCESS		(1 << 1)
#define DMA_BEGINPOP		(1 << 0)	




