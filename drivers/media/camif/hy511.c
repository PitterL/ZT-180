

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

#include "hy511.h"

//-----------------------------
//      SENSOR_SET_MODE
//-----------------------------

//INIT_SENSOR
static const struct sensor_cmd set_mode_init_reg_list[]=
{
    /////// Start Sleep ///////
    cmos_sensor(0x01, 0xf9), //sleep on
    cmos_sensor(0x08, 0x0f), //Hi-Z on
    cmos_sensor(0x01, 0xf8), //sleep off

    cmos_sensor(0x03, 0x00), // Dummy 750us START
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00), // Dummy 750us END

    cmos_sensor(0x0e, 0x03), //PLL On
    cmos_sensor(0x0e, 0x73), //PLLx2

    cmos_sensor(0x03, 0x00), // Dummy 750us START
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00), // Dummy 750us END

    cmos_sensor(0x0e, 0x00), //PLL off
    cmos_sensor(0x01, 0xf1), //sleep on
    cmos_sensor(0x08, 0x00), //Hi-Z off

    cmos_sensor(0x01, 0xf3),
    cmos_sensor(0x01, 0xf1),

    // PAGE 20
    cmos_sensor(0x03, 0x20), //page 20
    cmos_sensor(0x10, 0x1c), //ae off

    // PAGE 22
    cmos_sensor(0x03, 0x22), //page 22
    cmos_sensor(0x10, 0x69), //awb off


    //Initial Start
    /////// PAGE 0 START ///////
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x11), // Sub1/2_Preview2 Mode_H binning
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    cmos_sensor(0x24, 0x04),
    cmos_sensor(0x25, 0xb0),
    cmos_sensor(0x26, 0x06),
    cmos_sensor(0x27, 0x40), // WINROW END

    cmos_sensor(0x40, 0x01), //Hblank 408
    cmos_sensor(0x41, 0x68), 
    cmos_sensor(0x42, 0x00), //Vblank 20
    cmos_sensor(0x43, 0x14),

    cmos_sensor(0x45, 0x04),
    cmos_sensor(0x46, 0x18),
    cmos_sensor(0x47, 0xd8),

    //BLC
    cmos_sensor(0x80, 0x2e),
    cmos_sensor(0x81, 0x7e),
    cmos_sensor(0x82, 0x90),
    cmos_sensor(0x83, 0x00),
    cmos_sensor(0x84, 0x0c),
    cmos_sensor(0x85, 0x00),
    cmos_sensor(0x90, 0x14), //BLC_TIME_TH_ON
    cmos_sensor(0x91, 0x14), //BLC_TIME_TH_OFF 
    cmos_sensor(0x92, 0xd8), //BLC_AG_TH_ON
    cmos_sensor(0x93, 0xd0), //BLC_AG_TH_OFF
    cmos_sensor(0x94, 0x75),
    cmos_sensor(0x95, 0x70),
    cmos_sensor(0x96, 0xdc),
    cmos_sensor(0x97, 0xfe),
    cmos_sensor(0x98, 0x38),

    //OutDoor  BLC
    cmos_sensor(0x99, 0x43),
    cmos_sensor(0x9a, 0x43),
    cmos_sensor(0x9b, 0x43),
    cmos_sensor(0x9c, 0x43),

    //Dark BLC
    cmos_sensor(0xa0, 0x00),
    cmos_sensor(0xa2, 0x00),
    cmos_sensor(0xa4, 0x00),
    cmos_sensor(0xa6, 0x00),

    //Normal BLC
    cmos_sensor(0xa8, 0x43),
    cmos_sensor(0xaa, 0x43),
    cmos_sensor(0xac, 0x43),
    cmos_sensor(0xae, 0x43),

    cmos_sensor(0x03, 0x02), //Page 02
    cmos_sensor(0x10, 0x00), //Mode_test
    cmos_sensor(0x11, 0x00), //Mode_dead_test
    cmos_sensor(0x12, 0x03), //pwr_ctl_ctl1
    cmos_sensor(0x13, 0x03), //Mode_ana_test
    cmos_sensor(0x14, 0x00), //mode_memory
    cmos_sensor(0x16, 0x00), //dcdc_ctl1
    cmos_sensor(0x17, 0x8c), //dcdc_ctl2
    cmos_sensor(0x18, 0x4C), //analog_func1
    cmos_sensor(0x19, 0x00), //analog_func2
    cmos_sensor(0x1a, 0x39), //analog_func3
    cmos_sensor(0x1b, 0x00), //analog_func4
    cmos_sensor(0x1c, 0x09), //dcdc_ctl3
    cmos_sensor(0x1d, 0x40), //dcdc_ctl4
    cmos_sensor(0x1e, 0x30), //analog_func7
    cmos_sensor(0x1f, 0x10), //analog_func8
    cmos_sensor(0x20, 0x77), //pixel bias
    cmos_sensor(0x21, 0xde), //adc,asp bias
    cmos_sensor(0x22, 0xa7), //main,bus bias
    cmos_sensor(0x23, 0x30), //clamp
    cmos_sensor(0x24, 0x4a),		
    cmos_sensor(0x25, 0x10),		
    cmos_sensor(0x27, 0x3c),		
    cmos_sensor(0x28, 0x00),		
    cmos_sensor(0x29, 0x0c),		
    cmos_sensor(0x2a, 0x80),		
    cmos_sensor(0x2b, 0x80),		
    cmos_sensor(0x2c, 0x02),		
    cmos_sensor(0x2d, 0xa0),		
    cmos_sensor(0x2e, 0x11),		
    cmos_sensor(0x2f, 0xa1),		
    cmos_sensor(0x30, 0x05), //swap_ctl
    cmos_sensor(0x31, 0x99),		
    cmos_sensor(0x32, 0x00),		
    cmos_sensor(0x33, 0x00),		
    cmos_sensor(0x34, 0x22),		
    cmos_sensor(0x38, 0x88),		
    cmos_sensor(0x39, 0x88),		
    cmos_sensor(0x50, 0x20),		
    cmos_sensor(0x51, 0x00),		
    cmos_sensor(0x52, 0x01),		
    cmos_sensor(0x53, 0xc1),		
    cmos_sensor(0x54, 0x10),		
    cmos_sensor(0x55, 0x1c),		
    cmos_sensor(0x56, 0x11),		
    cmos_sensor(0x58, 0x10),		
    cmos_sensor(0x59, 0x0e),		
    cmos_sensor(0x5d, 0xa2),		
    cmos_sensor(0x5e, 0x5a),		
    cmos_sensor(0x60, 0x87),		
    cmos_sensor(0x61, 0x99),		
    cmos_sensor(0x62, 0x88),		
    cmos_sensor(0x63, 0x97),		
    cmos_sensor(0x64, 0x88),		
    cmos_sensor(0x65, 0x97),		
    cmos_sensor(0x67, 0x0c),		
    cmos_sensor(0x68, 0x0c),		
    cmos_sensor(0x69, 0x0c),		
    cmos_sensor(0x6a, 0xb4),		
    cmos_sensor(0x6b, 0xc4),		
    cmos_sensor(0x6c, 0xb5),		
    cmos_sensor(0x6d, 0xc2),		
    cmos_sensor(0x6e, 0xb5),		
    cmos_sensor(0x6f, 0xc0),		
    cmos_sensor(0x70, 0xb6),		
    cmos_sensor(0x71, 0xb8),		
    cmos_sensor(0x72, 0x89),		
    cmos_sensor(0x73, 0x96),		
    cmos_sensor(0x74, 0x89),		
    cmos_sensor(0x75, 0x96),		
    cmos_sensor(0x76, 0x89),		
    cmos_sensor(0x77, 0x96),		
    cmos_sensor(0x7c, 0x85),		
    cmos_sensor(0x7d, 0xaf),		
    cmos_sensor(0x80, 0x01),		
    cmos_sensor(0x81, 0x7f),		
    cmos_sensor(0x82, 0x13), //rx_on1_read
    cmos_sensor(0x83, 0x24),		
    cmos_sensor(0x84, 0x7D),		
    cmos_sensor(0x85, 0x81),		
    cmos_sensor(0x86, 0x7D),		
    cmos_sensor(0x87, 0x81),		
    cmos_sensor(0x88, 0xab),		
    cmos_sensor(0x89, 0xbc),		
    cmos_sensor(0x8a, 0xac),		
    cmos_sensor(0x8b, 0xba),		
    cmos_sensor(0x8c, 0xad),		
    cmos_sensor(0x8d, 0xb8),		
    cmos_sensor(0x8e, 0xae),		
    cmos_sensor(0x8f, 0xb2),		
    cmos_sensor(0x90, 0xb3),		
    cmos_sensor(0x91, 0xb7),		
    cmos_sensor(0x92, 0x48),		
    cmos_sensor(0x93, 0x54),		
    cmos_sensor(0x94, 0x7D),		
    cmos_sensor(0x95, 0x81),		
    cmos_sensor(0x96, 0x7D),		
    cmos_sensor(0x97, 0x81),		
    cmos_sensor(0xa0, 0x02),		
    cmos_sensor(0xa1, 0x7B),		
    cmos_sensor(0xa2, 0x02),		
    cmos_sensor(0xa3, 0x7B),		
    cmos_sensor(0xa4, 0x7B),		
    cmos_sensor(0xa5, 0x02),		
    cmos_sensor(0xa6, 0x7B),		
    cmos_sensor(0xa7, 0x02),		
    cmos_sensor(0xa8, 0x85),		
    cmos_sensor(0xa9, 0x8C),		
    cmos_sensor(0xaa, 0x85),		
    cmos_sensor(0xab, 0x8C),		
    cmos_sensor(0xac, 0x10), //Rx_pwr_off1_read
    cmos_sensor(0xad, 0x16), //Rx_pwr_on1_read
    cmos_sensor(0xae, 0x10), //Rx_pwr_off2_read
    cmos_sensor(0xaf, 0x16), //Rx_pwr_on1_read
    cmos_sensor(0xb0, 0x99),		
    cmos_sensor(0xb1, 0xA3),		
    cmos_sensor(0xb2, 0xA4),		
    cmos_sensor(0xb3, 0xAE),		
    cmos_sensor(0xb4, 0x9B),		
    cmos_sensor(0xb5, 0xA2),		
    cmos_sensor(0xb6, 0xA6),		
    cmos_sensor(0xb7, 0xAC),		
    cmos_sensor(0xb8, 0x9B),		
    cmos_sensor(0xb9, 0x9F),		
    cmos_sensor(0xba, 0xA6),		
    cmos_sensor(0xbb, 0xAA),		
    cmos_sensor(0xbc, 0x9B),		
    cmos_sensor(0xbd, 0x9F),		
    cmos_sensor(0xbe, 0xA6),		
    cmos_sensor(0xbf, 0xaa),		
    cmos_sensor(0xc4, 0x2c),		
    cmos_sensor(0xc5, 0x43),		
    cmos_sensor(0xc6, 0x63),		
    cmos_sensor(0xc7, 0x79),		
    cmos_sensor(0xc8, 0x2d),		
    cmos_sensor(0xc9, 0x42),		
    cmos_sensor(0xca, 0x2d),		
    cmos_sensor(0xcb, 0x42),		
    cmos_sensor(0xcc, 0x64),		
    cmos_sensor(0xcd, 0x78),		
    cmos_sensor(0xce, 0x64),		
    cmos_sensor(0xcf, 0x78),		
    cmos_sensor(0xd0, 0x0a),		
    cmos_sensor(0xd1, 0x09),		
    cmos_sensor(0xd2, 0x20),		
    cmos_sensor(0xd3, 0x00),	
    	
    cmos_sensor(0xd4, 0x0a),		
    cmos_sensor(0xd5, 0x0a),		
    cmos_sensor(0xd6, 0xb8),		
    cmos_sensor(0xd7, 0xb0),
    		
    cmos_sensor(0xe0, 0xc4),		
    cmos_sensor(0xe1, 0xc4),		
    cmos_sensor(0xe2, 0xc4),		
    cmos_sensor(0xe3, 0xc4),		
    cmos_sensor(0xe4, 0x00),		
    cmos_sensor(0xe8, 0x80),		
    cmos_sensor(0xe9, 0x40),		
    cmos_sensor(0xea, 0x7f),		
    cmos_sensor(0xf0, 0x01), //sram1_cfg
    cmos_sensor(0xf1, 0x01), //sram2_cfg
    cmos_sensor(0xf2, 0x01), //sram3_cfg
    cmos_sensor(0xf3, 0x01), //sram4_cfg
    cmos_sensor(0xf4, 0x01), //sram5_cfg


    /////// PAGE 3 ///////
    cmos_sensor(0x03, 0x03),
    cmos_sensor(0x10, 0x10),

    /////// PAGE 10 START ///////
    cmos_sensor(0x03, 0x10),
    cmos_sensor(0x10, /*0x01*/0x03), // CrYCbY // For Demoset 0x03
    cmos_sensor(0x12, 0x30),
    cmos_sensor(0x13, 0x0a), // contrast on
    cmos_sensor(0x20, 0x00),

    cmos_sensor(0x30, 0x00),
    cmos_sensor(0x31, 0x00),
    cmos_sensor(0x32, 0x00),
    cmos_sensor(0x33, 0x00),

    cmos_sensor(0x34, 0x30),
    cmos_sensor(0x35, 0x00),
    cmos_sensor(0x36, 0x00),
    cmos_sensor(0x38, 0x00),
    cmos_sensor(0x3e, 0x58),
    cmos_sensor(0x3f, 0x00),

    cmos_sensor(0x40, 0x06), // YOFS 80
    cmos_sensor(0x41, 0x10), // DYOFS
    cmos_sensor(0x48, 0x85), // Contrast
    cmos_sensor(0x50, 0x90), // Contrast

    cmos_sensor(0x60, 0x67),
    cmos_sensor(0x61, 0x7c), //7e //8e //88 //80
    cmos_sensor(0x62, 0x7c), //7e //8e //88 //80
    cmos_sensor(0x63, 0x50), //Double_AG 50->30
    cmos_sensor(0x64, 0x41),

    cmos_sensor(0x66, 0x42),
    cmos_sensor(0x67, 0x20),

    cmos_sensor(0x6a, 0x80), //8a
    cmos_sensor(0x6b, 0x84), //74
    cmos_sensor(0x6c, 0x80), //7e //7a
    cmos_sensor(0x6d, 0x80), //8e

    //Don't touch//////////////////////////
    //cmos_sensor(0x72, 0x84),
    //cmos_sensor(0x76, 0x19),
    //cmos_sensor(0x73, 0x70),
    //cmos_sensor(0x74, 0x68),
    //cmos_sensor(0x75, 0x60), // white protection ON
    //cmos_sensor(0x77, 0x0e), //08 //0a
    //cmos_sensor(0x78, 0x2a), //20
    //cmos_sensor(0x79, 0x08),
    ////////////////////////////////////////

    /////// PAGE 11 START ///////
    cmos_sensor(0x03, 0x11),
    cmos_sensor(0x10, 0x7f),
    cmos_sensor(0x11, 0x40),
    cmos_sensor(0x12, 0x0a), // Blue Max-Filter Delete
    cmos_sensor(0x13, 0xbb),

    cmos_sensor(0x26, 0x31), // Double_AG 31->20
    cmos_sensor(0x27, 0x34), // Double_AG 34->22
    cmos_sensor(0x28, 0x0f),
    cmos_sensor(0x29, 0x10),
    cmos_sensor(0x2b, 0x30),
    cmos_sensor(0x2c, 0x32),

    //Out2 D-LPF th
    cmos_sensor(0x30, 0x70),
    cmos_sensor(0x31, 0x10),
    cmos_sensor(0x32, 0x58),
    cmos_sensor(0x33, 0x09),
    cmos_sensor(0x34, 0x06),
    cmos_sensor(0x35, 0x03),

    //Out1 D-LPF th
    cmos_sensor(0x36, 0x70),
    cmos_sensor(0x37, 0x18),
    cmos_sensor(0x38, 0x58),
    cmos_sensor(0x39, 0x09),
    cmos_sensor(0x3a, 0x06),
    cmos_sensor(0x3b, 0x03),

    //Indoor D-LPF th
    cmos_sensor(0x3c, 0x80),
    cmos_sensor(0x3d, 0x18),
    cmos_sensor(0x3e, 0x83), //80
    cmos_sensor(0x3f, 0x0c),
    cmos_sensor(0x40, 0x06),//adam 0x03
    cmos_sensor(0x41, 0x06),

    cmos_sensor(0x42, 0x80),
    cmos_sensor(0x43, 0x18),
    cmos_sensor(0x44, 0x83), //80
    cmos_sensor(0x45, 0x12),
    cmos_sensor(0x46, 0x10),
    cmos_sensor(0x47, 0x10),

    cmos_sensor(0x48, 0x90),
    cmos_sensor(0x49, 0x40),
    cmos_sensor(0x4a, 0x80),
    cmos_sensor(0x4b, 0x13),
    cmos_sensor(0x4c, 0x10),
    cmos_sensor(0x4d, 0x11),

    cmos_sensor(0x4e, 0x80),
    cmos_sensor(0x4f, 0x30),
    cmos_sensor(0x50, 0x80),
    cmos_sensor(0x51, 0x13),
    cmos_sensor(0x52, 0x10),
    cmos_sensor(0x53, 0x13),

    cmos_sensor(0x54, 0x11),
    cmos_sensor(0x55, 0x17),
    cmos_sensor(0x56, 0x20),
    cmos_sensor(0x57, 0x01),
    cmos_sensor(0x58, 0x00),
    cmos_sensor(0x59, 0x00),

    cmos_sensor(0x5a, 0x1f), //18
    cmos_sensor(0x5b, 0x00),
    cmos_sensor(0x5c, 0x00),

    cmos_sensor(0x60, 0x3f),
    cmos_sensor(0x62, 0x60),
    cmos_sensor(0x70, 0x06),

    /////// PAGE 12 START ///////
    cmos_sensor(0x03, 0x12),
    cmos_sensor(0x20, 0x0f),
    cmos_sensor(0x21, 0x0f),

    cmos_sensor(0x25, 0x00), //0x30

    cmos_sensor(0x28, 0x00),
    cmos_sensor(0x29, 0x00),
    cmos_sensor(0x2a, 0x00),

    cmos_sensor(0x30, 0x50),
    cmos_sensor(0x31, 0x18),
    cmos_sensor(0x32, 0x32),
    cmos_sensor(0x33, 0x40),
    cmos_sensor(0x34, 0x50),
    cmos_sensor(0x35, 0x70),
    cmos_sensor(0x36, 0xa0),

    //Out2 th
    cmos_sensor(0x40, 0xa0),
    cmos_sensor(0x41, 0x40),
    cmos_sensor(0x42, 0xa0),
    cmos_sensor(0x43, 0x90),
    cmos_sensor(0x44, 0x90),
    cmos_sensor(0x45, 0x80),

    //Out1 th
    cmos_sensor(0x46, 0xb0),
    cmos_sensor(0x47, 0x55),
    cmos_sensor(0x48, 0xa0),
    cmos_sensor(0x49, 0x90),
    cmos_sensor(0x4a, 0x90),
    cmos_sensor(0x4b, 0x80),

    //Indoor th
    cmos_sensor(0x4c, 0xb0),
    cmos_sensor(0x4d, 0x40),
    cmos_sensor(0x4e, 0x90),
    cmos_sensor(0x4f, 0x90),
    cmos_sensor(0x50, 0xa0),
    cmos_sensor(0x51, 0x28),//60

    //Dark1 th
    cmos_sensor(0x52, 0xb0),
    cmos_sensor(0x53, 0x60),
    cmos_sensor(0x54, 0xc0),
    cmos_sensor(0x55, 0xc0),
    cmos_sensor(0x56, 0x80),
    cmos_sensor(0x57, 0x38),

    //Dark2 th
    cmos_sensor(0x58, 0x90),
    cmos_sensor(0x59, 0x40),
    cmos_sensor(0x5a, 0xd0),
    cmos_sensor(0x5b, 0xd0),
    cmos_sensor(0x5c, 0xe0),
    cmos_sensor(0x5d, 0x50),

    //Dark3 th
    cmos_sensor(0x5e, 0x88),
    cmos_sensor(0x5f, 0x40),
    cmos_sensor(0x60, 0xe0),
    cmos_sensor(0x61, 0xe0),
    cmos_sensor(0x62, 0xe0),
    cmos_sensor(0x63, 0x80),

    cmos_sensor(0x70, 0x15),
    cmos_sensor(0x71, 0x01), //Don't Touch register

    cmos_sensor(0x72, 0x18),
    cmos_sensor(0x73, 0x01), //Don't Touch register

    cmos_sensor(0x74, 0x25),
    cmos_sensor(0x75, 0x15),

    cmos_sensor(0x80, 0x20),
    cmos_sensor(0x81, 0x40),
    cmos_sensor(0x82, 0x65),
    cmos_sensor(0x85, 0x1a),
    cmos_sensor(0x88, 0x00),
    cmos_sensor(0x89, 0x00),
    cmos_sensor(0x90, 0x5d), //For Preview

    //Dont Touch register
    cmos_sensor(0xD0, 0x0c),
    cmos_sensor(0xD1, 0x80),
    cmos_sensor(0xD2, 0x67),
    cmos_sensor(0xD3, 0x00),
    cmos_sensor(0xD4, 0x00),
    cmos_sensor(0xD5, 0x02),
    cmos_sensor(0xD6, 0xff),
    cmos_sensor(0xD7, 0x18),
    //End
    cmos_sensor(0x3b, 0x06),
    cmos_sensor(0x3c, 0x06),

    cmos_sensor(0xc5, 0x00),//55->48
    cmos_sensor(0xc6, 0x00),//48->40

    /////// PAGE 13 START ///////
    cmos_sensor(0x03, 0x13),
    //Edge
    cmos_sensor(0x10, 0xcb),
    cmos_sensor(0x11, 0x7b),
    cmos_sensor(0x12, 0x07),
    cmos_sensor(0x14, 0x00),

    cmos_sensor(0x20, 0x15),
    cmos_sensor(0x21, 0x13),
    cmos_sensor(0x22, 0x33),
    cmos_sensor(0x23, 0x05),
    cmos_sensor(0x24, 0x09),

    cmos_sensor(0x25, 0x0a),

    cmos_sensor(0x26, 0x18),
    cmos_sensor(0x27, 0x30),
    cmos_sensor(0x29, 0x12),
    cmos_sensor(0x2a, 0x50),

    //Low clip th
    cmos_sensor(0x2b, 0x02),
    cmos_sensor(0x2c, 0x02),
    cmos_sensor(0x25, 0x06),
    cmos_sensor(0x2d, 0x0c),
    cmos_sensor(0x2e, 0x12),
    cmos_sensor(0x2f, 0x12),

    //Out2 Edge
    cmos_sensor(0x50, 0x10),
    cmos_sensor(0x51, 0x14),
    cmos_sensor(0x52, 0x12),
    cmos_sensor(0x53, 0x0c),
    cmos_sensor(0x54, 0x0f),
    cmos_sensor(0x55, 0x0c),

    //Out1 Edge
    cmos_sensor(0x56, 0x10),
    cmos_sensor(0x57, 0x13),
    cmos_sensor(0x58, 0x12),
    cmos_sensor(0x59, 0x0c),
    cmos_sensor(0x5a, 0x0f),
    cmos_sensor(0x5b, 0x0c),

    //Indoor Edge
    cmos_sensor(0x5c, 0x0a),
    cmos_sensor(0x5d, 0x0b),
    cmos_sensor(0x5e, 0x0a),
    cmos_sensor(0x5f, 0x08),
    cmos_sensor(0x60, 0x09),
    cmos_sensor(0x61, 0x08),

    //Dark1 Edge
    cmos_sensor(0x62, 0x08),
    cmos_sensor(0x63, 0x08),
    cmos_sensor(0x64, 0x08),
    cmos_sensor(0x65, 0x06),
    cmos_sensor(0x66, 0x06),
    cmos_sensor(0x67, 0x06),

    //Dark2 Edge
    cmos_sensor(0x68, 0x07),
    cmos_sensor(0x69, 0x07),
    cmos_sensor(0x6a, 0x07),
    cmos_sensor(0x6b, 0x05),
    cmos_sensor(0x6c, 0x05),
    cmos_sensor(0x6d, 0x05),

    //Dark3 Edge
    cmos_sensor(0x6e, 0x07),
    cmos_sensor(0x6f, 0x07),
    cmos_sensor(0x70, 0x07),
    cmos_sensor(0x71, 0x05),
    cmos_sensor(0x72, 0x05),
    cmos_sensor(0x73, 0x05),

    //2DY
    cmos_sensor(0x80, 0xfd),
    cmos_sensor(0x81, 0x1f),
    cmos_sensor(0x82, 0x05),
    cmos_sensor(0x83, 0x31),

    cmos_sensor(0x90, 0x05),
    cmos_sensor(0x91, 0x05),
    cmos_sensor(0x92, 0x33),
    cmos_sensor(0x93, 0x30),
    cmos_sensor(0x94, 0x03),
    cmos_sensor(0x95, 0x14),
    cmos_sensor(0x97, 0x20),
    cmos_sensor(0x99, 0x20),

    cmos_sensor(0xa0, 0x01),
    cmos_sensor(0xa1, 0x02),
    cmos_sensor(0xa2, 0x01),
    cmos_sensor(0xa3, 0x02),
    cmos_sensor(0xa4, 0x05),
    cmos_sensor(0xa5, 0x05),
    cmos_sensor(0xa6, 0x07),
    cmos_sensor(0xa7, 0x08),
    cmos_sensor(0xa8, 0x07),
    cmos_sensor(0xa9, 0x08),
    cmos_sensor(0xaa, 0x07),
    cmos_sensor(0xab, 0x08),

    //Out2 
    cmos_sensor(0xb0, 0x22),
    cmos_sensor(0xb1, 0x2a),
    cmos_sensor(0xb2, 0x28),
    cmos_sensor(0xb3, 0x22),
    cmos_sensor(0xb4, 0x2a),
    cmos_sensor(0xb5, 0x28),

    //Out1 
    cmos_sensor(0xb6, 0x22),
    cmos_sensor(0xb7, 0x2a),
    cmos_sensor(0xb8, 0x28),
    cmos_sensor(0xb9, 0x22),
    cmos_sensor(0xba, 0x2a),
    cmos_sensor(0xbb, 0x28),

    //Indoor 
    cmos_sensor(0xbc, 0x25),
    cmos_sensor(0xbd, 0x2a),
    cmos_sensor(0xbe, 0x27),
    cmos_sensor(0xbf, 0x25),
    cmos_sensor(0xc0, 0x2a),
    cmos_sensor(0xc1, 0x27),

    //Dark1
    cmos_sensor(0xc2, 0x1e),
    cmos_sensor(0xc3, 0x24),
    cmos_sensor(0xc4, 0x20),
    cmos_sensor(0xc5, 0x1e),
    cmos_sensor(0xc6, 0x24),
    cmos_sensor(0xc7, 0x20),

    //Dark2
    cmos_sensor(0xc8, 0x18),
    cmos_sensor(0xc9, 0x20),
    cmos_sensor(0xca, 0x1e),
    cmos_sensor(0xcb, 0x18),
    cmos_sensor(0xcc, 0x20),
    cmos_sensor(0xcd, 0x1e),

    //Dark3 
    cmos_sensor(0xce, 0x18),
    cmos_sensor(0xcf, 0x20),
    cmos_sensor(0xd0, 0x1e),
    cmos_sensor(0xd1, 0x18),
    cmos_sensor(0xd2, 0x20),
    cmos_sensor(0xd3, 0x1e),

    /////// PAGE 14 START ///////
    cmos_sensor(0x03, 0x14),
    cmos_sensor(0x10, 0x11),

    cmos_sensor(0x14, 0x80), // GX
    cmos_sensor(0x15, 0x80), // GY
    cmos_sensor(0x16, 0x80), // RX
    cmos_sensor(0x17, 0x80), // RY
    cmos_sensor(0x18, 0x80), // BX
    cmos_sensor(0x19, 0x80), // BY

    cmos_sensor(0x20, 0x60), //X 60 //a0
    cmos_sensor(0x21, 0x80), //Y

    cmos_sensor(0x22, 0x80),
    cmos_sensor(0x23, 0x80),
    cmos_sensor(0x24, 0x80),

    cmos_sensor(0x30, 0xc8),
    cmos_sensor(0x31, 0x2b),
    cmos_sensor(0x32, 0x00),
    cmos_sensor(0x33, 0x00),
    cmos_sensor(0x34, 0x90),

    cmos_sensor(0x40, 0x48), //31
    cmos_sensor(0x50, 0x34), //23 //32
    cmos_sensor(0x60, 0x29), //1a //27
    cmos_sensor(0x70, 0x34), //23 //32

    /////// PAGE 15 START ///////
    cmos_sensor(0x03, 0x15),
    cmos_sensor(0x10, 0x0f),

    //Rstep H 16
    //Rstep L 14
    cmos_sensor(0x14, 0x42), //CMCOFSGH_Day //4c
    cmos_sensor(0x15, 0x32), //CMCOFSGM_CWF //3c
    cmos_sensor(0x16, 0x24), //CMCOFSGL_A //2e
    cmos_sensor(0x17, 0x2f), //CMC SIGN

    //CMC_Default_CWF
    cmos_sensor(0x30, 0x8f),
    cmos_sensor(0x31, 0x59),
    cmos_sensor(0x32, 0x0a),
    cmos_sensor(0x33, 0x15),
    cmos_sensor(0x34, 0x5b),
    cmos_sensor(0x35, 0x06),
    cmos_sensor(0x36, 0x07),
    cmos_sensor(0x37, 0x40),
    cmos_sensor(0x38, 0x87), //86

    //CMC OFS L_A
    cmos_sensor(0x40, 0x92),
    cmos_sensor(0x41, 0x1b),
    cmos_sensor(0x42, 0x89),
    cmos_sensor(0x43, 0x81),
    cmos_sensor(0x44, 0x00),
    cmos_sensor(0x45, 0x01),
    cmos_sensor(0x46, 0x89),
    cmos_sensor(0x47, 0x9e),
    cmos_sensor(0x48, 0x28),

    //cmos_sensor(0x40, 0x93),
    //cmos_sensor(0x41, 0x1c),
    //cmos_sensor(0x42, 0x89),
    //cmos_sensor(0x43, 0x82),
    //cmos_sensor(0x44, 0x01),
    //cmos_sensor(0x45, 0x01),
    //cmos_sensor(0x46, 0x8a),
    //cmos_sensor(0x47, 0x9d),
    //cmos_sensor(0x48, 0x28),

    //CMC POFS H_DAY
    cmos_sensor(0x50, 0x02),
    cmos_sensor(0x51, 0x82),
    cmos_sensor(0x52, 0x00),
    cmos_sensor(0x53, 0x07),
    cmos_sensor(0x54, 0x11),
    cmos_sensor(0x55, 0x98),
    cmos_sensor(0x56, 0x00),
    cmos_sensor(0x57, 0x0b),
    cmos_sensor(0x58, 0x8b),

    cmos_sensor(0x80, 0x03),
    cmos_sensor(0x85, 0x40),
    cmos_sensor(0x87, 0x02),
    cmos_sensor(0x88, 0x00),
    cmos_sensor(0x89, 0x00),
    cmos_sensor(0x8a, 0x00),

    /////// PAGE 16 START ///////
    cmos_sensor(0x03, 0x16),
    cmos_sensor(0x10, 0x31),
    cmos_sensor(0x18, 0x5e),// Double_AG 5e->37
    cmos_sensor(0x19, 0x5d),// Double_AG 5e->36
    cmos_sensor(0x1a, 0x0e),
    cmos_sensor(0x1b, 0x01),
    cmos_sensor(0x1c, 0xdc),
    cmos_sensor(0x1d, 0xfe),

    //GMA Default
    /*
    cmos_sensor(0x30, 0x00),
    cmos_sensor(0x31, 0x0a),
    cmos_sensor(0x32, 0x1f),
    cmos_sensor(0x33, 0x33),
    cmos_sensor(0x34, 0x53),
    cmos_sensor(0x35, 0x6c),
    cmos_sensor(0x36, 0x81),
    cmos_sensor(0x37, 0x94),
    cmos_sensor(0x38, 0xa4),
    cmos_sensor(0x39, 0xb3),
    cmos_sensor(0x3a, 0xc0),
    cmos_sensor(0x3b, 0xcb),
    cmos_sensor(0x3c, 0xd5),
    cmos_sensor(0x3d, 0xde),
    cmos_sensor(0x3e, 0xe6),
    cmos_sensor(0x3f, 0xee),
    cmos_sensor(0x40, 0xf5),
    cmos_sensor(0x41, 0xfc),
    cmos_sensor(0x42, 0xff),
*/
    cmos_sensor(0x30, 0x00),
    cmos_sensor(0x31, 0x0a),
    cmos_sensor(0x32, 0x1b),
    cmos_sensor(0x33, 0x2f),
    cmos_sensor(0x34, 0x4f),
    cmos_sensor(0x35, 0x68),
    cmos_sensor(0x36, 0x7c),
    cmos_sensor(0x37, 0x90),
    cmos_sensor(0x38, 0xa3),
    cmos_sensor(0x39, 0xb3),
    cmos_sensor(0x3a, 0xc4),
    cmos_sensor(0x3b, 0xd0),
    cmos_sensor(0x3c, 0xdb),
    cmos_sensor(0x3d, 0xe4),
    cmos_sensor(0x3e, 0xec),
    cmos_sensor(0x3f, 0xef),
    cmos_sensor(0x40, 0xf4),
    cmos_sensor(0x41, 0xf6),
    cmos_sensor(0x42, 0xf7),
    
    cmos_sensor(0x50, 0x00),
    cmos_sensor(0x51, 0x09),
    cmos_sensor(0x52, 0x1f),
    cmos_sensor(0x53, 0x37),
    cmos_sensor(0x54, 0x5b),
    cmos_sensor(0x55, 0x76),
    cmos_sensor(0x56, 0x8d),
    cmos_sensor(0x57, 0xa1),
    cmos_sensor(0x58, 0xb2),
    cmos_sensor(0x59, 0xbe),
    cmos_sensor(0x5a, 0xc9),
    cmos_sensor(0x5b, 0xd2),
    cmos_sensor(0x5c, 0xdb),
    cmos_sensor(0x5d, 0xe3),
    cmos_sensor(0x5e, 0xeb),
    cmos_sensor(0x5f, 0xf0),
    cmos_sensor(0x60, 0xf5),
    cmos_sensor(0x61, 0xf7),
    cmos_sensor(0x62, 0xf8),

    cmos_sensor(0x70, 0x00),
    cmos_sensor(0x71, 0x08),
    cmos_sensor(0x72, 0x17),
    cmos_sensor(0x73, 0x2f),
    cmos_sensor(0x74, 0x53),
    cmos_sensor(0x75, 0x6c),
    cmos_sensor(0x76, 0x81),
    cmos_sensor(0x77, 0x94),
    cmos_sensor(0x78, 0xa4),
    cmos_sensor(0x79, 0xb3),
    cmos_sensor(0x7a, 0xc0),
    cmos_sensor(0x7b, 0xcb),
    cmos_sensor(0x7c, 0xd5),
    cmos_sensor(0x7d, 0xde),
    cmos_sensor(0x7e, 0xe6),
    cmos_sensor(0x7f, 0xee),
    cmos_sensor(0x80, 0xf4),
    cmos_sensor(0x81, 0xfa),
    cmos_sensor(0x82, 0xff),

    /////// PAGE 17 START ///////
    cmos_sensor(0x03, 0x17),
    cmos_sensor(0x10, 0xf7),

    cmos_sensor(0x03, 0x18),
    cmos_sensor(0x12, 0x20),
    cmos_sensor(0x10, 0x07),
    cmos_sensor(0x11, 0x00),
    cmos_sensor(0x20, 0x05),
    cmos_sensor(0x21, 0x20),
    cmos_sensor(0x22, 0x03),
    cmos_sensor(0x23, 0xd8),
    cmos_sensor(0x24, 0x00),
    cmos_sensor(0x25, 0x10),
    cmos_sensor(0x26, 0x00),
    cmos_sensor(0x27, 0x0c),
    cmos_sensor(0x28, 0x05),
    cmos_sensor(0x29, 0x10),
    cmos_sensor(0x2a, 0x03),
    cmos_sensor(0x2b, 0xcc),
    cmos_sensor(0x2c, 0x09),
    cmos_sensor(0x2d, 0xc1),
    cmos_sensor(0x2e, 0x09),
    cmos_sensor(0x2f, 0xc1),
    cmos_sensor(0x30, 0x41),

    /////// PAGE 20 START ///////
    cmos_sensor(0x03, 0x20),
    cmos_sensor(0x11, 0x1c),
    cmos_sensor(0x18, 0x30),
    cmos_sensor(0x1a, 0x08),
    cmos_sensor(0x20, 0x01), //05_lowtemp Y Mean off
    cmos_sensor(0x21, 0x30),
    cmos_sensor(0x22, 0x10),
    cmos_sensor(0x23, 0x00),
    cmos_sensor(0x24, 0x00), //Uniform Scene Off

    cmos_sensor(0x28, 0xe7),
    cmos_sensor(0x29, 0x0d), //20100305 ad->0d
    cmos_sensor(0x2a, 0xff),
    cmos_sensor(0x2b, 0x04), //f4->Adaptive off

    cmos_sensor(0x2c, 0xc2),
    cmos_sensor(0x2d, 0xcf),  //ff->AE Speed option
    cmos_sensor(0x2e, 0x33),
    cmos_sensor(0x30, 0x78), //f8
    cmos_sensor(0x32, 0x03),
    cmos_sensor(0x33, 0x2e),
    cmos_sensor(0x34, 0x30),
    cmos_sensor(0x35, 0xd4),
    cmos_sensor(0x36, 0xfe),
    cmos_sensor(0x37, 0x32),
    cmos_sensor(0x38, 0x04),

    cmos_sensor(0x39, 0x22), //AE_escapeC10
    cmos_sensor(0x3a, 0xde), //AE_escapeC11

    cmos_sensor(0x3b, 0x22), //AE_escapeC1
    cmos_sensor(0x3c, 0xde), //AE_escapeC2

    cmos_sensor(0x50, 0x45),
    cmos_sensor(0x51, 0x88),

    cmos_sensor(0x56, 0x03),
    cmos_sensor(0x57, 0xf7),
    cmos_sensor(0x58, 0x14),
    cmos_sensor(0x59, 0x88),
    cmos_sensor(0x5a, 0x04),

    //New Weight For Samsung
    //cmos_sensor(0x60, 0xaa),
    //cmos_sensor(0x61, 0xaa),
    //cmos_sensor(0x62, 0xaa),
    //cmos_sensor(0x63, 0xaa),
    //cmos_sensor(0x64, 0xaa),
    //cmos_sensor(0x65, 0xaa),
    //cmos_sensor(0x66, 0xab),
    //cmos_sensor(0x67, 0xEa),
    //cmos_sensor(0x68, 0xab),
    //cmos_sensor(0x69, 0xEa),
    //cmos_sensor(0x6a, 0xaa),
    //cmos_sensor(0x6b, 0xaa),
    //cmos_sensor(0x6c, 0xaa),
    //cmos_sensor(0x6d, 0xaa),
    //cmos_sensor(0x6e, 0xaa),
    //cmos_sensor(0x6f, 0xaa),

    cmos_sensor(0x60, 0x55), // AEWGT1
    cmos_sensor(0x61, 0x55), // AEWGT2
    cmos_sensor(0x62, 0x6a), // AEWGT3
    cmos_sensor(0x63, 0xa9), // AEWGT4
    cmos_sensor(0x64, 0x6a), // AEWGT5
    cmos_sensor(0x65, 0xa9), // AEWGT6
    cmos_sensor(0x66, 0x6a), // AEWGT7
    cmos_sensor(0x67, 0xa9), // AEWGT8
    cmos_sensor(0x68, 0x6b), // AEWGT9
    cmos_sensor(0x69, 0xe9), // AEWGT10
    cmos_sensor(0x6a, 0x6a), // AEWGT11
    cmos_sensor(0x6b, 0xa9), // AEWGT12
    cmos_sensor(0x6c, 0x6a), // AEWGT13
    cmos_sensor(0x6d, 0xa9), // AEWGT14
    cmos_sensor(0x6e, 0x55), // AEWGT15
    cmos_sensor(0x6f, 0x55), // AEWGT16

    cmos_sensor(0x70, 0x70), //6e
    cmos_sensor(0x71, 0x00), //82(+8)->+0

    // haunting control
    cmos_sensor(0x76, 0x43),
    cmos_sensor(0x77, 0xe2), //04
    cmos_sensor(0x78, 0x23), //Yth1
    cmos_sensor(0x79, 0x42), //Yth2
    cmos_sensor(0x7a, 0x23), //23
    cmos_sensor(0x7b, 0x22), //22
    cmos_sensor(0x7d, 0x23),

    cmos_sensor(0x83, 0x01), //EXP Normal 33.33 fps 
    cmos_sensor(0x84, 0x5f), 
    cmos_sensor(0x85, 0x90), 

    cmos_sensor(0x86, 0x01), //EXPMin 5859.38 fps
    cmos_sensor(0x87, 0xf4), 

    cmos_sensor(0x88, 0x06), //EXP Max 10.00 fps 
    cmos_sensor(0x89, 0x68), 
    cmos_sensor(0x8a, 0xa0), 

    cmos_sensor(0x8B, 0x75), //EXP100 
    cmos_sensor(0x8C, 0x30), 
    cmos_sensor(0x8D, 0x61), //EXP120 
    cmos_sensor(0x8E, 0xa8), 

    cmos_sensor(0x9c, 0x17), //EXP Limit 488.28 fps 
    cmos_sensor(0x9d, 0x70), 
    cmos_sensor(0x9e, 0x01), //EXP Unit 
    cmos_sensor(0x9f, 0xf4), 

    //AE_Middle Time option
    //cmos_sensor(0xa0, 0x03),
    //cmos_sensor(0xa1, 0xa9),
    //cmos_sensor(0xa2, 0x80),

    cmos_sensor(0xb0, 0x18),
    cmos_sensor(0xb1, 0x14), //ADC 400->560
    cmos_sensor(0xb2, 0xa0), //d0
    cmos_sensor(0xb3, 0x18),
    cmos_sensor(0xb4, 0x1a),
    cmos_sensor(0xb5, 0x44),
    cmos_sensor(0xb6, 0x2f),
    cmos_sensor(0xb7, 0x28),
    cmos_sensor(0xb8, 0x25),
    cmos_sensor(0xb9, 0x22),
    cmos_sensor(0xba, 0x21),
    cmos_sensor(0xbb, 0x20),
    cmos_sensor(0xbc, 0x1f),
    cmos_sensor(0xbd, 0x1f),

    //AE_Adaptive Time option
    //cmos_sensor(0xc0, 0x10),
    //cmos_sensor(0xc1, 0x2b),
    //cmos_sensor(0xc2, 0x2b),
    //cmos_sensor(0xc3, 0x2b),
    //cmos_sensor(0xc4, 0x08),

    cmos_sensor(0xc8, 0x80),
    cmos_sensor(0xc9, 0x40),

    /////// PAGE 22 START ///////
    cmos_sensor(0x03, 0x22),
    cmos_sensor(0x10, 0xfd),
    cmos_sensor(0x11, 0x2e),
    cmos_sensor(0x19, 0x01), // Low On //
    cmos_sensor(0x20, 0x30),
    cmos_sensor(0x21, 0x80),
    cmos_sensor(0x24, 0x01),
    //cmos_sensor(0x25, 0x00), //7f New Lock Cond & New light stable

    cmos_sensor(0x30, 0x80),
    cmos_sensor(0x31, 0x80),
    cmos_sensor(0x38, 0x11),
    cmos_sensor(0x39, 0x34),

    cmos_sensor(0x40, 0xf4),
    cmos_sensor(0x41, 0x55), //44
    cmos_sensor(0x42, 0x33), //43

    cmos_sensor(0x43, 0xf6),
    cmos_sensor(0x44, 0x55), //44
    cmos_sensor(0x45, 0x44), //33

    cmos_sensor(0x46, 0x00),
    cmos_sensor(0x50, 0xb2),
    cmos_sensor(0x51, 0x81),
    cmos_sensor(0x52, 0x98),

    cmos_sensor(0x80, 0x40), //3e
    cmos_sensor(0x81, 0x20),
    cmos_sensor(0x82, 0x3e),

    cmos_sensor(0x83, 0x5e), //5e
    cmos_sensor(0x84, 0x1e), //24
    cmos_sensor(0x85, 0x5e), //54 //56 //5a
    cmos_sensor(0x86, 0x22), //24 //22

    cmos_sensor(0x87, 0x49),
    cmos_sensor(0x88, 0x39),
    cmos_sensor(0x89, 0x37), //38
    cmos_sensor(0x8a, 0x28), //2a

    cmos_sensor(0x8b, 0x41), //47
    cmos_sensor(0x8c, 0x39), 
    cmos_sensor(0x8d, 0x34), 
    cmos_sensor(0x8e, 0x28), //2c

    cmos_sensor(0x8f, 0x53), //4e
    cmos_sensor(0x90, 0x52), //4d
    cmos_sensor(0x91, 0x51), //4c
    cmos_sensor(0x92, 0x4e), //4a
    cmos_sensor(0x93, 0x4a), //46
    cmos_sensor(0x94, 0x45),
    cmos_sensor(0x95, 0x3d),
    cmos_sensor(0x96, 0x31),
    cmos_sensor(0x97, 0x28),
    cmos_sensor(0x98, 0x24),
    cmos_sensor(0x99, 0x20),
    cmos_sensor(0x9a, 0x20),

    cmos_sensor(0x9b, 0x77),
    cmos_sensor(0x9c, 0x77),
    cmos_sensor(0x9d, 0x48),
    cmos_sensor(0x9e, 0x38),
    cmos_sensor(0x9f, 0x30),

    cmos_sensor(0xa0, 0x60),
    cmos_sensor(0xa1, 0x34),
    cmos_sensor(0xa2, 0x6f),
    cmos_sensor(0xa3, 0xff),

    cmos_sensor(0xa4, 0x14), //1500fps
    cmos_sensor(0xa5, 0x2c), // 700fps
    cmos_sensor(0xa6, 0xcf),

    cmos_sensor(0xad, 0x40),
    cmos_sensor(0xae, 0x4a),

    cmos_sensor(0xaf, 0x28),  // low temp Rgain
    cmos_sensor(0xb0, 0x26),  // low temp Rgain

    cmos_sensor(0xb1, 0x00), //0x20 -> 0x00 0405 modify
    cmos_sensor(0xb4, 0xea),
    cmos_sensor(0xb8, 0xa0), //a2: b-2, R+2  //b4 B-3, R+4 lowtemp
    cmos_sensor(0xb9, 0x00),

    /////// PAGE 20 ///////
    cmos_sensor(0x03, 0x20),
    cmos_sensor(0x10, 0x8c),

    // PAGE 20
    cmos_sensor(0x03, 0x20), //page 20
    cmos_sensor(0x10, 0x9c), //ae off

    // PAGE 22
    cmos_sensor(0x03, 0x22), //page 22
    cmos_sensor(0x10, 0xe9), //awb off

    // PAGE 0
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x0e, 0x03), //PLL On
    cmos_sensor(0x0e, 0x73), //PLLx2

    cmos_sensor(0x03, 0x00), // Dummy 750us
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x03, 0x00),

    cmos_sensor(0x03, 0x00), // Page 0
    cmos_sensor(0x01, 0x50), // Sleep Off 0xf8->0x50 for solve green line issue
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};


//SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA
static const struct sensor_cmd set_mode_switch_high_svga_reg_list[]=
{
    //640x480

    //record
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x11), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA
static const struct sensor_cmd set_mode_switch_low_svga_reg_list[]=
{
    //stop record

    //640x480
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x11), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

 
    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA
static const struct sensor_cmd set_mode_switch_high_xuga_reg_list[]=
{
    //1600x1200
    //record    
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x00), // Sub1/2_Preview2 Mode_H binning
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    cmos_sensor(0x24, 0x04),
    cmos_sensor(0x25, 0xb0),
    cmos_sensor(0x26, 0x06),
    cmos_sensor(0x27, 0x40), // WINROW END

    cmos_sensor(0x03, 0x18),
    cmos_sensor(0x10, 0x00),
    

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};

//SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA
static const struct sensor_cmd set_mode_switch_upmid_xuga_reg_list[]=
{
    //1280x960
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x00), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END

	
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};


//SET_MODE_SWITCH_SENSOR_TO_MID_XUGA
static const struct sensor_cmd set_mode_switch_mid_xuga_reg_list[]=
{  
    //640x480
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x10), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),
    
    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
    
};

//SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA
static const struct sensor_cmd set_mode_switch_low_xuga_reg_list[]=
{
    //320*240   
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x20), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END
    
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),

};


