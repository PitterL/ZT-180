/***************************************************************************** 
 * common.h
 * 
 * Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Description: common head file of imapx200 media sensor driver
 *
 * Author:
 *     Sololz <sololz@infotm.com>
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.1  12/10/2009 Sololz 
 *******************************************************************************/

#ifndef __DEC_COMMON_H__
#define __DEC_COMMON_H__


#define SENSOR_DEFAULT_MAJOR		205
#define SENSOR_DEFAULT_MINOR		0

/*
 * debug macros include debug alert error
 */
//#define CONFIG_IMAP_SENSOR_DEBUG
#ifdef CONFIG_IMAP_SENSOR_DEBUG

#define SENSOR_DEBUG(debug, ...)         \
	                printk(KERN_INFO "%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)

#define SENSOR_ALERT(alert, ...)         \
	                printk(KERN_ALERT "%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)

#define SENSOR_ERROR(error, ...)         \
	                printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#else

#define SENSOR_DEBUG(debug, ...)	do{}while(0)
#define SENSOR_ALERT(alert, ...)	do{}while(0)
#define SENSOR_ERROR(error, ...)	do{}while(0)

#endif /* CONFIG_IMAP_MEDIA_SENSOR_DEBUG */

#define sensor_debug(debug, ...)         SENSOR_DEBUG(debug, ##__VA_ARGS__)
#define sensor_alert(alert, ...)         SENSOR_ALERT(alert, ##__VA_ARGS__)
#define sensor_error(error, ...)         SENSOR_ERROR(error, ##__VA_ARGS__)

/*
 * global variables
 */

typedef struct 
{
	struct resource         *resource_mem;
	void __iomem            *reg_base_virt_addr;
	unsigned int            reg_base_phys_addr;
	unsigned int            reg_reserved_size;
	unsigned int            dec_instance;
	unsigned int            pp_instance;
}sensor_param_t;

#endif	/* __DEC_COMMON_H__ */
