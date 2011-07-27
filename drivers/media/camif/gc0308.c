

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

#include "gc0308.h"


//-----------------------------
//      SENSOR_SET_MODE
//-----------------------------

//INIT_SENSOR
static const struct sensor_cmd set_mode_init_reg_list[]=
{
  	cmos_sensor(0xfe , 0x80),   	
		
	cmos_sensor(0xfe, 0x00),      // set page0

		
	cmos_sensor(0xd2 , 0x10),   // close AEC
	cmos_sensor(0x22 , 0x55),   // close AWB

	cmos_sensor(0x5a , 0x56), 
	cmos_sensor(0x5b , 0x40),
	cmos_sensor(0x5c , 0x4a),			

	cmos_sensor(0x22 , 0x57),  // Open AWB
				
	cmos_sensor(0x01 , 0xfa), 
	cmos_sensor(0x02 , 0x70), 
	cmos_sensor(0x0f , 0x01), 

	cmos_sensor(0x03 , 0x01), 
	cmos_sensor(0x04 , 0x2c), 

	cmos_sensor(0xe2 , 0x00), 	//anti-flicker step [11:8]
	cmos_sensor(0xe3 , 0x64),   //anti-flicker step [7:0]
		
	cmos_sensor(0xe4 , 0x02),   //exp level 1  16.67fps
	cmos_sensor(0xe5 , 0x58), 
	cmos_sensor(0xe6 , 0x03),   //exp level 2  12.5fps
	cmos_sensor(0xe7 , 0x20), 
	cmos_sensor(0xe8 , 0x04),   //exp level 3  8.33fps
	cmos_sensor(0xe9 , 0xb0), 
	cmos_sensor(0xea , 0x09),   //exp level 4  4.00fps
	cmos_sensor(0xeb , 0xc4), 

	cmos_sensor(0x05 , 0x00),
	cmos_sensor(0x06 , 0x00),
	cmos_sensor(0x07 , 0x00),
	cmos_sensor(0x08 , 0x00),
	cmos_sensor(0x09 , 0x01),
	cmos_sensor(0x0a , 0xe8),
	cmos_sensor(0x0b , 0x02),
	cmos_sensor(0x0c , 0x88),
	cmos_sensor(0x0d , 0x02),
	cmos_sensor(0x0e , 0x02),
	cmos_sensor(0x10 , 0x22),
	cmos_sensor(0x11 , 0xfd),
	cmos_sensor(0x12 , 0x2a),
	cmos_sensor(0x13 , 0x00),

	cmos_sensor(0x14 , 0x11), //  0x10:normal  , 0x11:IMAGE_H_MIRROR , 0x12:IMAGE_V_MIRROR,0x13:IMAGE_HV_MIRROR
 
	cmos_sensor(0x15 , 0x0a),
	cmos_sensor(0x16 , 0x05),
	cmos_sensor(0x17 , 0x01),
	cmos_sensor(0x18 , 0x44),
	cmos_sensor(0x19 , 0x44),
	cmos_sensor(0x1a , 0x1e),
	cmos_sensor(0x1b , 0x00),
	cmos_sensor(0x1c , 0xc1),
	cmos_sensor(0x1d , 0x08),
	cmos_sensor(0x1e , 0x60),
	cmos_sensor(0x1f , 0x16),

	
	cmos_sensor(0x20 , 0xff),
	cmos_sensor(0x21 , 0xf8),
	cmos_sensor(0x22 , 0x57),
	cmos_sensor(0x24 , /*0xa0*/0xa2),
	cmos_sensor(0x25 , 0x0f),
	                         
	//output sync_mode       
	cmos_sensor(0x26 , 0x03),
	cmos_sensor(0x2f , 0x01),
	cmos_sensor(0x30 , 0xf7),
	cmos_sensor(0x31 , 0x50),
	cmos_sensor(0x32 , 0x00),
	cmos_sensor(0x39 , 0x04),
	cmos_sensor(0x3a , 0x18),
	cmos_sensor(0x3b , 0x20),
	cmos_sensor(0x3c , 0x00),
	cmos_sensor(0x3d , 0x00),
	cmos_sensor(0x3e , 0x00),
	cmos_sensor(0x3f , 0x00),
	cmos_sensor(0x50 , 0x10),
	cmos_sensor(0x53 , 0x82),
	cmos_sensor(0x54 , 0x80),
	cmos_sensor(0x55 , 0x80),
	cmos_sensor(0x56 , 0x82),
	cmos_sensor(0x8b , 0x40),
	cmos_sensor(0x8c , 0x40),
	cmos_sensor(0x8d , 0x40),
	cmos_sensor(0x8e , 0x2e),
	cmos_sensor(0x8f , 0x2e),
	cmos_sensor(0x90 , 0x2e),
	cmos_sensor(0x91 , 0x3c),
	cmos_sensor(0x92 , 0x50),
	cmos_sensor(0x5d , 0x12),
	cmos_sensor(0x5e , 0x1a),
	cmos_sensor(0x5f , 0x24),
	cmos_sensor(0x60 , 0x07),
	cmos_sensor(0x61 , 0x15),
	cmos_sensor(0x62 , 0x08),
	cmos_sensor(0x64 , 0x03),
	cmos_sensor(0x66 , 0xe8),
	cmos_sensor(0x67 , 0x86),
	cmos_sensor(0x68 , 0xa2),
	cmos_sensor(0x69 , 0x18),
	cmos_sensor(0x6a , 0x0f),
	cmos_sensor(0x6b , 0x00),
	cmos_sensor(0x6c , 0x5f),
	cmos_sensor(0x6d , 0x8f),
	cmos_sensor(0x6e , 0x55),
	cmos_sensor(0x6f , 0x38),
	cmos_sensor(0x70 , 0x15),
	cmos_sensor(0x71 , 0x33),
	cmos_sensor(0x72 , 0xdc),
	cmos_sensor(0x73 , 0x80),
	cmos_sensor(0x74 , 0x02),
	cmos_sensor(0x75 , 0x3f),
	cmos_sensor(0x76 , 0x02),
	cmos_sensor(0x77 , 0x36),
	cmos_sensor(0x78 , 0x88),
	cmos_sensor(0x79 , 0x81),
	cmos_sensor(0x7a , 0x81),
	cmos_sensor(0x7b , 0x22),
	cmos_sensor(0x7c , 0xff),
	cmos_sensor(0x93 , 0x48),
	cmos_sensor(0x94 , 0x00),
	cmos_sensor(0x95 , 0x05),
	cmos_sensor(0x96 , 0xe8),
	cmos_sensor(0x97 , 0x40),
	cmos_sensor(0x98 , 0xf0),
	cmos_sensor(0xb1 , 0x38),
	cmos_sensor(0xb2 , 0x38),
	cmos_sensor(0xbd , 0x38),
	cmos_sensor(0xbe , 0x36),
	cmos_sensor(0xd0 , 0xc9),
	cmos_sensor(0xd1 , 0x10),
	//cmos_sensor(0xd2 , 0x90),
	cmos_sensor(0xd3 , /*0x80*/0x88),
	cmos_sensor(0xd5 , 0xf2),
	cmos_sensor(0xd6 , 0x16),
	cmos_sensor(0xdb , 0x92),
	cmos_sensor(0xdc , 0xa5),
	cmos_sensor(0xdf , 0x23),
	cmos_sensor(0xd9 , 0x00),
	cmos_sensor(0xda , 0x00),
	cmos_sensor(0xe0 , 0x09),
	cmos_sensor(0xec , 0x20),
	cmos_sensor(0xed , 0x04),
	cmos_sensor(0xee , 0xa0),
	cmos_sensor(0xef , 0x40),
	cmos_sensor(0x80 , 0x03),
	cmos_sensor(0x80 , 0x03),


	//case 3   gamma
	cmos_sensor( 0x9F, 0x10 ), 
	cmos_sensor( 0xA0, 0x20 ), 
	cmos_sensor( 0xA1, 0x38 ), 
	cmos_sensor( 0xA2, 0x4E ), 
	cmos_sensor( 0xA3, 0x63 ), 
	cmos_sensor( 0xA4, 0x76 ), 
	cmos_sensor( 0xA5, 0x87 ), 
	cmos_sensor( 0xA6, 0xA2 ), 
	cmos_sensor( 0xA7, 0xB8 ), 
	cmos_sensor( 0xA8, 0xCA ), 
	cmos_sensor( 0xA9, 0xD8 ), 
	cmos_sensor( 0xAA, 0xE3 ), 
	cmos_sensor( 0xAB, 0xEB ), 
	cmos_sensor( 0xAC, 0xF0 ), 
	cmos_sensor( 0xAD, 0xF8 ), 
	cmos_sensor( 0xAE, 0xFD ), 
	cmos_sensor( 0xAF, 0xFF ), 

	/*********************************************
	
		//case 1:                                          
			cmos_sensor( 0x9F, 0x0B ), // 	case 1  smallest gamma curve
			cmos_sensor( 0xA0, 0x16 ), 
			cmos_sensor( 0xA1, 0x29 ), 
			cmos_sensor( 0xA2, 0x3C ), 
			cmos_sensor( 0xA3, 0x4F ), 
			cmos_sensor( 0xA4, 0x5F ), 
			cmos_sensor( 0xA5, 0x6F ), 
			cmos_sensor( 0xA6, 0x8A ), 
			cmos_sensor( 0xA7, 0x9F ), 
			cmos_sensor( 0xA8, 0xB4 ), 
			cmos_sensor( 0xA9, 0xC6 ), 
			cmos_sensor( 0xAA, 0xD3 ), 
			cmos_sensor( 0xAB, 0xDD ),  
			cmos_sensor( 0xAC, 0xE5 ),  
			cmos_sensor( 0xAD, 0xF1 ), 
			cmos_sensor( 0xAE, 0xFA ), 
			cmos_sensor( 0xAF, 0xFF ), 	
			
		//case 2:			
			cmos_sensor( 0x9F, 0x0E ), 
			cmos_sensor( 0xA0, 0x1C ), 
			cmos_sensor( 0xA1, 0x34 ), 
			cmos_sensor( 0xA2, 0x48 ), 
			cmos_sensor( 0xA3, 0x5A ), 
			cmos_sensor( 0xA4, 0x6B ), 
			cmos_sensor( 0xA5, 0x7B ), 
			cmos_sensor( 0xA6, 0x95 ), 
			cmos_sensor( 0xA7, 0xAB ), 
			cmos_sensor( 0xA8, 0xBF ),
			cmos_sensor( 0xA9, 0xCE ), 
			cmos_sensor( 0xAA, 0xD9 ), 
			cmos_sensor( 0xAB, 0xE4 ),  
			cmos_sensor( 0xAC, 0xEC ), 
			cmos_sensor( 0xAD, 0xF7 ), 
			cmos_sensor( 0xAE, 0xFD ), 
			cmos_sensor( 0xAF, 0xFF ), 
		 
		//case 3
			cmos_sensor( 0x9F, 0x10 ), 
			cmos_sensor( 0xA0, 0x20 ), 
			cmos_sensor( 0xA1, 0x38 ), 
			cmos_sensor( 0xA2, 0x4E ), 
			cmos_sensor( 0xA3, 0x63 ), 
			cmos_sensor( 0xA4, 0x76 ), 
			cmos_sensor( 0xA5, 0x87 ), 
			cmos_sensor( 0xA6, 0xA2 ), 
			cmos_sensor( 0xA7, 0xB8 ), 
			cmos_sensor( 0xA8, 0xCA ), 
			cmos_sensor( 0xA9, 0xD8 ), 
			cmos_sensor( 0xAA, 0xE3 ), 
			cmos_sensor( 0xAB, 0xEB ), 
			cmos_sensor( 0xAC, 0xF0 ), 
			cmos_sensor( 0xAD, 0xF8 ), 
			cmos_sensor( 0xAE, 0xFD ), 
			cmos_sensor( 0xAF, 0xFF ), 

		 
		//case 4
			cmos_sensor( 0x9F, 0x14 ), 
			cmos_sensor( 0xA0, 0x28 ), 
			cmos_sensor( 0xA1, 0x44 ), 
			cmos_sensor( 0xA2, 0x5D ), 
			cmos_sensor( 0xA3, 0x72 ), 
			cmos_sensor( 0xA4, 0x86 ), 
			cmos_sensor( 0xA5, 0x95 ), 
			cmos_sensor( 0xA6, 0xB1 ), 
			cmos_sensor( 0xA7, 0xC6 ), 
			cmos_sensor( 0xA8, 0xD5 ), 
			cmos_sensor( 0xA9, 0xE1 ), 
			cmos_sensor( 0xAA, 0xEA ), 
			cmos_sensor( 0xAB, 0xF1 ), 
			cmos_sensor( 0xAC, 0xF5 ), 
			cmos_sensor( 0xAD, 0xFB ), 
			cmos_sensor( 0xAE, 0xFE ), 
			cmos_sensor( 0xAF, 0xFF ),
		 
		//case 5						// largest gamma curve
			cmos_sensor( 0x9F, 0x15 ), 
			cmos_sensor( 0xA0, 0x2A ), 
			cmos_sensor( 0xA1, 0x4A ), 
			cmos_sensor( 0xA2, 0x67 ), 
			cmos_sensor( 0xA3, 0x79 ), 
			cmos_sensor( 0xA4, 0x8C ), 
			cmos_sensor( 0xA5, 0x9A ), 
			cmos_sensor( 0xA6, 0xB3 ), 
			cmos_sensor( 0xA7, 0xC5 ), 
			cmos_sensor( 0xA8, 0xD5 ), 
			cmos_sensor( 0xA9, 0xDF ), 
			cmos_sensor( 0xAA, 0xE8 ), 
			cmos_sensor( 0xAB, 0xEE ), 
			cmos_sensor( 0xAC, 0xF3 ), 
			cmos_sensor( 0xAD, 0xFA ), 
			cmos_sensor( 0xAE, 0xFD ), 
			cmos_sensor( 0xAF, 0xFF ),
			 
	****************************************/
	cmos_sensor(0xc0 , 0x00),
	cmos_sensor(0xc1 , 0x10),
	cmos_sensor(0xc2 , 0x1C),
	cmos_sensor(0xc3 , 0x30),
	cmos_sensor(0xc4 , 0x43),
	cmos_sensor(0xc5 , 0x54),
	cmos_sensor(0xc6 , 0x65),
	cmos_sensor(0xc7 , 0x75),
	cmos_sensor(0xc8 , 0x93),
	cmos_sensor(0xc9 , 0xB0),
	cmos_sensor(0xca , 0xCB),
	cmos_sensor(0xcb , 0xE6),
	cmos_sensor(0xcc , 0xFF),
	cmos_sensor(0xf0 , 0x02),
	cmos_sensor(0xf1 , 0x01),
	cmos_sensor(0xf2 , 0x01),
	cmos_sensor(0xf3 , 0x30),
	cmos_sensor(0xf9 , 0x9f),
	cmos_sensor(0xfa , 0x78),

	//---------------------------------------------------------------
	cmos_sensor(0xfe, 0x01),  // setting awb

	cmos_sensor(0x00 , 0xf5),
	cmos_sensor(0x02 , 0x1a),
	cmos_sensor(0x0a , 0xa0),
	cmos_sensor(0x0b , 0x60),
	cmos_sensor(0x0c , 0x08),
	cmos_sensor(0x0e , 0x4c),
	cmos_sensor(0x0f , 0x39),
	cmos_sensor(0x11 , 0x3f),
	cmos_sensor(0x12 , 0x72),
	cmos_sensor(0x13 , 0x13),
	cmos_sensor(0x14 , 0x42),
	cmos_sensor(0x15 , 0x43),
	cmos_sensor(0x16 , 0xc2),
	cmos_sensor(0x17 , 0xa8),
	cmos_sensor(0x18 , 0x18),
	cmos_sensor(0x19 , 0x40),
	cmos_sensor(0x1a , 0xd0),
	cmos_sensor(0x1b , 0xf5),
	cmos_sensor(0x70 , 0x40),
	cmos_sensor(0x71 , 0x58),
	cmos_sensor(0x72 , 0x30),
	cmos_sensor(0x73 , 0x48),
	cmos_sensor(0x74 , 0x20),
	cmos_sensor(0x75 , 0x60),
	cmos_sensor(0x77 , 0x20),
	cmos_sensor(0x78 , 0x32),
	cmos_sensor(0x30 , 0x03),
	cmos_sensor(0x31 , 0x40),
	cmos_sensor(0x32 , 0xe0),
	cmos_sensor(0x33 , 0xe0),
	cmos_sensor(0x34 , 0xe0),
	cmos_sensor(0x35 , 0xb0),
	cmos_sensor(0x36 , 0xc0),
	cmos_sensor(0x37 , 0xc0),
	cmos_sensor(0x38 , 0x04),
	cmos_sensor(0x39 , 0x09),
	cmos_sensor(0x3a , 0x12),
	cmos_sensor(0x3b , 0x1C),
	cmos_sensor(0x3c , 0x28),
	cmos_sensor(0x3d , 0x31),
	cmos_sensor(0x3e , 0x44),
	cmos_sensor(0x3f , 0x57),
	cmos_sensor(0x40 , 0x6C),
	cmos_sensor(0x41 , 0x81),
	cmos_sensor(0x42 , 0x94),
	cmos_sensor(0x43 , 0xA7),
	cmos_sensor(0x44 , 0xB8),
	cmos_sensor(0x45 , 0xD6),
	cmos_sensor(0x46 , 0xEE),
	cmos_sensor(0x47 , 0x0d), // end awb
	
	cmos_sensor(0xfe, 0x00),

     cmos_sensor(0xd2 , 0x90),  // Open AEC at last.  


	cmos_sensor(0xfe, 0x00), 

	cmos_sensor(0x10 , 0x26),                                 
	cmos_sensor(0x11 , 0x0d),  // fd,modified by mormo 2010/07/06                               
	cmos_sensor(0x1a , 0x2a),  // 1e,modified by mormo 2010/07/06                                  

	cmos_sensor(0x1c , 0x49), // c1,modified by mormo 2010/07/06                                 
	cmos_sensor(0x1d , 0x9a), // 08,modified by mormo 2010/07/06                                 
	cmos_sensor(0x1e , 0x61), // 60,modified by mormo 2010/07/06                                 

	cmos_sensor(0x3a , 0x20),

	cmos_sensor(0x50 , 0x14),  // 10,modified by mormo 2010/07/06                               
	cmos_sensor(0x53 , 0x80),                                  
	cmos_sensor(0x56 , 0x80),
	
	cmos_sensor(0x8b , 0x20), //LSC                                 
	cmos_sensor(0x8c , 0x20),                                  
	cmos_sensor(0x8d , 0x20),                                  
	cmos_sensor(0x8e , 0x14),                                  
	cmos_sensor(0x8f , 0x10),                                  
	cmos_sensor(0x90 , 0x14),                                  

	cmos_sensor(0x94 , 0x02),                                  
	cmos_sensor(0x95 , 0x07),                                  
	cmos_sensor(0x96 , 0xe0),                                  

	cmos_sensor(0xb1 , 0x40), // YCPT                                 
	cmos_sensor(0xb2 , 0x40),                                  
	cmos_sensor(0xb3 , 0x40),
	cmos_sensor(0xb6 , 0xe0),

	//cmos_sensor(0xd0 , 0xcb), // AECT  c9,modifed by mormo 2010/07/06                                
	//cmos_sensor(0xd3 , 0x48), // 80,modified by mormor 2010/07/06                           

	cmos_sensor(0xf2 , 0x02),                                  
	cmos_sensor(0xf7 , 0x12),
	cmos_sensor(0xf8 , 0x0a),

	//Registers of Page1
	cmos_sensor(0xfe,  0x01),

	cmos_sensor(0x02 , 0x20),
	cmos_sensor(0x04 , 0x10),
	cmos_sensor(0x05 , 0x08),
	cmos_sensor(0x06 , 0x20),
	cmos_sensor(0x08 , 0x0a),

	cmos_sensor(0x0e , 0x44),                                  
	cmos_sensor(0x0f , 0x32),
	cmos_sensor(0x10 , 0x41),                                  
	cmos_sensor(0x11 , 0x37),                                  
	cmos_sensor(0x12 , 0x22),                                  
	cmos_sensor(0x13 , 0x19),                                  
	cmos_sensor(0x14 , 0x44),                                  
	cmos_sensor(0x15 , 0x44),  
	
	cmos_sensor(0x19 , 0x50),                                  
	cmos_sensor(0x1a , 0xd8), 
	
	cmos_sensor(0x32 , 0x10), 
	
	cmos_sensor(0x35 , 0x00),                                  
	cmos_sensor(0x36 , 0x80),                                  
	cmos_sensor(0x37 , 0x00), 

    cmos_sensor(0xfe , 0x00), 
	cmos_sensor(0xd2 , 0x90), 
	
	cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};


//SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA
static const struct sensor_cmd set_mode_switch_high_svga_reg_list[]=
{
    //640x480

    //record
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA
static const struct sensor_cmd set_mode_switch_low_svga_reg_list[]=
{
    //stop record

    //640x480
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA
static const struct sensor_cmd set_mode_switch_high_xuga_reg_list[]=
{
    //1600x1200
    //record    
 
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA
static const struct sensor_cmd set_mode_switch_upmid_xuga_reg_list[]=
{
    //1280x960
 
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};


//SET_MODE_SWITCH_SENSOR_TO_MID_XUGA
static const struct sensor_cmd set_mode_switch_mid_xuga_reg_list[]=
{  
    //640x480
 
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA
static const struct sensor_cmd set_mode_switch_low_xuga_reg_list[]=
{
    //320*240   
	cmos_sensor(0xfe, 0x01),

	cmos_sensor(0x54, 0x22),
	cmos_sensor(0xfe, 0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};


//SET_MODE_SENSOR_TO_HIGH_PREVIEW
static const struct sensor_cmd set_mode_high_prev_reg_list[]=
{
    //640x480

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SENSOR_TO_LOW_PREVIEW
static const struct sensor_cmd set_mode_low_prev_reg_list[]=
{
    //320*240
	cmos_sensor(0xfe, 0x01),

	cmos_sensor(0x54, 0x22),
	cmos_sensor(0xfe, 0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_MODE_CLOSE_SENSOR
static const struct sensor_cmd set_mode_close_reg_list[]=
{
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd (* sensor_set_mode_list[SET_MODE_NUM])[]={
//const struct sensor_cmd * sensor_set_mode_list[SET_MODE_NUM]={
    NULL,
    &set_mode_init_reg_list,
    &set_mode_switch_high_svga_reg_list,
    &set_mode_switch_low_svga_reg_list,
    &set_mode_switch_high_xuga_reg_list,
    &set_mode_switch_upmid_xuga_reg_list,
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
	cmos_sensor(0x5a,0x57), //for AWB can adjust back
	cmos_sensor(0x5b,0x40),
	cmos_sensor(0x5c,0x4a),			
	cmos_sensor(0x22,0x57),	 // Enable AWB
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_WB_INCANDESCENT
static const struct sensor_cmd set_wb_incandescent_reg_list[]=
{
	cmos_sensor(0x22, 0x55), 
	cmos_sensor(0x5a,0x48),
	cmos_sensor(0x5b,0x40),
	cmos_sensor(0x5c,0x5c),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),			
};
//SET_WB_FLUORESCENT
static const struct sensor_cmd set_wb_fluorescent_reg_list[]=
{
	cmos_sensor(0x22,0x55),   
	cmos_sensor(0x5a,0x40),
	cmos_sensor(0x5b,0x42),
	cmos_sensor(0x5c,0x50),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),	
};
//SET_WB_DAYLIGHT
static const struct sensor_cmd set_wb_daylight_reg_list[]=
{
	cmos_sensor(0x22,0x55 ),   
	cmos_sensor(0x5a,0x74), 
	cmos_sensor(0x5b,0x52),
	cmos_sensor(0x5c,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),	
};
//SET_WB_WARM_FLUORECENT

//SET_WB_CLOUDY
static const struct sensor_cmd set_wb_cloudy_reg_list[]=
{
	cmos_sensor(0x22,0x55),   // Disable AWB 
	cmos_sensor(0x5a,0x8c), //WB_manual_gain 
	cmos_sensor(0x5b,0x50),
	cmos_sensor(0x5c,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
};
//SET_WB_TWILIGHT
//SET_WB_SHADE
static const struct sensor_cmd set_wb_shade_reg_list[]=
{
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
	cmos_sensor(0x23,0x00),   //normal
	cmos_sensor(0x2d,0x0a), // 0x08
	cmos_sensor(0x20,0xff),
	cmos_sensor(0xd2,0x90),
	cmos_sensor(0x73,0x00),
	cmos_sensor(0x77,0x54),
	
	cmos_sensor(0xb3,0x40),
	cmos_sensor(0xb4,0x80),
	cmos_sensor(0xba,0x00),
	cmos_sensor(0xbb,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
	
};

//SET_EFFECT_MONO
static const struct sensor_cmd set_color_effect_mono_reg_list[]=
{
	cmos_sensor(0x23,0x02),   
	cmos_sensor(0x2d,0x0a), 
	cmos_sensor(0x20,0x7f),
	cmos_sensor(0xd2,0x90),
	cmos_sensor(0x73,0x00),
	cmos_sensor(0x77,0x54),
	
	cmos_sensor(0xb3,0x40),
	cmos_sensor(0xb4,0x80),
	cmos_sensor(0xba,0xd2),
	cmos_sensor(0xbb,0x28),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_NEGATIVE
static const struct sensor_cmd set_color_effect_negative_reg_list[]=
{
	cmos_sensor(0x23,0x01),    
	cmos_sensor(0x2d,0x0a),  
	cmos_sensor(0x20,0x7f),
	cmos_sensor(0xd2,0x90),
	cmos_sensor(0x73,0x00),
	cmos_sensor(0x77,0x54),
	
	cmos_sensor(0xb3,0x40),
	cmos_sensor(0xb4,0x80),
	cmos_sensor(0xba,0x00),
	cmos_sensor(0xbb,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_SOLARIZE
static const struct sensor_cmd set_color_effect_solarize_reg_list[]=
{
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_PASTEL
static const struct sensor_cmd set_color_effect_pastel_reg_list[]=
{
   cmos_sensor(MAG_REG,MAG_VAL),
   cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_MOSAIC
//SET_EFFECT_RESIZE

//SET_EFFECT_SEPIA
static const struct sensor_cmd set_color_effect_sepia_reg_list[]=
{
	cmos_sensor(0x23,0x02),		
	cmos_sensor(0x2d,0x0a),
	cmos_sensor(0x20,0xff),
	cmos_sensor(0xd2,0x90),
	cmos_sensor(0x73,0x00),

	cmos_sensor(0xb3,0x40),
	cmos_sensor(0xb4,0x80),
	cmos_sensor(0xba,0xd0),
	cmos_sensor(0xbb,0x28),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
};
//SET_EFFECT_POSTERIZE
//SET_EFFECT_WHITEBOARD
static const struct sensor_cmd set_color_effect_whiteboard_reg_list[]=
{
	cmos_sensor(0x23,0x02),		
	cmos_sensor(0x2d,0x0a),
	cmos_sensor(0x20,0xbf),
	cmos_sensor(0xd2,0x10),
	cmos_sensor(0x73,0x01),

	cmos_sensor(0x51,0x40),
	cmos_sensor(0x52,0x40),
	cmos_sensor(0xb3,0x60),
	cmos_sensor(0xb4,0x40),
	cmos_sensor(0xba,0x00),
	cmos_sensor(0xbb,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
};
//SET_EFFECT_BLACKBOARD
static const struct sensor_cmd set_color_effect_blackboard_reg_list[]=
{
	cmos_sensor(0x23,0x02),	
	cmos_sensor(0x2d,0x0a),
	cmos_sensor(0x20,0xbf),
	cmos_sensor(0xd2,0x10),
	cmos_sensor(0x73,0x01),

	cmos_sensor(0x51,0x40),
	cmos_sensor(0x52,0x40),

	cmos_sensor(0xb3,0x98),
	cmos_sensor(0xb4,0xb0),
	cmos_sensor(0xba,0x00),
	cmos_sensor(0xbb,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
};

//SET_EFFECT_AQUA
static const struct sensor_cmd set_color_effect_aqua_reg_list[]=
{
	cmos_sensor(0x23,0x02),	
	cmos_sensor(0x2d,0x0a),
	cmos_sensor(0x20,0x7f),
	cmos_sensor(0xd2,0x90),
	cmos_sensor(0x77,0x88),
	


	cmos_sensor(0xb3,0x40),
	cmos_sensor(0xb4,0x80),
	cmos_sensor(0xba,0xc0),
	cmos_sensor(0xbb,0xc0),
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
    /* 50 Hz */     
	cmos_sensor(0x01  ,0xfa), 	  // 24M
	cmos_sensor(0x02  ,0x70), 
	cmos_sensor(0x0f  ,0x01),

	//cmos_sensor(0x03  ,0x01), 	
	//cmos_sensor(0x04  ,0x2c), 	

	cmos_sensor(0xe2  ,0x00), 	//anti-flicker step [11:8]
	cmos_sensor(0xe3  ,0x64),   //anti-flicker step [7:0]
		
	cmos_sensor(0xe4  ,0x02),   //exp level 1  16.67fps
	cmos_sensor(0xe5  ,0x58), 
	cmos_sensor(0xe6  ,0x03),   //exp level 2  12.5fps
	cmos_sensor(0xe7  ,0x20), 
	cmos_sensor(0xe8  ,0x04),   //exp level 3  8.33fps
	cmos_sensor(0xe9  ,0xb0), 
	cmos_sensor(0xea  ,0x09),   //exp level 4  4.00fps
	cmos_sensor(0xeb  ,0xc4),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
};

static const struct sensor_cmd set_antibanding_60hz_reg_list[]=
{
    /* 60 Hz */     
	cmos_sensor(0x01  ,0x2c), 	
	cmos_sensor(0x02  ,0x98), 
	cmos_sensor(0x0f  ,0x02),

		//cmos_sensor(0x03  ,0x01), 	
	//cmos_sensor(0x04  ,0x40), 	

	cmos_sensor(0xe2  ,0x00), 	//anti-flicker step [11:8]
	cmos_sensor(0xe3  ,0x50),   //anti-flicker step [7:0]
		
	cmos_sensor(0xe4  ,0x02),   //exp level 1  15.00fps
	cmos_sensor(0xe5  ,0x80), 
	cmos_sensor(0xe6  ,0x03),   //exp level 2  10.00fps
	cmos_sensor(0xe7  ,0xc0), 
	cmos_sensor(0xe8  ,0x05),   //exp level 3  7.50fps
	cmos_sensor(0xe9  ,0x00), 
	cmos_sensor(0xea  ,0x09),   //exp level 4  4.00fps
	cmos_sensor(0xeb  ,0x60), 
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
    /*cmos_sensor(0x03,0x10),
    cmos_sensor(0x12,0x30), 
    cmos_sensor(0x41,0x15),*/
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_SCENCE_NIGHT
static const struct sensor_cmd set_scence_night_reg_list[]=
{
  
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

const struct sensor_op_list * gc0308_get_op_table(int op)
{
    if(op<SENSOR_OP_NUM)
        return GET_OP_TABLE(op);
        
    return NULL;
}

extern struct cam_interface gc0308_interface;

struct cam_interface *gc0308_detect(int * invvsync)
{
    unsigned char id;

    if(cam_reg_read(GC0308_I2C_ADDR,0x00,&id,1)==0){
        printk("check camara(gc0308 id 0x9b) 0x%x\n",id);
        if(id==0x9b){
            printk("Found Camara GC0308 ID[0x%x]\n",id);
            if(invvsync)
                *invvsync=1;
            return &gc0308_interface;
        }
    }
   
    return NULL;
}


int  gc0308_init(void)
{
    int ret;

    cam_power_set(CAMARA_POWER_ON);

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_INIT_SENSOR);
    
    return ret;
}

int  gc0308_switch_low_svga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA);
    return ret;
}
int  gc0308_switch_high_svga(void){
    int ret;

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA);
    return ret;
}
int  gc0308_switch_high_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA);
    return ret;
}
int  gc0308_switch_upmid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA);
    return ret;
}
int  gc0308_switch_mid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_MID_XUGA);
    return ret;
}
int  gc0308_switch_low_xuga(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA);
    return ret;
}
int  gc0308_to_high_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_HIGH_PREVIEW);
    return ret;
}
int  gc0308_to_low_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_LOW_PREVIEW);
    return ret;
}

//int  cam_switch_low_xuga(void){return -1;}
int  gc0308_close(void)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_CLOSE_SENSOR);
    
    cam_power_set(CAMARA_POWER_OFF);

    return ret;
}
//-----------------------------------
//      SENSOR_SET_WHITE_BALANCE
//-----------------------------------
int  gc0308_set_wb(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_WHITE_BALANCE,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_COLOR_EFFECT
//-----------------------------------
int  gc0308_set_effect(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_COLOR_EFFECT,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------
int  gc0308_set_brightness(unsigned char value)
{
    struct sensor_cmd cmd;
    int ret;
    
    cmd.reg = 0x03;
    cmd.val = value;
    ret=cam_reg_write(GC0308_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));

    if(ret)
        return ret;

    cmd.reg = 0x40;
    cmd.val = value;
    ret=cam_reg_write(GC0308_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));
    
    return ret;
}

//-----------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------
int  gc0308_night_mode_on(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_NIGHT);
    return ret;
}                                                    
int  gc0308_night_mode_off(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_AUTO);
    return ret;
}

int  gc0308_dump_reg(void)
{
    unsigned char reg;
    unsigned char val;
    int ret;
    
    for(reg=0;reg<0xf0;reg++){

        ret = cam_reg_read(GC0308_I2C_ADDR,reg,&val,1);
        if(ret != 0){
            printk("reg 0x%x  failed\n",reg);
        }else{
            printk("0x%x 0x%x\n",reg,val);
        }
    }

    return 0;
}

struct device_info gc0308_info={
    GC0308_I2C_ADDR,
};


struct cam_interface gc0308_interface={
    &gc0308_info,
    gc0308_get_op_table,
    gc0308_init,
    gc0308_switch_low_svga,
    gc0308_switch_high_svga,
    gc0308_switch_high_xuga,
    gc0308_switch_upmid_xuga,
    gc0308_switch_mid_xuga,
    gc0308_switch_low_xuga,
    gc0308_to_high_preview,
    gc0308_to_low_preview,
    gc0308_close,
    gc0308_set_wb,
    gc0308_set_effect,
    gc0308_set_brightness,
    gc0308_night_mode_on,                                                    
    gc0308_night_mode_off,
    gc0308_dump_reg,
};