//SET_MODE_SENSOR_TO_HIGH_PREVIEW
static const struct sensor_cmd set_mode_high_prev_reg_list[]=
{
    //640x480
   
     //640x480
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x10), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    
    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END


    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_MODE_SENSOR_TO_LOW_PREVIEW
static const struct sensor_cmd set_mode_low_prev_reg_list[]=
{
    //320*240
    cmos_sensor(0x03, 0x00),
    cmos_sensor(0x10, 0x20), // Sub1/2_Preview2 Mode_H binning  
    cmos_sensor(0x11, 0x91),
    cmos_sensor(0x12, /*0x21*/ 0x04 /*0x25*/),

    cmos_sensor(0x0b, 0xaa), // ESD Check Register
    cmos_sensor(0x0c, 0xaa), // ESD Check Register
    cmos_sensor(0x0d, 0xaa), // ESD Check Register

    cmos_sensor(0x20, 0x00), // Windowing start point Y
    cmos_sensor(0x21, 0x04),
    cmos_sensor(0x22, 0x00), // Windowing start point X
    cmos_sensor(0x23, 0x07),

    cmos_sensor(0x24, 0x03),
    cmos_sensor(0x25, 0xc0),
    cmos_sensor(0x26, 0x05),
    cmos_sensor(0x27, 0x00), // WINROW END

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
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_WB_INCANDESCENT
static const struct sensor_cmd set_wb_incandescent_reg_list[]=
{
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_FLUORESCENT
static const struct sensor_cmd set_wb_fluorescent_reg_list[]=
{
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_DAYLIGHT
static const struct sensor_cmd set_wb_daylight_reg_list[]=
{
    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_WB_WARM_FLUORECENT

//SET_WB_CLOUDY
static const struct sensor_cmd set_wb_cloudy_reg_list[]=
{
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
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x30),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x44, 0x80),
	cmos_sensor(0x45, 0x80),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_MONO
static const struct sensor_cmd set_color_effect_mono_reg_list[]=
{
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x03),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x40, 0x00),
	cmos_sensor(0x44, 0x80),
	cmos_sensor(0x45, 0x80),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_NEGATIVE
static const struct sensor_cmd set_color_effect_negative_reg_list[]=
{
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x08),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x14, 0x00),

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
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x33),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x44, 0x70),
	cmos_sensor(0x45, 0x98),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_POSTERIZE
