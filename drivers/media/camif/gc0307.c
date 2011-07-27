

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

#include "gc0307.h"


//-----------------------------
//      SENSOR_SET_MODE
//-----------------------------

//INIT_SENSOR
static const struct sensor_cmd set_mode_init_reg_list[]=
{
  	// Initail Sequence Write In.
	//========= close output
	cmos_sensor(0x43, 0x00),
	cmos_sensor(0x44, 0xa2),
	//========= close some functions
	// open them after configure their parmameters
	cmos_sensor(0x40, 0x10),
	cmos_sensor(0x41, 0x00), 			
	cmos_sensor(0x42, 0x10),					  	
	cmos_sensor(0x47, 0x00), //mode1,				  	
	cmos_sensor(0x48, 0xc3), //mode2, 	
	cmos_sensor(0x49, 0x00), //dither_mode 		
	cmos_sensor(0x4a, 0x00), //clock_gating_en
	cmos_sensor(0x4b, 0x00), //mode_reg3
	cmos_sensor(0x4E, 0x23), //sync mode
	cmos_sensor(0x4F, 0x01), //AWB, AEC, every N frame	
	
	//========= frame timing
	cmos_sensor(0x01, 0x6a), //HB
	cmos_sensor(0x02, 0x0c), //VB
	cmos_sensor(0x1C, 0x00), //Vs_st
	cmos_sensor(0x1D, 0x00), //Vs_et
	cmos_sensor(0x10, 0x00), //high 4 bits of VB, HB
	cmos_sensor(0x11, 0x05), //row_tail,  AD_pipe_number
	
	//========= windowing
	cmos_sensor(0x05, 0x00), //row_start
	cmos_sensor(0x06, 0x00),
	cmos_sensor(0x07, 0x00), //col start
	cmos_sensor(0x08, 0x00), 
	cmos_sensor(0x09, 0x01), //win height
	cmos_sensor(0x0A, 0xE8),
	cmos_sensor(0x0B, 0x02), //win width, pixel array only 640
	cmos_sensor(0x0C, 0x80),
	
	//========= analog
	cmos_sensor(0x0D, 0x22), //rsh_width
	cmos_sensor(0x0E, 0x02), //CISCTL mode2,  
	cmos_sensor(0x12, 0x70), //7 hrst, 6_4 darsg,
	cmos_sensor(0x13, 0x00), //7 CISCTL_restart, 0 apwd
	cmos_sensor(0x14, 0x00), //NA
	cmos_sensor(0x15, 0xba), //7_4 vref
	cmos_sensor(0x16, 0x13), //5to4 _coln_r,  __1to0__da18
	cmos_sensor(0x17, 0x52), //opa_r, ref_r, sRef_r
	//cmos_sensor(0x18, 0xc0), //analog_mode, best case for left band.
	
	cmos_sensor(0x1E, 0x0d), //tsp_width 		   
	cmos_sensor(0x1F, 0x32), //sh_delay
	
	//========= offset
	cmos_sensor(0x47, 0x00),  //7__test_image, __6__fixed_pga, __5__auto_DN, __4__CbCr_fix, 
				//__3to2__dark_sequence, __1__allow_pclk_vcync, __0__LSC_test_image
	cmos_sensor(0x19, 0x06),  //pga_o			 
	cmos_sensor(0x1a, 0x06),  //pga_e			 
	
	cmos_sensor(0x31, 0x00),  //pga_oFFset ,	 high 8bits of 11bits
	cmos_sensor(0x3B, 0x00),  //global_oFFset, low 8bits of 11bits
	
	cmos_sensor(0x59, 0x0f),  //offset_mode 	
	cmos_sensor(0x58, 0x88),  //DARK_VALUE_RATIO_G,  DARK_VALUE_RATIO_RB
	cmos_sensor(0x57, 0x08),  //DARK_CURRENT_RATE
	cmos_sensor(0x56, 0x77),  //PGA_OFFSET_EVEN_RATIO, PGA_OFFSET_ODD_RATIO
	
	//========= blk
	cmos_sensor(0x35, 0xd8),  //blk_mode
	cmos_sensor(0x36, 0x40),  
	cmos_sensor(0x3C, 0x00), 
	cmos_sensor(0x3D, 0x00), 
	cmos_sensor(0x3E, 0x00), 
	cmos_sensor(0x3F, 0x00), 
	cmos_sensor(0xb5, 0x70), 
	cmos_sensor(0xb6, 0x40), 
	cmos_sensor(0xb7, 0x00), 
	cmos_sensor(0xb8, 0x38), 
	cmos_sensor(0xb9, 0xc3), 		  
	cmos_sensor(0xba, 0x0f), 
	
	cmos_sensor(0x7e, 0x35), 
	cmos_sensor(0x7f, 0x86), 
	cmos_sensor(0x5c, 0x68), //78
	cmos_sensor(0x5d, 0x78), //88
	
	//========= manual_gain 
	cmos_sensor(0x61, 0x80), //manual_gain_g1	
	cmos_sensor(0x63, 0x80), //manual_gain_r
	cmos_sensor(0x65, 0x98), //manual_gai_b, 0xa0=1.25, 0x98=1.1875
	cmos_sensor(0x67, 0x80), //manual_gain_g2
	cmos_sensor(0x68, 0x18), //global_manual_gain	 2.4bits
	
	//=========CC _R
	cmos_sensor(0x69, 0x58),  //54
	cmos_sensor(0x6A, 0xf6),  //ff
	cmos_sensor(0x6B, 0xfb),  //fe
	cmos_sensor(0x6C, 0xf4),  //ff
	cmos_sensor(0x6D, 0x5a),  //5f
	cmos_sensor(0x6E, 0xe6),  //e1
	cmos_sensor(0x6f, 0x00), 	
	
	//=========lsc							  
	cmos_sensor(0x70, 0x14), 
	cmos_sensor(0x71, 0x1c), 
	cmos_sensor(0x72, 0x20), 
	cmos_sensor(0x73, 0x10), 	
	cmos_sensor(0x74, 0x3c), 
	cmos_sensor(0x75, 0x52), 
	
	//=========dn																			 
	cmos_sensor(0x7d, 0x2f),  //dn_mode   	
	cmos_sensor(0x80, 0x0c), //when auto_dn, check 7e,7f
	cmos_sensor(0x81, 0x0c),
	cmos_sensor(0x82, 0x44),
																						
	//dd																		   
	cmos_sensor(0x83, 0x18),  //DD_TH1 					  
	cmos_sensor(0x84, 0x18),  //DD_TH2 					  
	cmos_sensor(0x85, 0x04),  //DD_TH3 																							  
	cmos_sensor(0x87, 0x34),  //32 b DNDD_low_range X16,  DNDD_low_range_C_weight_center					

	//=========intp-ee																		   
	cmos_sensor(0x88, 0x04),  													   
	cmos_sensor(0x89, 0x01),  										  
	cmos_sensor(0x8a, 0x50),//60  										   
	cmos_sensor(0x8b, 0x50),//60  										   
	cmos_sensor(0x8c, 0x07),  												 				  
																					  
	cmos_sensor(0x50, 0x0c),   						   		
	cmos_sensor(0x5f, 0x3c), 																					 
																					 
	cmos_sensor(0x8e, 0x02),  															  
	cmos_sensor(0x86, 0x02),  																  
	cmos_sensor(0x51, 0x20),  																
	cmos_sensor(0x52, 0x08),  
	cmos_sensor(0x53, 0x00), 
	
	//========= YCP 
	//contrast_center																			  
	cmos_sensor(0x77, 0x80), //contrast_center 																  
	cmos_sensor(0x78, 0x00), //fixed_Cb																		  
	cmos_sensor(0x79, 0x00), //fixed_Cr																		  
	cmos_sensor(0x7a, 0x00), //luma_offset 																																							
	cmos_sensor(0x7b, 0x40), //hue_cos 																		  
	cmos_sensor(0x7c, 0x00), //hue_sin 																		  
																							 
	//saturation																				  
	cmos_sensor(0xa0, 0x40), //global_saturation
	cmos_sensor(0xa1, 0x40), //luma_contrast																	  
	cmos_sensor(0xa2, 0x34), //saturation_Cb																	  
	cmos_sensor(0xa3, 0x34), //saturation_Cr
																				
	cmos_sensor(0xa4, 0xc8), 																  
	cmos_sensor(0xa5, 0x02), 
	cmos_sensor(0xa6, 0x28), 																			  
	cmos_sensor(0xa7, 0x02), 
	
	//skin																								  
	cmos_sensor(0xa8, 0xee), 															  
	cmos_sensor(0xa9, 0x12), 															  
	cmos_sensor(0xaa, 0x01), 														  
	cmos_sensor(0xab, 0x20), 													  
	cmos_sensor(0xac, 0xf0), 														  
	cmos_sensor(0xad, 0x10), 															  
		
	//========= ABS
	cmos_sensor(0xae, 0x18), 
	cmos_sensor(0xaf, 0x74), 
	cmos_sensor(0xb0, 0xe0), 	  
	cmos_sensor(0xb1, 0x20), 
	cmos_sensor(0xb2, 0x6c), 
	cmos_sensor(0xb3, 0x40), 
	cmos_sensor(0xb4, 0x04), 
		
	//========= AWB 
	cmos_sensor(0xbb, 0x42), 
	cmos_sensor(0xbc, 0x60),
	cmos_sensor(0xbd, 0x50),
	cmos_sensor(0xbe, 0x50),
	
	cmos_sensor(0xbf, 0x0c), 
	cmos_sensor(0xc0, 0x06), 
	cmos_sensor(0xc1, 0x60), 
	cmos_sensor(0xc2, 0xf1),  //f1
	cmos_sensor(0xc3, 0x40),
	cmos_sensor(0xc4, 0x1c), //18//20
	cmos_sensor(0xc5, 0x56),  //33
	cmos_sensor(0xc6, 0x1d), 
	cmos_sensor(0xca, 0x70), 
	cmos_sensor(0xcb, 0x70), 
	cmos_sensor(0xcc, 0x78),
	cmos_sensor(0xcd, 0x80), //R_ratio 									 
	cmos_sensor(0xce, 0x80), //G_ratio,  cold_white white 								   
	cmos_sensor(0xcf, 0x80), //B_ratio  	
	
	//=========  aecT  
	cmos_sensor(0x20, 0x06),//0x02 
	cmos_sensor(0x21, 0xc0), 
	cmos_sensor(0x22, 0x60),    
	cmos_sensor(0x23, 0x88), 
	cmos_sensor(0x24, 0x96), 
	cmos_sensor(0x25, 0x30), 
	cmos_sensor(0x26, 0xd0), 
	cmos_sensor(0x27, 0x00), 
	cmos_sensor(0x28, 0x01), //AEC_exp_level_1bit11to8   
	cmos_sensor(0x29, 0xf4), //AEC_exp_level_1bit7to0	  
	cmos_sensor(0x2a, 0x02), //AEC_exp_level_2bit11to8   
	cmos_sensor(0x2b, 0xbc), //AEC_exp_level_2bit7to0			 
	cmos_sensor(0x2c, 0x03), //AEC_exp_level_3bit11to8   659 - 8FPS,  8ca - 6FPS  //	 
	cmos_sensor(0x2d, 0xe8), //AEC_exp_level_3bit7to0			 
	cmos_sensor(0x2e, 0x09), //AEC_exp_level_4bit11to8   4FPS 
	cmos_sensor(0x2f, 0xc4), //AEC_exp_level_4bit7to0	 
	cmos_sensor(0x30, 0x20), 						  
	cmos_sensor(0x31, 0x00), 					   
	cmos_sensor(0x32, 0x1c), 
	cmos_sensor(0x33, 0x90), 			  
	cmos_sensor(0x34, 0x10),	

	cmos_sensor(0xd0, 0x34), 
	cmos_sensor(0xd1, 0x50), //AEC_target_Y						   
	cmos_sensor(0xd2, 0x61),//0xf2 	  
	cmos_sensor(0xd4, 0x96), 
	cmos_sensor(0xd5, 0x01), // william 0318
	cmos_sensor(0xd6, 0x96), //antiflicker_step 					   
	cmos_sensor(0xd7, 0x03), //AEC_exp_time_min ,william 20090312			   
	cmos_sensor(0xd8, 0x02), 
	cmos_sensor(0xdd, 0x22),//0x12 
	  															
	//========= measure window										
	cmos_sensor(0xe0, 0x03), 						 
	cmos_sensor(0xe1, 0x02), 							 
	cmos_sensor(0xe2, 0x27), 								 
	cmos_sensor(0xe3, 0x1e), 				 
	cmos_sensor(0xe8, 0x3b), 					 
	cmos_sensor(0xe9, 0x6e), 						 
	cmos_sensor(0xea, 0x2c), 					 
	cmos_sensor(0xeb, 0x50), 					 
	cmos_sensor(0xec, 0x73), 		 
	
	//========= close_frame													
	cmos_sensor(0xed, 0x00), //close_frame_num1 ,can be use to reduce FPS				 
	cmos_sensor(0xee, 0x00), //close_frame_num2  
	cmos_sensor(0xef, 0x00), //close_frame_num
	
	// page1
	cmos_sensor(0xf0, 0x01), //select page1 
	cmos_sensor(0x00, 0x20),
	cmos_sensor(0x01, 0x20),
	cmos_sensor(0x02, 0x20),
	cmos_sensor(0x03, 0x20),
	cmos_sensor(0x04, 0x78),
	cmos_sensor(0x05, 0x78),
	cmos_sensor(0x06, 0x78),
	cmos_sensor(0x07, 0x78),

	cmos_sensor(0x10, 0x04),
	cmos_sensor(0x11, 0x04),
	cmos_sensor(0x12, 0x04),
	cmos_sensor(0x13, 0x04),
	cmos_sensor(0x14, 0x01),
	cmos_sensor(0x15, 0x01),
	cmos_sensor(0x16, 0x01),
	cmos_sensor(0x17, 0x01),

	cmos_sensor(0x20, 0x00),
	cmos_sensor(0x21, 0x00),
	cmos_sensor(0x22, 0x00),
	cmos_sensor(0x23, 0x00),
	cmos_sensor(0x24, 0x00),
	cmos_sensor(0x25, 0x00),
	cmos_sensor(0x26, 0x00),
	cmos_sensor(0x27, 0x00),
	cmos_sensor(0x40, 0x11),
	
	//=============================lscP 
	cmos_sensor(0x45, 0x06),
	cmos_sensor(0x46, 0x06),
	cmos_sensor(0x47, 0x05),
	cmos_sensor(0x48, 0x04),
	cmos_sensor(0x49, 0x03),
	cmos_sensor(0x4a, 0x03),

	cmos_sensor(0x62, 0xd8),
	cmos_sensor(0x63, 0x24),
	cmos_sensor(0x64, 0x24),
	cmos_sensor(0x65, 0x24),
	cmos_sensor(0x66, 0xd8),
	cmos_sensor(0x67, 0x24),

	cmos_sensor(0x5a, 0x00),
	cmos_sensor(0x5b, 0x00),
	cmos_sensor(0x5c, 0x00),
	cmos_sensor(0x5d, 0x00),
	cmos_sensor(0x5e, 0x00),
	cmos_sensor(0x5f, 0x00),
	
	//============================= ccP 
	cmos_sensor(0x69, 0x03), //cc_mode
		  
	//CC_G
	cmos_sensor(0x70, 0x5d), 
	cmos_sensor(0x71, 0xed), 
	cmos_sensor(0x72, 0xff), 
	cmos_sensor(0x73, 0xe5), 
	cmos_sensor(0x74, 0x5f), 
	cmos_sensor(0x75, 0xe6), 
	
  //CC_B
	cmos_sensor(0x76, 0x41), 
	cmos_sensor(0x77, 0xef), 
	cmos_sensor(0x78, 0xff), 
	cmos_sensor(0x79, 0xff), 
	cmos_sensor(0x7a, 0x5f), 
	cmos_sensor(0x7b, 0xfa), 	 
	
	//============================= AGP
	cmos_sensor(0x7e, 0x00),  
	cmos_sensor(0x7f, 0x00),  
	cmos_sensor(0x80, 0xc8),  
	cmos_sensor(0x81, 0x06),  
	cmos_sensor(0x82, 0x08),  
	
	cmos_sensor(0x83, 0x23),  
	cmos_sensor(0x84, 0x38),  
	cmos_sensor(0x85, 0x4F),  
	cmos_sensor(0x86, 0x61),  
	cmos_sensor(0x87, 0x72),  
	cmos_sensor(0x88, 0x80),  
	cmos_sensor(0x89, 0x8D),  
	cmos_sensor(0x8a, 0xA2),  
	cmos_sensor(0x8b, 0xB2),  
	cmos_sensor(0x8c, 0xC0),  
	cmos_sensor(0x8d, 0xCA),  
	cmos_sensor(0x8e, 0xD3),  
	cmos_sensor(0x8f, 0xDB),  
	cmos_sensor(0x90, 0xE2),  
	cmos_sensor(0x91, 0xED),  
	cmos_sensor(0x92, 0xF6),  
	cmos_sensor(0x93, 0xFD),  
	
	//about gamma1 is hex r oct
	cmos_sensor(0x94, 0x04),  
	cmos_sensor(0x95, 0x0E),  
	cmos_sensor(0x96, 0x1B),  
	cmos_sensor(0x97, 0x28),  
	cmos_sensor(0x98, 0x35),  
	cmos_sensor(0x99, 0x41),  
	cmos_sensor(0x9a, 0x4E),  
	cmos_sensor(0x9b, 0x67),  
	cmos_sensor(0x9c, 0x7E),  
	cmos_sensor(0x9d, 0x94),  
	cmos_sensor(0x9e, 0xA7),  
	cmos_sensor(0x9f, 0xBA),  
	cmos_sensor(0xa0, 0xC8),  
	cmos_sensor(0xa1, 0xD4),  
	cmos_sensor(0xa2, 0xE7),  
	cmos_sensor(0xa3, 0xF4),  
	cmos_sensor(0xa4, 0xFA), 
	
	//========= open functions	
	cmos_sensor(0xf0, 0x00), //set back to page0	
	cmos_sensor(0x40, 0x7e), 
	cmos_sensor(0x41, 0x2F),
	

	//=========open output
	cmos_sensor(0x43, 0x40),
	cmos_sensor(0x44, 0xE2),

	cmos_sensor(0x0f, 0xa2/*0x82*/),  //b2
	cmos_sensor(0x45, 0x26/*0x24*/),  // 27
	cmos_sensor(0x47, 0x28/*0x20*/),   //  2c

    cmos_sensor(0x01,/*0x32*/0xfa), //normal preview  50hz  24M
    cmos_sensor(0x02,0x70), 
    cmos_sensor(0x10,0x01),
    cmos_sensor(0xd6,/*0x78*/0x64), 
    cmos_sensor(0x28,0x02), 
    cmos_sensor(0x29,0x58), 
    cmos_sensor(0x2a,0x02), 
    cmos_sensor(0x2b,0x58), 
    cmos_sensor(0x2c,0x02), 
    cmos_sensor(0x2d,0x58), 
    cmos_sensor(0x2e,0x04), 
    cmos_sensor(0x2f,0xb0), 

	
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

		cmos_sensor(0xf0,  0x00), 
		cmos_sensor(0x0e , 0x0a),  //row even skip
		cmos_sensor(0x43 , 0xc0),  //more boundary mode opclk output enable
			//{0x44 , 0xe2),  // mtk is e0
			//{0x45,  0x2b),  //col subsample  
		cmos_sensor(0x45,  0x28),  //col subsample  
		cmos_sensor(0x4e,  0x33),  //  32 opclk gate in subsample  // mtk is 33

		cmos_sensor(0x01,  0xd1), 
		cmos_sensor(0x02,  0x82), 
		cmos_sensor(0x10,  0x00), 
		cmos_sensor(0xd6,  0xce), 
		                            
		cmos_sensor(0x28,  0x02),  //AEC_exp_level_1bit11to8   // 33.3fps
		cmos_sensor(0x29,  0x6a),  //AEC_exp_level_1bit7to0 
		cmos_sensor(0x2a,  0x04),  //AEC_exp_level_2bit11to8   // 20fps
		cmos_sensor(0x2b,  0x06),  //AEC_exp_level_2bit7to0  
		cmos_sensor(0x2c,  0x06),  //AEC_exp_level_3bit11to8    // 12.5fps
		cmos_sensor(0x2d,  0x70),  //AEC_exp_level_3bit7to0           
		cmos_sensor(0x2e,  0x0c),  //AEC_exp_level_4bit11to8   // 6.25fps
		cmos_sensor(0x2f,  0xe0),  //AEC_exp_level_4bit7to0   
		                            
		cmos_sensor(0xe1,  0x01),  //big_win_y0                                                     
		cmos_sensor(0xe3,  0x0f),  //432, big_win_y1    , height                                 
		cmos_sensor(0xea,  0x16),  //small_win_height1                        
		cmos_sensor(0xeb,  0x28),  //small_win_height2                        
		cmos_sensor(0xec,  0x39),  //small_win_heigh3 //only for AWB 
		cmos_sensor(0xae,  0x0c),  //black pixel target number
		cmos_sensor(0xc3,  0x20),  //number limit
		cmos_sensor(0x74,  0x1e),  //lsc_row_center , 0x3c
		cmos_sensor(0x75,  0x52),  //lsc_col_center , 0x52
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

		cmos_sensor(0xf0,  0x00), //  set  page 0
		cmos_sensor(0x0e , 0x0a),  //row even skip
		cmos_sensor(0x43 , 0xc0),  //more boundary mode opclk output enable
			//{0x44 , 0xe2),  // mtk is e0
			//{0x45,  0x2b),  //col subsample  
		cmos_sensor(0x45,  0x28),  //col subsample  
		cmos_sensor(0x4e,  0x33),  //  32 opclk gate in subsample  // mtk is 33

		cmos_sensor(0x01,  0xd1), 
		cmos_sensor(0x02,  0x82), 
		cmos_sensor(0x10,  0x00), 
		cmos_sensor(0xd6,  0xce), 
		                            
		cmos_sensor(0x28,  0x02),  //AEC_exp_level_1bit11to8   // 33.3fps
		cmos_sensor(0x29,  0x6a),  //AEC_exp_level_1bit7to0 
		cmos_sensor(0x2a,  0x04),  //AEC_exp_level_2bit11to8   // 20fps
		cmos_sensor(0x2b,  0x06),  //AEC_exp_level_2bit7to0  
		cmos_sensor(0x2c,  0x06),  //AEC_exp_level_3bit11to8    // 12.5fps
		cmos_sensor(0x2d,  0x70),  //AEC_exp_level_3bit7to0           
		cmos_sensor(0x2e,  0x0c),  //AEC_exp_level_4bit11to8   // 6.25fps
		cmos_sensor(0x2f,  0xe0),  //AEC_exp_level_4bit7to0   
		                            
		cmos_sensor(0xe1,  0x01),  //big_win_y0                                                     
		cmos_sensor(0xe3,  0x0f),  //432, big_win_y1    , height                                 
		cmos_sensor(0xea,  0x16),  //small_win_height1                        
		cmos_sensor(0xeb,  0x28),  //small_win_height2                        
		cmos_sensor(0xec,  0x39),  //small_win_heigh3 //only for AWB 
		cmos_sensor(0xae,  0x0c),  //black pixel target number
		cmos_sensor(0xc3,  0x20),  //number limit
		cmos_sensor(0x74,  0x1e),  //lsc_row_center , 0x3c
		cmos_sensor(0x75,  0x52),  //lsc_col_center , 0x52
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
			cmos_sensor(0xc7,0x4c), //for AWB can adjust back
			cmos_sensor(0xc8,0x40),
			cmos_sensor(0xc9,0x4a),			
			cmos_sensor(0x41,0x2F),	 // Enable AWB
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_WB_INCANDESCENT
static const struct sensor_cmd set_wb_incandescent_reg_list[]=
{
			cmos_sensor(0x41,0x2b),  //disable awb
			cmos_sensor(0xc7,0x48),
			cmos_sensor(0xc8,0x40),
			cmos_sensor(0xc9,0x5c),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),			
};
//SET_WB_FLUORESCENT
static const struct sensor_cmd set_wb_fluorescent_reg_list[]=
{
			cmos_sensor(0x41,0x2b),   
			cmos_sensor(0xc7,0x40),
			cmos_sensor(0xc8,0x42),
			cmos_sensor(0xc9,0x50),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),	
};
//SET_WB_DAYLIGHT
static const struct sensor_cmd set_wb_daylight_reg_list[]=
{
            cmos_sensor(0x41,0x2b),   
            cmos_sensor(0xc7,0x50),
            cmos_sensor(0xc8,0x45),
            cmos_sensor(0xc9,0x40),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),	
};
//SET_WB_WARM_FLUORECENT

