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
 * Description: common head file of imapx200 media decode driver
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

#define IMAPX200_DECODE_RET_OK		0
#define IMAPX200_DECODE_RET_ERROR	-1000

/*
 * macros about irq
 * irq register location and someothers
 */
#define IMAPX200_DECODE_IRQ_STAT_DEC	1
#define IMAPX200_DECODE_IRQ_STAT_PP	60

#define IMAPX200_DECODE_IRQ_BIT_DEC	0x100
#define IMAPX200_DECODE_IRQ_BIT_PP	0x100

#define DECODE_DEFAULT_MAJOR		112
#define DECODE_DEFAULT_MINOR		112

/*
 * in arch/arm/mach-imapx200/devices.c there reserved 4KB for
 * Decode register, but actually only first 404 bytes used, 
 * 4KB is a page size, so reserve 4KB much larger than decode
 * needs does make sense
 */
#define IMAPX200_DECODE_ACT_REG_SIZE	(101 * 4)

/*
 * ioctl commands
 */
#define IMAPX200_DECODE_MAGIC		'k'
#define IMAPX200_DECODE_MAX_CMD		6

#define IMAPX200_PP_INSTANCE		_IOR(IMAPX200_DECODE_MAGIC, 1, unsigned int *)
#define IMAPX200_HW_PERFORMANCE		_IOR(IMAPX200_DECODE_MAGIC, 2, unsigned int *)
#define IMAPX200_REG_BASE		_IOR(IMAPX200_DECODE_MAGIC, 3, unsigned long *)
#define IMAPX200_REG_SIZE		_IOR(IMAPX200_DECODE_MAGIC, 4, unsigned int *)

#define IMAPX200_IRQ_DISABLE		_IOR(IMAPX200_DECODE_MAGIC, 5, unsigned int *)
#define IMAPX200_IRQ_ENABLE		_IOR(IMAPX200_DECODE_MAGIC, 6, unsigned int *)

/*
 * debug macros include debug alert error
 */
#ifdef CONFIG_IMAP_DECODE_DEBUG

#define DECODE_DEBUG(debug, ...)         \
	                printk(KERN_DEBUG "%s line %d: " debug, __func__, __LINE__, ##__VA_ARGS__)

#define DECODE_ALERT(alert, ...)         \
	                printk(KERN_ALERT "%s line %d: " alert, __func__, __LINE__, ##__VA_ARGS__)

#define DECODE_ERROR(error, ...)         \
	                printk(KERN_ERR "%s line %d: " error, __func__, __LINE__, ##__VA_ARGS__)

#else

#define DECODE_DEBUG(debug, ...)	do{}while(0)
#define DECODE_ALERT(alert, ...)	do{}while(0)
#define DECODE_ERROR(error, ...)	do{}while(0)

#endif /* CONFIG_IMAP_MEDIA_DECODE_DEBUG */

#define decode_debug(debug, ...)         DECODE_DEBUG(debug, ##__VA_ARGS__)
#define decode_alert(alert, ...)         DECODE_ALERT(alert, ##__VA_ARGS__)
#define decode_error(error, ...)         DECODE_ERROR(error, ##__VA_ARGS__)

/*
 * global variables
 */
typedef struct 
{
	struct resource         *resource_mem;
	void __iomem            *reg_base_virt_addr;
	unsigned int            reg_base_phys_addr;
	unsigned int            reg_reserved_size;
#ifdef CONFIG_IMAP_DECODE_SIGNAL_MODE
	struct fasync_struct    *async_queue_dec;
	struct fasync_struct    *async_queue_pp;
#endif	/* CONFIG_IMAP_DECODE_SIGNAL_MODE */
	unsigned int            dec_instance;
	unsigned int            pp_instance;
#ifdef CONFIG_IMAP_DEC_HW_PERFORMANCE
	struct timeval		end_time;
#endif
}decode_param_t;

#endif	/* __DEC_COMMON_H__ */