//SET_EFFECT_WHITEBOARD
static const struct sensor_cmd set_color_effect_whiteboard_reg_list[]=
{
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x03),
	cmos_sensor(0x40, 0x00),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x44, 0x30),
	cmos_sensor(0x45, 0x50),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};
//SET_EFFECT_BLACKBOARD
static const struct sensor_cmd set_color_effect_blackboard_reg_list[]=
{
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x03),
	cmos_sensor(0x40, 0x00),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x44, 0xb0),
	cmos_sensor(0x45, 0x40),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

//SET_EFFECT_AQUA
static const struct sensor_cmd set_color_effect_aqua_reg_list[]=
{
	cmos_sensor(0x03, 0x10),
	cmos_sensor(0x11, 0x03),
	cmos_sensor(0x12, 0x03),
	cmos_sensor(0x13, 0x02),
	cmos_sensor(0x40, 0x00),
	cmos_sensor(0x44, 0x80),
	cmos_sensor(0x45, 0x80),

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
	cmos_sensor(0x03,0x20),
	cmos_sensor(0x10,0x9c),

    cmos_sensor(MAG_REG,MAG_VAL),
    cmos_sensor(MAG_REG,MAG_VAL),
};

static const struct sensor_cmd set_antibanding_60hz_reg_list[]=
{
    /* 60 Hz */     
	cmos_sensor(0x03,0x20),
	cmos_sensor(0x10,0x8c),

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
    /*cmos_sensor(0x03,0x10),
    cmos_sensor(0x12,0x30), 
    cmos_sensor(0x41,0x60),*/
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

const struct sensor_op_list * hy511_get_op_table(int op)
{
    if(op<SENSOR_OP_NUM)
        return GET_OP_TABLE(op);
        
    return NULL;
}

extern struct cam_interface hy511_interface;

struct cam_interface *hy511_detect(int * invvsync)
{
    unsigned char id;

    if(cam_reg_read(HY511_I2C_ADDR,0x04,&id,1)==0){
        printk("check camara(hy511 id 0x92) 0x%x\n",id);
        if(id==0x92){
            printk("Found Camara HY511 ID[0x%x]\n",id);
            if(invvsync)
                *invvsync=0;
            return &hy511_interface;
        }
    }
   
    return NULL;
}


int  hy511_init(void)
{
    int ret;

    cam_power_set(CAMARA_POWER_ON);

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_INIT_SENSOR);
    
    return ret;
}

