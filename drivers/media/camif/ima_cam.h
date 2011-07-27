/***************************************************************************** 
 ** ima_cam.h 
 ** 
 ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 ** 
 ** This program is free software; you can redistribute it and/or modify
 ** it under the terms of the GNU General Public License as published by
 ** the Free Software Foundation; either version 2 of the License, or
 ** (at your option) any later version.
 ** 
 ** Description: main file of ima media encode driver
 **
 ** Author:
 **     neville <haixu_fu@infotm.com>
 **      
 ** Revision History: 
 ** ­­­­­­­­­­­­­­­­­ 
 ** 1.1  06/24/2010 neville 
 *******************************************************************************/

#ifndef  __IMA_CAM_H__
#define	__IMA_CAM_H__
#endif


#define	CAMIF_RET_OK	0
#define CAMIF_DEFAULT_MAJOR   119
#define	CAMIF_DEFAULT_MINOR   119

/*debugs marco*/
#define CONFIG_IMAP_CAMIF_DEBUG
#ifdef CONFIG_IMAP_CAMIF_DEBUG
#undef pr_debug
#define pr_debug printk
#define CAMIF_DEBUG(debug, ...)         \
		printk(/*KERN_DEBUG */"%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)
#define CAMIF_ALERT(alert, ...)         \
		printk(/*KERN_ALERT */"%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)
#else
#define CAMIF_DEBUG(debug, ...)	do{}while(0)
#define CAMIF_ALERT(alert, ...)	do{}while(0)
#endif 


#define CAMIF_ERROR(error, ...)         \
		printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#define camif_debug(debug, ...)         CAMIF_DEBUG(debug, ##__VA_ARGS__)
#define camif_alert(alert, ...)         CAMIF_ALERT(alert, ##__VA_ARGS__)
#define camif_error(error, ...)         CAMIF_ERROR(error, ##__VA_ARGS__)

/*
#define PR_WAIT_OK 	 		0
#define	CO_WAIT_OK  			1
#define SENSOR_SET_MODE			3	
#define	SENSOR_SET_WHITE_BALANCE        4
#define	SENSOR_SET_COLOR_EFFECT		5
#define SENSOR_SET_ANTIBANDING          6
#define	SENSOR_SET_BRIGHTNESS		7
#define SENSOR_SET_NIGHT_MODE		8*/

#define CAMARA_POWER_OFF 1
#define CAMARA_POWER_ON 0


enum{
    SENSOR_PR_WAIT_OK = 0,
    SENSOR_CO_WAIT_OK,
    SENSOR_RESVERED,
    SENSOR_SET_MODE,
    SENSOR_SET_WHITE_BALANCE,
    SENSOR_SET_COLOR_EFFECT,
    SENSOR_SET_ANTIBANDING,
    SENSOR_SET_BRIGHTNESS,
    SENSOR_SET_SCENCE_MODE,
    SENSOR_GET_FLAG_PHY =9,
    SENSOR_SET_FLAG_DIRTY,
    SENSOR_OP_NUM,
};


//SENSOR_SET_MODE
enum{
    SET_MODE_RESERVED = 0,
    SET_MODE_INIT_SENSOR,
    SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA,
    SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA,
    SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA,
    SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA,
    SET_MODE_SWITCH_SENSOR_TO_MID_XUGA,
    SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA,
    SET_MODE_SENSOR_TO_HIGH_PREVIEW,
    SET_MODE_SENSOR_TO_LOW_PREVIEW,
    SET_MODE_CLOSE_SENSOR,
    SET_MODE_NUM,
};

//SENSOR_SET_WHITE_BALANCE
enum{
    SET_WB_RESERVED =0,
    SET_WB_AUTO,
    SET_WB_CUSTOM,
    SET_WB_INCANDESCENT,
    SET_WB_FLUORESCENT,
    SET_WB_DAYLIGHT,
    SET_WB_WARM_FLUORECENT,
    SET_WB_CLOUDY,
    SET_WB_TWILIGHT,
    SET_WB_SHADE,
    SET_WB_NUM,
};

//SENSOR_SET_COLOR_EFECT
enum{
    SET_EFFECT_RESERVED = 0,
    SET_EFFECT_OFF,
    SET_EFFECT_MONO,
    SET_EFFECT_NEGATIVE,
    SET_EFFECT_SOLARIZE,
    SET_EFFECT_PASTEL,
    SET_EFFECT_MOSAIC,
    SET_EFFECT_RESIZE,
    SET_EFFECT_SEPIA,
    SET_EFFECT_POSTERIZE,
    SET_EFFECT_WHITEBOARD,
    SET_EFFECT_BLACKBOARD,
    SET_EFFECT_AQUA,
    SET_EFFECT_NUM,
};

//SENSOR_SET_ANTIBANDING
enum{
    SET_ANTIBANDING_RESVERED = 0,
    SET_ANTIBANDING_NUM,
};

//SENSOR_SET_BRIGHTNESS
enum{
    SET_BRIGHTNESS_RESVERED = 0,
    SET_BRIGHTNESS_0,
    SET_BRIGHTNESS_1,
    SET_BRIGHTNESS_2,
    SET_BRIGHTNESS_3,
    SET_BRIGHTNESS_4,
    SET_BRIGHTNESS_DEFAULT =SET_BRIGHTNESS_4,
    SET_BRIGHTNESS_5,
    SET_BRIGHTNESS_6,
    SENSOR_SET_BRIGHTNESS_NUM,
};


//SENSOR_SET_NIGHT_MODE
enum{
    SET_SCENCE_RESVERED = 0,
    SET_SCENCE_AUTO,
    SET_SCENCE_ACTION,
    SET_SCENCE_PORTRAIT,
    SET_SCENCE_LANDSCAPE,
    SET_SCENCE_NIGHT,
    SET_SCENCE_NUM,
};

//SENSOR_OP_MODE
struct sensor_op_list{
    int num;
    //const struct sensor_cmd (* cmd_table)[];
    void **cmd_table;
};

#define MAG_REG 0xfe
#define MAG_VAL 0x55


struct device_info{    
    int dev_addr;
};

struct cam_interface{
    const struct device_info *info;
    const struct sensor_op_list *(* dev_get_op_table)(int op);
    int  (*dev_init)(void);
    int  (*dev_switch_low_svga)(void);
    int  (*dev_switch_high_svga)(void);
    int  (*dev_switch_high_xuga)(void);
    int  (*dev_switch_upmid_xuga)(void);
    int  (*dev_switch_mid_xuga)(void);
    int  (*dev_switch_low_xuga)(void);
    int  (*dev_to_high_preview)(void);
    int  (*dev_to_low_preview)(void);
    int  (*dev_close)(void);
    int  (*dev_set_wb)(int cmd_code);
    int  (*dev_set_effect)(int cmd_code);
    int  (*dev_set_brightness)(unsigned char value);
    int  (*dev_night_mode_on)(void);                                               
    int  (*dev_night_mode_off)(void);
    int  (*dev_dump_reg)(void);
};


struct ima_camif_param_t {
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
	SENSOR_BF3603,
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

struct sensor_cmd{
    unsigned char reg;
    unsigned char val;
};

#define cmos_sensor(reg,val) {(reg),(val)}

//----------------------------
//      SENSOR_SET_MODE
//-----------------------------
int  cam_switch_low_svga(void);
int  cam_switch_high_svga(void);
int  cam_switch_high_xuga(void);
int  cam_switch_upmid_xuga(void);
int  cam_switch_mid_xuga(void);
int  cam_get_3008(void);
int  cam_to_high_preview(void);
int  cam_to_low_preview(void);
int  cam_switch_low_xuga(void);
int  cam_init(int invvsync);
int  cam_close(void);
//-----------------------------------
//      SENSOR_SET_WHITE_BALANCE
//-----------------------------------
int  cam_set_wb(int cmd_code);

//-----------------------------------
//      SENSOR_SET_COLOR_EFFECT
//-----------------------------------
int  cam_set_effect(int cmd_code);

//-----------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------
int  cam_set_brightness(unsigned char value);

//-----------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------
int  cam_night_mode_on(void);                                                    
int  cam_night_mode_off(void);
int  cam_dump_reg(void);



int cam_reg_write(unsigned char addr, unsigned char * data, int size);
int cam_reg_read(unsigned char addr,unsigned char reg, unsigned char *data, unsigned int size);
void cam_power_set(int mode);
int excute_cam_cmd(int op_code,int cmd_code);


