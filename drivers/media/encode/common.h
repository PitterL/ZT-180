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
 * Description: this head file include common macros
 *
 * Author:
 *     Sololz <sololz@alerttm.com>
 *      
 * Revision History: 
 * ­­­­­­­­­­­­­­­­­ 
 * 1.1  12/29/2009 Sololz 
 ******************************************************************************/
#ifndef __ENC_COMMON_H__
#define __ENC_COMMON_H__

#define ENCODE_RET_OK			0
#define ENCODE_RET_ERROR		-1000

/* registers related macros */
#define IMAPX200_ENCODE_ACT_REG_SIZE	(96 * 4)

/* device number related macros */
#define ENCODE_DYNAMIC_MAJOR		0
#define ENCODE_DYNAMIC_MINOR		255
#define ENCODE_DEFAULT_MAJOR		113	/* better not use default major or minor */
#define ENCODE_DEFAULT_MINOR		113

/* Ioctl commands related macros */
#define HX280ENC_IOC_MAGIC  		'k'
#define HX280ENC_IOC_MAXNR 		8
#define HX280ENC_IOCGHWOFFSET      	_IOR(HX280ENC_IOC_MAGIC, 3, unsigned long *)
#define HX280ENC_IOCGHWIOSIZE      	_IOR(HX280ENC_IOC_MAGIC, 4, unsigned int *)
#define HX280ENC_IOC_CLI		_IOR(HX280ENC_IOC_MAGIC, 5, unsigned int *)
#define HX280ENC_IOC_STI           	_IOR(HX280ENC_IOC_MAGIC, 6, unsigned int *)
#define HX280ENC_IOCXVIRT2BUS      	_IOWR(HX280ENC_IOC_MAGIC, 7, unsigned long *)
#define HX280ENC_IOCHARDRESET      	_IOR(HX280ENC_IOC_MAGIC, 8, unsigned int *)   /* debugging tool */

/*
 * Debug macros include debug alert error
 */
#ifdef CONFIG_IMAP_ENCODE_DEBUG

#define ENCODE_DEBUG(debug, ...) 	\
	printk(KERN_DEBUG "%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)

#define ENCODE_ALERT(alert, ...) 	\
	printk(KERN_ALERT "%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)

#define ENCODE_ERROR(error, ...)	\
	printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#else

#define ENCODE_DEBUG(debug, ...)   	do{}while(0)
#define ENCODE_ALERT(alert, ...)  	do{}while(0)
#define ENCODE_ERROR(error, ...)    	do{}while(0)

#endif /* CONFIG_IMAP_MEDIA_ENCODE_DEBUG */

#define encode_debug(debug, ...) 	ENCODE_DEBUG(debug, ##__VA_ARGS__)
#define encode_alert(alert, ...)	ENCODE_ALERT(alert, ##__VA_ARGS__)
#define encode_error(error, ...)        ENCODE_ERROR(error, ##__VA_ARGS__)

typedef struct {
	int major;
	int minor;
	dev_t dev;
	unsigned int enc_instance;
	void __iomem *reg_base_virt_addr;
	unsigned int reg_base_phys_addr;
	unsigned int reg_reserved_size;
	struct resource	*resource_mem;
	struct fasync_struct *async_queue_enc;
} encode_param_t;

#endif	/* __ENC_COMMON_H__ */