int  hy511_switch_low_svga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_SVGA);
    return ret;
}
int  hy511_switch_high_svga(void){
    int ret;

    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_SVGA);
    return ret;
}
int  hy511_switch_high_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_HIGH_XUGA);
    return ret;
}
int  hy511_switch_upmid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_UPMID_XUGA);
    return ret;
}
int  hy511_switch_mid_xuga(void){
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_MID_XUGA);
    return ret;
}
int  hy511_switch_low_xuga(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SWITCH_SENSOR_TO_LOW_XUGA);
    return ret;
}
int  hy511_to_high_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_HIGH_PREVIEW);
    return ret;
}
int  hy511_to_low_preview(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_SENSOR_TO_LOW_PREVIEW);
    return ret;
}

//int  cam_switch_low_xuga(void){return -1;}
int  hy511_close(void)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_MODE,SET_MODE_CLOSE_SENSOR);
    
    cam_power_set(CAMARA_POWER_OFF);

    return ret;
}
//-----------------------------------
//      SENSOR_SET_WHITE_BALANCE
//-----------------------------------
int  hy511_set_wb(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_WHITE_BALANCE,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_COLOR_EFFECT
//-----------------------------------
int  hy511_set_effect(int cmd_code)
{
    int ret;
    ret = excute_cam_cmd(SENSOR_SET_COLOR_EFFECT,cmd_code);
    
    return ret;
}
//-----------------------------------
//      SENSOR_SET_BRIGHTNESS
//-----------------------------------
int  hy511_set_brightness(unsigned char value)
{
    struct sensor_cmd cmd;
    int ret;
    
    cmd.reg = 0x03;
    cmd.val = value;
    ret=cam_reg_write(HY511_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));

    if(ret)
        return ret;

    cmd.reg = 0x40;
    cmd.val = value;
    ret=cam_reg_write(HY511_I2C_ADDR,(unsigned char *)&cmd,sizeof(struct sensor_cmd));
    
    return ret;
}

