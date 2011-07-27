/*
 * Copyright (C) 2010 MEMSIC, Inc.
 *
 * Initial Code:
 *	Robbie Cao
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 *
 */

/*
 * Definitions for mxc622x accelorometer sensor chip.
 */
#ifndef __MMA7660_H__
#define __MMA7660_H__

#if 0 

#include <linux/ioctl.h>

#define MMA7660_I2C_NAME		"mxc622x"

/*
 * This address comes must match the part# on your target.
 * Address to the sensor part# support as following list:
 *   MXC6220	- 0x10
 *   MXC6221	- 0x11
 *   MXC6222	- 0x12
 *   MXC6223	- 0x13
 *   MXC6224	- 0x14
 *   MXC6225	- 0x15
 *   MXC6226	- 0x16
 *   MXC6227	- 0x17
 * Please refer to sensor datasheet for detail.
 */
#define MMA7660_I2C_ADDR		0x15

/* MMA7660 register address */
#define MMA7660_REG_CTRL		0x04
#define MMA7660_REG_DATA		0x00

/* MMA7660 control bit */
#define MMA7660_CTRL_PWRON		0x00	/* power on */
#define MMA7660_CTRL_PWRDN		0x80	/* power donw */

/* Use 'm' as magic number */
#define MMA7660_IOM			'm'

/* IOCTLs for MMA7660 device */
#define MMA7660_IOC_PWRON		_IO (MMA7660_IOM, 0x00)
#define MMA7660_IOC_PWRDN		_IO (MMA7660_IOM, 0x01)
#define MMA7660_IOC_READXYZ		_IOR(MMA7660_IOM, 0x05, int[3])
#define MMA7660_IOC_READSTATUS		_IOR(MMA7660_IOM, 0x07, int[3])
#define MMA7660_IOC_SETDETECTION	_IOW(MMA7660_IOM, 0x08, unsigned char)


#else


#include <linux/ioctl.h>

#define MMA7660_I2C_NAME				"mma7660"

/*
 * This address comes must match the part# on your target.
 * Address to the sensor part# support as following list:
 *   MXC6220	- 0x10
 *   MXC6221	- 0x11
 *   MXC6222	- 0x12
 *   MXC6223	- 0x13
 *   MXC6224	- 0x14
 *   MXC6225	- 0x15
 *   MXC6226	- 0x16
 *   MXC6227	- 0x17
 * Please refer to sensor datasheet for detail.
 */
#define MMA7660_I2C_ADDR				0x4c

/* MMA7660 register address */
//#define MMA7660_REG_CTRL				0x04
//#define MMA7660_REG_DATA				0x00

/* MMA7660 control bit */
//#define MMA7660_CTRL_PWRON				0x00				/* power on */
//#define MMA7660_CTRL_PWRDN				0x80				/* power donw */

/* Use 'm' as magic number */
#define MMA7660_IOM						'm'

/* IOCTLs for MMA7660 device */
#define MMA7660_IOC_PWRON				_IO (MMA7660_IOM, 0x00)
#define MMA7660_IOC_PWRDN				_IO (MMA7660_IOM, 0x01)
#define MMA7660_IOC_READXYZ				_IOR(MMA7660_IOM, 0x05, int[3])
#define MMA7660_IOC_READSTATUS			_IOR(MMA7660_IOM, 0x07, int[3])
#define MMA7660_IOC_SETDETECTION		_IOW(MMA7660_IOM, 0x08, unsigned char)
#define MMA7660_IOC_GET_DIR	            _IOR(MMA7660_IOM, 0x09, unsigned char)

#endif /* __MMA7660_H__ */


#ifdef BIT_MASK
#undef BIT_MASK
#endif