//SET_WB_CLOUDY
static const struct sensor_cmd set_wb_cloudy_reg_list[]=
{
			cmos_sensor(0x41,0x2b),   
			cmos_sensor(0xc7,0x5a), //WB_manual_gain
			cmos_sensor(0xc8,0x42),
			cmos_sensor(0xc9,0x40),
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

			cmos_sensor(0x41,0x2f),   // normal

			cmos_sensor(0x40,0x7e),
			cmos_sensor(0x42,0x10),
			
			cmos_sensor(0x47, 0x20),  //change
			cmos_sensor(0x48,0xc3),
			cmos_sensor(0x8a,0x50),//60
			cmos_sensor(0x8b,0x50),
			cmos_sensor(0x8c,0x07),
			cmos_sensor(0x50,0x0c),
			cmos_sensor(0x77,0x80),
			cmos_sensor(0xa1,0x40),
			cmos_sensor(0x7a,0x00),
			cmos_sensor(0x78,0x00),
			cmos_sensor(0x79,0x00),
			cmos_sensor(0x7b,0x40),
			cmos_sensor(0x7c,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),		
	
};

//SET_EFFECT_MONO
static const struct sensor_cmd set_color_effect_mono_reg_list[]=
{
			cmos_sensor(0x41,0x2f),   // danse   w&b
			
			cmos_sensor(0x40,0x7e),
			cmos_sensor(0x42,0x10),
			cmos_sensor(0x47, 0x30),
			cmos_sensor(0x48,0xc3),
			cmos_sensor(0x8a,0x60),
			cmos_sensor(0x8b,0x60),
			cmos_sensor(0x8c,0x07),
			cmos_sensor(0x50,0x0c),
			cmos_sensor(0x77,0x80),
			cmos_sensor(0xa1,0x40),
			cmos_sensor(0x7a,0x00),
			cmos_sensor(0x78,0x00),
			cmos_sensor(0x79,0x00),
			cmos_sensor(0x7b,0x40),
			cmos_sensor(0x7c,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

		 
};
//SET_EFFECT_NEGATIVE
static const struct sensor_cmd set_color_effect_negative_reg_list[]=
{

			cmos_sensor(0x41,0x6f),

			cmos_sensor(0x40,0x7e),
			cmos_sensor(0x42,0x10),
			cmos_sensor(0x47, 0x30),
			cmos_sensor(0x48,0xc3),
			cmos_sensor(0x8a,0x60),
			cmos_sensor(0x8b,0x60),
			cmos_sensor(0x8c,0x07),
			cmos_sensor(0x50,0x0c),
			cmos_sensor(0x77,0x80),
			cmos_sensor(0xa1,0x40),
			cmos_sensor(0x7a,0x00),
			cmos_sensor(0x78,0x00),
			cmos_sensor(0x79,0x00),
			cmos_sensor(0x7b,0x40),
			cmos_sensor(0x7c,0x00),
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
		
			cmos_sensor(0x41,0x2f),  //SEPIA
			
			cmos_sensor(0x40,0x7e),
			cmos_sensor(0x42,0x10),
			cmos_sensor(0x47, 0x30),
			cmos_sensor(0x48,0xc3),
			cmos_sensor(0x8a,0x60),
			cmos_sensor(0x8b,0x60),
			cmos_sensor(0x8c,0x07),
			cmos_sensor(0x50,0x0c),
			cmos_sensor(0x77,0x80),
			cmos_sensor(0xa1,0x40),
			cmos_sensor(0x7a,0x00),
			cmos_sensor(0x78,0xc0),
			cmos_sensor(0x79,0x20),
			cmos_sensor(0x7b,0x40),
			cmos_sensor(0x7c,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_POSTERIZE
//SET_EFFECT_WHITEBOARD
static const struct sensor_cmd set_color_effect_whiteboard_reg_list[]=
{
            cmos_sensor(0x41,0x00),           //RELIEVOS:  // CARVING
            cmos_sensor(0x40,0x3e),
            cmos_sensor(0x42,0x14),
            cmos_sensor(0x47, 0x30),
            cmos_sensor(0x48,0xc2),
            cmos_sensor(0x8d,0xff),
            cmos_sensor(0x8a,0xf0),
            cmos_sensor(0x8b,0xf0),
            cmos_sensor(0x8c,0x00),
            cmos_sensor(0x50,0x08),
            cmos_sensor(0xdb,0x50),
            cmos_sensor(0xb0,0xff),
            cmos_sensor(0x77,0xab),
            cmos_sensor(0xa1,0xff),
            cmos_sensor(0x7a,0x7f),
            cmos_sensor(0x78,0x00),
            cmos_sensor(0x79,0x00),
            cmos_sensor(0x7b,0x40),
            cmos_sensor(0x7c,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
            
};
//SET_EFFECT_BLACKBOARD
static const struct sensor_cmd set_color_effect_blackboard_reg_list[]=
{
            cmos_sensor(0x41,0x00),           
            cmos_sensor(0x40,0x3e),
            cmos_sensor(0x42,0x14),
           cmos_sensor(0x47, 0x30),
            cmos_sensor(0x48,0xc2),
            cmos_sensor(0x8d,0xff),
            cmos_sensor(0x8a,0xf0),
            cmos_sensor(0x8b,0xf0),
            cmos_sensor(0x8c,0x00),
            cmos_sensor(0x50,0x08),
            cmos_sensor(0xdb,0x46),
            cmos_sensor(0xb0,0xff),
            cmos_sensor(0x77,0xa8),
            cmos_sensor(0xa1,0xff),
            cmos_sensor(0x7a,0x98),
            cmos_sensor(0x78,0x00),
            cmos_sensor(0x79,0x00),
            cmos_sensor(0x7b,0x40),
            cmos_sensor(0x7c,0x00),
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_AQUA
static const struct sensor_cmd set_color_effect_aqua_reg_list[]=
{
			cmos_sensor(0x41,0x2f),			  // SEPIAGREEN green
			cmos_sensor(0x40,0x7e),
			cmos_sensor(0x42,0x10),
			cmos_sensor(0x47, 0x30),
			cmos_sensor(0x48,0xc3),
			cmos_sensor(0x8a,0x60),
			cmos_sensor(0x8b,0x60),
			cmos_sensor(0x8c,0x07),
			cmos_sensor(0x50,0x0c),
			cmos_sensor(0x77,0x80),
			cmos_sensor(0xa1,0x40),
			cmos_sensor(0x7a,0x00),
			cmos_sensor(0x78,0xc0),
			cmos_sensor(0x79,0xc0),
			cmos_sensor(0x7b,0x40),
			cmos_sensor(0x7c,0x00),
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
					
		cmos_sensor(0x01,0x32), //normal preview  50hz  24M
		cmos_sensor(0x02,0x70), 
		cmos_sensor(0x10,0x01),
		cmos_sensor(0xd6,0x78), 
		cmos_sensor(0x28,0x02), 
		cmos_sensor(0x29,0x58), 
		cmos_sensor(0x2a,0x02), 
		cmos_sensor(0x2b,0x58), 
		cmos_sensor(0x2c,0x02), 
		cmos_sensor(0x2d,0x58), 
		cmos_sensor(0x2e,0x04), 
		cmos_sensor(0x2f,0xb0), 
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd set_antibanding_60hz_reg_list[]=
{
    /* 60 Hz */     
					
		cmos_sensor(0x01,0x32), 
		cmos_sensor(0x02,0x70), 
		cmos_sensor(0x10,0x01),
		cmos_sensor(0xd6,0x64), 
		cmos_sensor(0x28,0x02), 
		cmos_sensor(0x29,0x58), 
		cmos_sensor(0x2a,0x02), 
		cmos_sensor(0x2b,0x58), 
		cmos_sensor(0x2c,0x02), 
		cmos_sensor(0x2d,0x58), 
		cmos_sensor(0x2e,0x03), 
		cmos_sensor(0x2f,0xe8), 
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

const struct sensor_op_list * gc0307_get_op_table(int op)
{
    if(op<SENSOR_OP_NUM)
        return GET_OP_TABLE(op);
        
    return NULL;
}

extern struct cam_interface gc0307_interface;

struct cam_interface *gc0307_detect(int * invvsync)
{
    unsigned char id;

    if(cam_reg_read(GC0307_I2C_ADDR,0x00,&id,1)==0){
        printk("check camara(gc0307 id 0x99) 0x%x\n",id);
        if(id==0x99){
            printk("Found Camara GC0307 ID[0x%x]\n",id);
            if(invvsync)
                *invvsync=1;
            return &gc0307_interface;
        }
    }
   
    return NULL;
}


int  gc0307_init(void)
{
    int ret;

    cam_power_set(CAMARA_POWER_ON);

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_INIT_SENSOR);
    
    return ret;
}

int  gc0307_switch_low_svga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA);
    return ret;
}
int  gc0307_switch_high_svga(void){
    int ret;

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA);
    return ret;
}
int  gc0307_switch_high_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA);
    return ret;
}
int  gc0307_switch_upmid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA);
    return ret;
}
int  gc0307_switch_mid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_MID_XUGA);
    return ret;
}
int  gc0307_switch_low_xuga(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA);
    return ret;
}
int  gc0307_to_high_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_HIGH_PREVIEW);
    return ret;
}
int  gc0307_to_low_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_LOW_PREVIEW);
    return ret;
}

