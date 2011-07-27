

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
#include "ima_cam.h"

#include "bf3603.h"

//-----------------------------
//      SENSOR_SET_MODE
//-----------------------------

//INIT_SENSOR
static const struct sensor_cmd set_mode_init_reg_list[]=
  {
        cmos_sensor(0x12,0x80),
        cmos_sensor(0xff,0xff),
        cmos_sensor(0x09,0x00),
        cmos_sensor(0x11,0x80),
        cmos_sensor(0x13,0x00),
        cmos_sensor(0x01,0x15),
        cmos_sensor(0x02,0x22),
        cmos_sensor(0x87,0x18),
        cmos_sensor(0x8c,0x02),//01 分频  02 不分频  
        cmos_sensor(0x8d,0x64),//7C 分频  64 不分频
        cmos_sensor(0x13,0x07),
        // DBLK manual  
        cmos_sensor(0x28,0x03),
        cmos_sensor(0x2c,0x03),  
        cmos_sensor(0x00,0x20),
        cmos_sensor(0x0d,0x20),
        cmos_sensor(0x0e,0x20),
        cmos_sensor(0x0f,0x20),
        cmos_sensor(0x05,0x16),  
        cmos_sensor(0x14,0x16),
        cmos_sensor(0x06,0x19),  
        cmos_sensor(0x08,0x19),
        cmos_sensor(0x26,0x08),  
        cmos_sensor(0x27,0x08),
        cmos_sensor(0X1F,0xa0),
        cmos_sensor(0X22,0xa0),//for  DBLK manual  
        //DBLK auto  
        cmos_sensor(0x28,0x00),
        cmos_sensor(0x2c,0x00),
	    cmos_sensor(0x1e,0x20), ///*0x30*/ 
        cmos_sensor(0x15,0x02),///*0x40*/
        cmos_sensor(0x3A,0x00),///*0x03*/
        cmos_sensor(0x2f,0x00),
        cmos_sensor(0x16,0x20),
        cmos_sensor(0x29,0x04),
        cmos_sensor(0x56,0x40),	
#if (defined(CONFIG_BOARD_E3)||defined(CONFIG_BOARD_E4)||defined(CONFIG_BOARD_E5))		
      	cmos_sensor(0x3e,0x00),//20110323
#endif
        /* 
        cmos_sensor(0x21,0x00), //  reset_check control 0x00 is open  modified by yang 2009/10/08
        cmos_sensor(0x04,0xbb),
        */

        //lens shading
        cmos_sensor(0x35,0x60),
        cmos_sensor(0x65,0x58),
        cmos_sensor(0x66,0x58),
        //global gain
        cmos_sensor(0x82,0x16),
        cmos_sensor(0x83,0x25),
        cmos_sensor(0x84,0x1a),
        cmos_sensor(0x85,0x26),
        cmos_sensor(0x86,0x30),   //white max 0x4f

        cmos_sensor(0x96,0x26),// AE speed
        cmos_sensor(0x97,0x0c),
        cmos_sensor(0x2b,0x00),//06 分频  00 不分频
        cmos_sensor(0x70,0x6f),
        cmos_sensor(0x72,0x4f),
        cmos_sensor(0x73,0x2f),
        cmos_sensor(0x74,0x27),
        cmos_sensor(0x75,0x0e),
        cmos_sensor(0x69,0x00),
        cmos_sensor(0x76,0xff),
        cmos_sensor(0x80,0x55),
        cmos_sensor(0x89,0x02),//02 分频  05 不分频
        cmos_sensor(0x8a,0xf8),//f8 分频  fc 不分频
        //black level
        cmos_sensor(0x90,0x20),
        cmos_sensor(0x91,0x1c),

        cmos_sensor(0X1F,0x30),//20
        cmos_sensor(0X22,0x30),
        cmos_sensor(0x39,0x80),  
        cmos_sensor(0x3f,0xa0),

        cmos_sensor(0x3b,0x60),
        cmos_sensor(0x3c,0x10),


        //gamma 低噪		
        cmos_sensor(0X40,0X32),
        cmos_sensor(0X41,0X2c),
        cmos_sensor(0X42,0X30),
        cmos_sensor(0X43,0X1d),
        cmos_sensor(0X44,0X1a),
        cmos_sensor(0X45,0X14),
        cmos_sensor(0X46,0X11),
        cmos_sensor(0X47,0X0e),
        cmos_sensor(0X48,0X0d),
        cmos_sensor(0X49,0X0c),
        cmos_sensor(0X4b,0X0B),
        cmos_sensor(0X4c,0X09),
        cmos_sensor(0X4e,0X09),
        cmos_sensor(0X4f,0X08),
        cmos_sensor(0X50,0X07),

        /*//gamma 1 
        cmos_sensor(0X40,0X20),
        cmos_sensor(0X41,0X30),
        cmos_sensor(0X42,0X28),
        cmos_sensor(0X43,0X28),
        cmos_sensor(0X44,0X1d),
        cmos_sensor(0X45,0X15),
        cmos_sensor(0X46,0X13),
        cmos_sensor(0X47,0X10),
        cmos_sensor(0X48,0X0E),
        cmos_sensor(0X49,0X0B),
        cmos_sensor(0X4b,0X0B),
        cmos_sensor(0X4c,0X09),
        cmos_sensor(0X4e,0X07),
        cmos_sensor(0X4f,0X06),
        cmos_sensor(0X50,0X05),

        //gamma 2 清晰			
        cmos_sensor(0X40,0X25),
        cmos_sensor(0X41,0X2a),
        cmos_sensor(0X42,0X28),
        cmos_sensor(0X43,0X28),
        cmos_sensor(0X44,0X20),
        cmos_sensor(0X45,0X1d),
        cmos_sensor(0X46,0X17),
        cmos_sensor(0X47,0X15),
        cmos_sensor(0X48,0X0f),
        cmos_sensor(0X49,0X0e),
        cmos_sensor(0X4b,0X0a),
        cmos_sensor(0X4c,0X06),
        cmos_sensor(0X4e,0X05),
        cmos_sensor(0X4f,0X04),
        cmos_sensor(0X50,0X02),

        //gamma 3 亮丽			
        cmos_sensor(0X40,0X42),
        cmos_sensor(0X41,0X3b),
        cmos_sensor(0X42,0X32),
        cmos_sensor(0X43,0X24),
        cmos_sensor(0X44,0X1c),
        cmos_sensor(0X45,0X15),
        cmos_sensor(0X46,0X11),
        cmos_sensor(0X47,0X0d),
        cmos_sensor(0X48,0X0d),
        cmos_sensor(0X49,0X0B),
        cmos_sensor(0X4b,0X09),
        cmos_sensor(0X4c,0X08),
        cmos_sensor(0X4e,0X08),
        cmos_sensor(0X4f,0X07),
        cmos_sensor(0X50,0X06),
        */


        //color matrix
        cmos_sensor(0x51,0x30),
        cmos_sensor(0x52,0x92),
        cmos_sensor(0x53,0x02),
        cmos_sensor(0x54,0x86),
        cmos_sensor(0x57,0x30),
        cmos_sensor(0x58,0x8a),
        cmos_sensor(0x59,0x80),
        cmos_sensor(0x5a,0x92),
        cmos_sensor(0x5b,0x32),

        /* the other one
        cmos_sensor(0x51,0x2e),
        cmos_sensor(0x52,0x8e),
        cmos_sensor(0x53,0x80),
        cmos_sensor(0x54,0x88),
        cmos_sensor(0x57,0x30),
        cmos_sensor(0x58,0x88),
        cmos_sensor(0x59,0x80),
        cmos_sensor(0x5a,0x92),
        cmos_sensor(0x5b,0x32),
        */
        cmos_sensor(0x5c,0x2e),
        cmos_sensor(0x5d,0x17),
        // old  AWB
        cmos_sensor(0x6a,0x01),
        cmos_sensor(0x23,0x66),
        cmos_sensor(0xa0,0x9f),
        cmos_sensor(0xa1,0x21),
        cmos_sensor(0xa2,0x11),
        cmos_sensor(0xa3,0x29),
        cmos_sensor(0xa4,0x0e),
        cmos_sensor(0xa5,0x26),
        cmos_sensor(0xa6,0x04),
        cmos_sensor(0xa7,0x80),
        cmos_sensor(0xa8,0x80),
        cmos_sensor(0xa9,0x26),
        cmos_sensor(0xaa,0x26),
        cmos_sensor(0xab,0x2a),
        cmos_sensor(0xac,0x3c),
        cmos_sensor(0xad,0xf0),
        cmos_sensor(0xae,0xff),
        /* new awb
        cmos_sensor(0x6a,0x01),
        cmos_sensor(0x23,0x66),
        cmos_sensor(0xa0,0x9f),
        cmos_sensor(0xa1,0x51),
        cmos_sensor(0xa2,0x10),
        cmos_sensor(0xa3,0x26),
        cmos_sensor(0xa4,0x0b),
        cmos_sensor(0xa5,0x26),
        cmos_sensor(0xa6,0x06),
        cmos_sensor(0xa7,0x98),
        cmos_sensor(0xa8,0x80),
        cmos_sensor(0xa9,0x42),
        cmos_sensor(0xaa,0x4b),
        cmos_sensor(0xab,0x3e),
        cmos_sensor(0xac,0x42),
        cmos_sensor(0xad,0x43),
        cmos_sensor(0xae,0x48),
        cmos_sensor(0xaf,0x2b),
        cmos_sensor(0xc5,0x32),
        cmos_sensor(0xc6,0x34),
        cmos_sensor(0xc7,0x39),
        cmos_sensor(0xc8,0x2f),
        cmos_sensor(0xc9,0x36),
        cmos_sensor(0xca,0x3f),
        cmos_sensor(0xcb,0x41),
        cmos_sensor(0xcc,0x42),
        cmos_sensor(0xcd,0x48),
        cmos_sensor(0xce,0x44),
        cmos_sensor(0xcf,0x4C),
        cmos_sensor(0xd0,0x4B),
        cmos_sensor(0xd1,0x55),
        */
        // color saturation
        cmos_sensor(0xb0,0xe4),
        cmos_sensor(0xb1,0xc0),
        cmos_sensor(0xb2,0xb0),
        cmos_sensor(0xb3,0x86),
        //AE target
        cmos_sensor(0x24,0x90),   //white
        cmos_sensor(0x25,0x80),
        cmos_sensor(0x94,0x20),

        cmos_sensor(0x2a,0x00),
        cmos_sensor(0x2b,0xd8), 
        
        //anti webcamera banding
        cmos_sensor(0x80,0x55),/* 50 Hz */     
        cmos_sensor(0x9d,0x78), //99
        
       //  cmos_sensor(0x80,0x54),/* 60 Hz */     
       //  cmos_sensor(0x9e,0x7f), 

        cmos_sensor(0x8e,0x02),// MIN FPS is 7.5  0x3fc   MCLK/(784*fps)  mclk=12M
        cmos_sensor(0x8f,0x28), 

        cmos_sensor(MAG_REG,MAG_VAL),
        cmos_sensor(MAG_REG,MAG_VAL),
  };