//-----------------------------------
//      SENSOR_SET_SCENCE_MODE
//-----------------------------------
int  hy511_night_mode_on(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_NIGHT);
    return ret;
}                                                    
int  hy511_night_mode_off(void)
{
    int ret;
    
    ret = excute_cam_cmd(SENSOR_SET_SCENCE_MODE,SET_SCENCE_AUTO);
    return ret;
}

int  hy511_dump_reg(void)
{
    unsigned char reg;
    unsigned char val;
    int ret;
    
    for(reg=0;reg<0xff;reg++){

        ret = cam_reg_read(HY511_I2C_ADDR,reg,&val,1);
        if(ret != 0){
            printk("reg 0x%x  failed\n",reg);
        }else{
            printk("0x%x 0x%x\n",reg,val);
        }
    }

    return 0;
}

struct device_info hy511_info={
    HY511_I2C_ADDR,
};


struct cam_interface hy511_interface={
    &hy511_info,
    hy511_get_op_table,
    hy511_init,
    hy511_switch_low_svga,
    hy511_switch_high_svga,
    hy511_switch_high_xuga,
    hy511_switch_upmid_xuga,
    hy511_switch_mid_xuga,
    hy511_switch_low_xuga,
    hy511_to_high_preview,
    hy511_to_low_preview,
    hy511_close,
    hy511_set_wb,
    hy511_set_effect,
    hy511_set_brightness,
    hy511_night_mode_on,                                                    
    hy511_night_mode_off,
    hy511_dump_reg,
};