//int  cam_switch_low_xuga(void){return -1;}
int  gc0307_close(void)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_CLOSE_SENSOR);
    
    cam_power_set(CAMARA_POWER_OFF);

    return ret;
}
//-----------------------------------
//      SENSOR_SET_WHITE_BALANCE
//-----------------------------------
int  gc0307_set_wb(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_WHITE_BALANCE,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_COLOR_EFFECT
//-----------------------------------
int  gc0307_set_effect(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_COLOR_EFFECT,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------
int  gc0307_set_brightness(unsigned char value)
{
    struct sensor_cmd cmd;
    int ret;
    
    cmd.reg = 0x03;
    cmd.val = value;
    ret=cam_reg_write(GC0307_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));

    if(ret)
        return ret;

    cmd.reg = 0x40;
    cmd.val = value;
    ret=cam_reg_write(GC0307_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));
    
    return ret;
}

//-----------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------
int  gc0307_night_mode_on(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_NIGHT);
    return ret;
}                                                    
int  gc0307_night_mode_off(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_AUTO);
    return ret;
}

int  gc0307_dump_reg(void)
{
    unsigned char reg;
    unsigned char val;
    int ret;
    
    for(reg=0;reg<0xf0;reg++){

        ret = cam_reg_read(GC0307_I2C_ADDR,reg,&val,1);
        if(ret != 0){
            printk("reg 0x%x  failed\n",reg);
        }else{
            printk("0x%x 0x%x\n",reg,val);
        }
    }

    return 0;
}

struct device_info gc0307_info={
    GC0307_I2C_ADDR,
};


struct cam_interface gc0307_interface={
    &gc0307_info,
    gc0307_get_op_table,
    gc0307_init,
    gc0307_switch_low_svga,
    gc0307_switch_high_svga,
    gc0307_switch_high_xuga,
    gc0307_switch_upmid_xuga,
    gc0307_switch_mid_xuga,
    gc0307_switch_low_xuga,
    gc0307_to_high_preview,
    gc0307_to_low_preview,
    gc0307_close,
    gc0307_set_wb,
    gc0307_set_effect,
    gc0307_set_brightness,
    gc0307_night_mode_on,                                                    
    gc0307_night_mode_off,
    gc0307_dump_reg,
};