//SET_MODE_SWITCH_SENSOR_TO_MID_XUGA
static const struct sensor_cmd set_mode_switch_mid_xuga_reg_list[]=
{
    //640*480   
    cmos_sensor(0x17,0x00),
    cmos_sensor(0x18,0xa0),
    cmos_sensor(0x19,0x00),
    cmos_sensor(0x1a,0x78),
    cmos_sensor(0x03,0x00),
    cmos_sensor(0x12,0x00),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA
static const struct sensor_cmd set_mode_switch_low_xuga_reg_list[]=
{
    //320*240   
    cmos_sensor(0x17,0x00),
    cmos_sensor(0x18,0xa0),
    cmos_sensor(0x19,0x00),
    cmos_sensor(0x1a,0x78),
    cmos_sensor(0x03,0x00),
    cmos_sensor(0x12,0x10),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};


//SET_MODE_SENSOR_TO_HIGH_PREVIEW
static const struct sensor_cmd set_mode_high_prev_reg_list[]=
{
    //640*480   
    cmos_sensor(0x17,0),
    cmos_sensor(0x18,0xa0),
    cmos_sensor(0x19,0),
    cmos_sensor(0x1a,0x78),
    cmos_sensor(0x03,0),
    cmos_sensor(0x12,0),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SENSOR_TO_LOW_PREVIEW
static const struct sensor_cmd set_mode_low_prev_reg_list[]=
{
    //320*240   
    cmos_sensor(0x17,0),
    cmos_sensor(0x18,0xa0),
    cmos_sensor(0x19,0),
    cmos_sensor(0x1a,0x78),
    cmos_sensor(0x03,0),
    cmos_sensor(0x12,0x10),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_MODE_CLOSE_SENSOR
static const struct sensor_cmd set_mode_close_reg_list[]=
{
    cmos_sensor(0x09,0x10),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd (* sensor_set_mode_list[SET_MODE_NUM])[]={
//const struct sensor_cmd * sensor_set_mode_list[SET_MODE_NUM]={
    NULL,
    &set_mode_init_reg_list,
    NULL,
    NULL,
    NULL,
    NULL,
    &set_mode_switch_mid_xuga_reg_list,
    &set_mode_switch_low_xuga_reg_list,
    &set_mode_high_prev_reg_list,
    &set_mode_low_prev_reg_list,
    &set_mode_close_reg_list,
};



//-----------------------------------------
//        SENSOR_SET_WHITE_BALANCE
//-----------------------------------------

//SET_WB_AUTO
static const struct sensor_cmd set_wb_auto_reg_list[]=
{
    cmos_sensor(0x01,0x15),
    cmos_sensor(0x02,0x22),
    cmos_sensor(0x13,0x07),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_WB_INCANDESCENT
static const struct sensor_cmd set_wb_incandescent_reg_list[]=
{
    cmos_sensor(0x13,0x05),
    cmos_sensor(0x01,0x2d),
    cmos_sensor(0x02,0x0c),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_FLUORESCENT
static const struct sensor_cmd set_wb_fluorescent_reg_list[]=
{
    cmos_sensor(0x13,0x05),
    cmos_sensor(0x01,0x22),
    cmos_sensor(0x02,0x1a),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_DAYLIGHT
static const struct sensor_cmd set_wb_daylight_reg_list[]=
{
    cmos_sensor(0x13,0x05),
    cmos_sensor(0x01,0x19),
    cmos_sensor(0x02,0x27),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_WARM_FLUORECENT

//SET_WB_CLOUDY
static const struct sensor_cmd set_wb_cloudy_reg_list[]=
{
    cmos_sensor(0x13,0x05),
    cmos_sensor(0x01,0x16),
    cmos_sensor(0x02,0x24),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_TWILIGHT
//SET_WB_SHADE
static const struct sensor_cmd set_wb_shade_reg_list[]=
{
    cmos_sensor(0x13,0x05),
    cmos_sensor(0x01,0x28),
    cmos_sensor(0x02,0xf),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd (* sensor_set_wb_list[SET_WB_NUM])[]={
    NULL,
    &set_wb_auto_reg_list,
    NULL,
    &set_wb_incandescent_reg_list,
    &set_wb_fluorescent_reg_list,
    &set_wb_daylight_reg_list,
    NULL,
    &set_wb_cloudy_reg_list,
    NULL,
    &set_wb_shade_reg_list,
};

//-----------------------------------------
//       SENSOR_SET_COLOR_EFECT
//-----------------------------------------
//SET_EFFECT_OFF
static const struct sensor_cmd set_color_effect_off_reg_list[]=
{
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x00),       
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_MONO
static const struct sensor_cmd set_color_effect_mono_reg_list[]=
{
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x20),       
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x45),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_NEGATIVE
static const struct sensor_cmd set_color_effect_negative_reg_list[]=
{
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x40),       
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_SOLARIZE
static const struct sensor_cmd set_color_effect_solarize_reg_list[]=
{
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x00),       
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x80),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_PASTEL
static const struct sensor_cmd set_color_effect_pastel_reg_list[]=
{
    /*
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x00),       
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x80),*/
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_MOSAIC
//SET_EFFECT_RESIZE

//SET_EFFECT_SEPIA
static const struct sensor_cmd set_color_effect_sepia_reg_list[]=
{
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x20),       
    cmos_sensor(0x67,0x98),       
    cmos_sensor(0x68,0x66),
    cmos_sensor(0x56,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_POSTERIZE
//SET_EFFECT_WHITEBOARD
static const struct sensor_cmd set_color_effect_whiteboard_reg_list[]=
{
    cmos_sensor(0x69,0x00),
    cmos_sensor(0x80,0xc5),       
    cmos_sensor(0x77,0xf0),
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_BLACKBOARD
static const struct sensor_cmd set_color_effect_blackboard_reg_list[]=
{
    cmos_sensor(0x69,0x00),
    cmos_sensor(0x80,0xc5),       
    cmos_sensor(0x77,0xe0),
    cmos_sensor(0x67,0x80),       
    cmos_sensor(0x68,0x80),
    cmos_sensor(0x56,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_AQUA
static const struct sensor_cmd set_color_effect_aqua_reg_list[]=
{
    cmos_sensor(0x80,0x45),       
    cmos_sensor(0x77,0x00),
    cmos_sensor(0x69,0x20),       
    cmos_sensor(0x67,0x70),       
    cmos_sensor(0x68,0x70),
    cmos_sensor(0x56,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};


static const struct sensor_cmd (* sensor_set_color_effect_list[SET_EFFECT_NUM])[]={
    NULL,
    &set_color_effect_off_reg_list,
    &set_color_effect_mono_reg_list,
    &set_color_effect_negative_reg_list,
    &set_color_effect_solarize_reg_list,
    NULL,
    NULL,
    NULL,
    &set_color_effect_sepia_reg_list,
    NULL,
    &set_color_effect_whiteboard_reg_list,
    &set_color_effect_blackboard_reg_list,
    &set_color_effect_aqua_reg_list
};
//-----------------------------------------
//      SENSOR_SET_ANTIBANDING
//-----------------------------------------

static const struct sensor_cmd set_antibanding_50hz_reg_list[]=
{
    cmos_sensor(0x80,0x55),/* 50 Hz */     
    cmos_sensor(0x9d,0x78), //99
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd set_antibanding_60hz_reg_list[]=
{
    cmos_sensor(0x80,0x54),/* 60 Hz */     
    cmos_sensor(0x9e,0x64), //7f
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd (* sensor_set_antibanding_list[SET_ANTIBANDING_NUM])[]={
//const struct sensor_cmd * sensor_set_antibanding_list[SET_ANTIBANDING_NUM]={
    NULL,
};
//-----------------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------------
//SET_BRIGHTNESS_0

static const struct sensor_cmd (* sensor_set_brightness_list[SENSOR_SET_BRIGHTNESS_NUM])[]={
//const struct sensor_cmd * sensor_set_brightness_list[SENSOR_SET_BRIGHTNESS_NUM]={
    NULL,
};

//-----------------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------------

//SET_SCENCE_DEFAULT
static const struct sensor_cmd set_scence_auto_reg_list[]=
{
    cmos_sensor(0x8e,0x02),
    cmos_sensor(0x8f,0x28), 
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_SCENCE_NIGHT
static const struct sensor_cmd set_scence_night_reg_list[]=
{
    cmos_sensor(0X85,0X2a),
    cmos_sensor(0X86,0X36),
    cmos_sensor(0x8e,0x04),
    cmos_sensor(0x8f,0x0c),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd (* sensor_set_scence_list[SET_SCENCE_NUM])[]={
    NULL,
    &set_scence_auto_reg_list,
    NULL,
    NULL,
    NULL,
    &set_scence_night_reg_list,
};

static const struct sensor_op_list sensor_op_table[SENSOR_OP_NUM]={
    {0,NULL},
    {0,NULL},
    {0,NULL},
    {SET_MODE_NUM,(void **)sensor_set_mode_list},
    {SET_WB_NUM,(void **)sensor_set_wb_list},
    {SET_EFFECT_NUM,(void **)sensor_set_color_effect_list},
    {SET_ANTIBANDING_NUM,(void **)sensor_set_antibanding_list},
    {SENSOR_SET_BRIGHTNESS_NUM,(void **)sensor_set_brightness_list},
    {SET_SCENCE_NUM,(void **)sensor_set_scence_list},
    {0,NULL},
    {0,NULL},
};

#define GET_OP_TABLE(x) (&sensor_op_table[(x)])

const struct sensor_op_list * bf3603_get_op_table(int op)
{
    if(op<SENSOR_OP_NUM)
        return GET_OP_TABLE(op);
        
    return NULL;
}

extern struct cam_interface bf3603_interface;

struct cam_interface *bf3603_detect(int *invvsync)
{
    unsigned char id;

    if(cam_reg_read(BF3603_I2C_ADDR,0x04,&id,1)==0){
        printk("check camara(bf3603 id 0x99) 0x%x\n",id);
        if(id==0x99){
            printk("Found Camara BF3603 ID[0x%x]\n",id);
            if(invvsync)
                *invvsync=1;
            return &bf3603_interface;
        }
    }
   
    return NULL;
}



//----------------------------
//      SENSOR_SET_MODE
//-----------------------------
int  bf3603_init(void)
{
    int ret;

    cam_power_set(CAMARA_POWER_ON);

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_INIT_SENSOR);
    
    return ret;
}

int  bf3603_switch_low_svga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA);
    return ret;
}
int  bf3603_switch_high_svga(void){
    int ret;

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA);
    return ret;
}
int  bf3603_switch_high_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA);
    return ret;
}
int  bf3603_switch_upmid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA);
    return ret;
}
int  bf3603_switch_mid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_MID_XUGA);
    return ret;
}
int  bf3603_switch_low_xuga(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA);
    return ret;
}
int  bf3603_to_high_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_HIGH_PREVIEW);
    return ret;
}
int  bf3603_to_low_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_LOW_PREVIEW);
    return ret;
}

//int  cam_switch_low_xuga(void){return -1;}
int  bf3603_close(void)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_CLOSE_SENSOR);
    
    cam_power_set(CAMARA_POWER_OFF);

    return ret;
}
//-----------------------------------
//      SENSOR_SET_WHITE_BALANCE
//-----------------------------------
int  bf3603_set_wb(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_WHITE_BALANCE,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_COLOR_EFFECT
//-----------------------------------
int  bf3603_set_effect(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_COLOR_EFFECT,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------
int  bf3603_set_brightness(unsigned char value)
{
    struct sensor_cmd cmd;
    int ret;
    
    cmd.reg = 0x55;
    cmd.val = value;
    ret=cam_reg_write(BF3603_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));
    
    return ret;
}

//-----------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------
int  bf3603_night_mode_on(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_NIGHT);
    return ret;
}                                                    
int  bf3603_night_mode_off(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_AUTO);
    return ret;
}

int  bf3603_dump_reg(void)
{
    unsigned char reg;
    unsigned char val;
    int ret;
    
    for(reg=0;reg<0xff;reg++){

        ret = cam_reg_read(BF3603_I2C_ADDR,reg,&val,1);
        if(ret != 0){
            printk("reg 0x%x  failed\n",reg);
        }else{
            printk("0x%x 0x%x\n",reg,val);
        }
    }

    return 0;
}

struct device_info bf3603_info={
    BF3603_I2C_ADDR,
};


struct cam_interface bf3603_interface={
    &bf3603_info,
    bf3603_get_op_table,
    bf3603_init,
    bf3603_switch_low_svga,
    bf3603_switch_high_svga,
    bf3603_switch_high_xuga,
    bf3603_switch_upmid_xuga,
    bf3603_switch_mid_xuga,
    bf3603_switch_low_xuga,
    bf3603_to_high_preview,
    bf3603_to_low_preview,
    bf3603_close,
    bf3603_set_wb,
    bf3603_set_effect,
    bf3603_set_brightness,
    bf3603_night_mode_on,                                                    
    bf3603_night_mode_off,
    bf3603_dump_reg,
};