#define   BIT_MASK(__bf)   (((1U   <<   (bw ##  __bf)) - 1) << (bs ## __bf))  
#define   SET_BITS(__dst, __bf, __val) \
  ((__dst) = ((__dst) & ~(BIT_MASK(__bf))) | \
  (((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))
#define   GET_BITS(__dst, __bf) \
      (((__dst) & (BIT_MASK(__bf)))>>(bs ## __bf))

//reg 0~2
#define MMA_XOUT    0  //data x
#define MMA_YOUT    1  //data y
#define MMA_ZOUT    2  //data z
#define bsMMA_xOUT_DATA     0
#define bwMMA_xOUT_DATA     5
#define bsMMA_xOUT_DIRECT   5
#define bwMMA_xOUT_DIRECT   1
#define bsMMA_xOUT_ALERT    6
#define bwMMA_xOUT_ALERT    1

//reg 3
#define MMA_TILT    3  //Tilt Status
#define bsMMA_TILT_BAFRO    0
#define bwMMA_TILT_BAFRO    2
#define BAFRO_UNKNOW      0   //Unknown condition of front or back
#define BAFRO_FRONT       1
#define BAFRO_BACK        2
#define bsMMA_TILT_POLA     2
#define bwMMA_TILT_POLA     3
#define POLA_UNKNOW      0
#define POLA_LEFT        1
#define POLA_RIGHT       2
#define POLA_DOWN        5
#define POLA_UP          6
#define bsMMA_TILT_TAP      5  //Equipment has detected a tap
#define bwMMA_TILT_TAP      1
#define bsMMA_TILT_ALERT    6  
#define bwMMA_TILT_ALERT    1
#define bsMMA_TILT_SHAKE    7  //Equipment is experiencing shake
#define bwMMA_TILT_SHAKE    1

//reg 4
#define MMA_SRST    4  //Sampling Rate Status
#define bsMMA_SRST_AMSRS  0  //Samples per second specified in AMSR[2:0] is active
#define bwMMA_SRST_AMSRS  1
#define bsMMA_SRST_AWSRS  1  //Samples per second specified in AWSR[1:0] is active
#define bwMMA_SRST_AWSRS  1

//reg 5
#define MMA_SPCNT   5  //Sleep Count

//reg 6
#define MMA_INTSU   6  //Interrupt Setup
#define MMA_INT_FB      (1<<0)  //Front/Back position change
#define MMA_INT_PL      (1<<1)  //Up/Down/Right/Left position change
#define MMA_INT_PD      (1<<2)  //Successful tap detection
#define MMA_INT_AS      (1<<3)  //Exiting Auto-Sleep
#define MMA_INT_G       (1<<4)  //interrupt after every measurement
#define MMA_INT_SHZ     (1<<5)  //Shake on the X-axis
#define MMA_INT_SHY     (1<<6)  //Shake on the Y-axis
#define MMA_INT_SHX     (1<<7)  //Shake on the Z-axis

//reg 7
#define MMA_MODE    7  //Mode
#define MMA_MODE_MODE   (1<<0)  //0: Standby mode or Test Mode depending on state of TON
                                //1: Active mode
#define MMA_MODE_TON    (1<<2)
                                //0: Standby Mode or Active Mode depending on state of MODE
                                //1: Test Mode
#define MMA_MODE_AWE    (1<<3)  //Auto-Wake enabled
#define MMA_MODE_ASE    (1<<4)  //Auto-Sleep enabled
#define MMA_MODE_SCPS   (1<<5)  //0: AMSR The prescaler is divide-by-1
                                //1: AMSR Prescaler is divide-by-16
#define MMA_MODE_IPP    (1<<6)  //0: Interrupt output INT is open-drain
                                //1: Interrupt output INT is push-pull
#define MMA_MODE_IAH    (1<<7)  //0: Interrupt output INT is active low
                                //1: Interrupt output INT is active high
//reg 8
#define MMA_SR      8  //Auto-Wake/Sleep and Portrait/Landscape samples per seconds and Debounce Filter
#define bsMMA_SR_AMSR   0     //Tap Detection Mode and n Samples/Second Active and Auto-Sleep Mode
#define bwMMA_SR_AMSR   3
#define AMSR_RATE_128       0
#define AMSR_RATE_64        1
#define AMSR_RATE_32        2
#define AMSR_RATE_16        3
#define AMSR_RATE_8         4
#define AMSR_RATE_4         5
#define AMSR_RATE_2         6
#define AMSR_RATE_1         7
#define bsMMA_SR_AWSR   3   //n Samples/Second Auto-Wake Mode
#define bwMMA_SR_AWSR   2
#define AWSR_RATE_32        0
#define AWSR_RATE_16        1
#define AWSR_RATE_8         2
#define AWSR_RATE_1         3
#define bsMMA_SR_FILT   5   //n measurement samples at the rate set by AMSR[2:0] or AWSR[1:0] have to match before the device updates portrait/landscape data in TILT (0x03) register.
#define bwMMA_SR_FILT   3
#define FILT_DISABLE        0
#define FILT_DEBOUNCE_2     1
#define FILT_DEBOUNCE_3     2
#define FILT_DEBOUNCE_4     3
#define FILT_DEBOUNCE_5     4
#define FILT_DEBOUNCE_6     5
#define FILT_DEBOUNCE_7     6
#define FILT_DEBOUNCE_8     7

//reg 9
#define MMA_PDET    9  //Tap Detection
#define bsMMA_PDET_PDTH     0    //Tap detection threshold
#define bwMMA_PDET_PDTH     5
#define bsMMA_PDET_XDA      5    //X-axis is enabled for tap detection
#define bwMMA_PDET_XDA      1
#define bsMMA_PDET_YDA      6    //Y-axis is enabled for tap detection
#define bwMMA_PDET_YDA      1
#define bsMMA_PDET_ZDA      5    //Z-axis is enabled for tap detection
#define bwMMA_PDET_ZDA      7

//reg 10
#define MMA_PD      10 //Tap Debounce Count
                       //The tap detection debounce filtering requires n adjacent tap detection

#define ADC_MAX  (1<<bwMMA_xOUT_DATA)

enum {
    SENSOR_UNKNOW,
    SENSOR_OPENED,
    SENSOR_WORKING,
    SENSOR_CLOSED
};


#endif
