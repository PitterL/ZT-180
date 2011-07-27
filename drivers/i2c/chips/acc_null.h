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
#ifndef __ACC_NULL_H__
#define __ACC_NULL_H__


#include <linux/ioctl.h>

#define ACC_NULL_NAME   "acc_null"

#define ACC_NULL_MAJOR		205
#define ACC_NULL_MINOR		0

/* Use 'm' as magic number */
#define ACC_NULL_IOM						'm'

/* IOCTLs for ACC_NULL device */
#define ACC_NULL_IOC_PWRON				_IO (ACC_NULL_IOM, 0x00)
#define ACC_NULL_IOC_PWRDN				_IO (ACC_NULL_IOM, 0x01)
#define ACC_NULL_IOC_READXYZ				_IOR(ACC_NULL_IOM, 0x05, int[3])
#define ACC_NULL_IOC_READSTATUS			_IOR(ACC_NULL_IOM, 0x07, int[3])
#define ACC_NULL_IOC_SETDETECTION		_IOW(ACC_NULL_IOM, 0x08, unsigned char)
#define ACC_NULL_IOC_GET_DIR	            _IOR(ACC_NULL_IOM, 0x09, unsigned char)



#ifdef BIT_MASK
#undef BIT_MASK
#endif


#define   BIT_MASK(__bf)   (((1U   <<   (bw ##  __bf)) - 1) << (bs ## __bf))  
#define   SET_BITS(__dst, __bf, __val) \
  ((__dst) = ((__dst) & ~(BIT_MASK(__bf))) | \
  (((__val) << (bs ## __bf)) & (BIT_MASK(__bf))))
#define   GET_BITS(__dst, __bf) \
      (((__dst) & (BIT_MASK(__bf)))>>(bs ## __bf))


enum {
    SENSOR_UNKNOW,
    SENSOR_OPENED,
    SENSOR_WORKING,
    SENSOR_CLOSED
};


#endif
