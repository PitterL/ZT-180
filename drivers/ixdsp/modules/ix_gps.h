/***************************************************************************** 
**  driver/gps/ix_gps.h
** 
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
** 
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
** 
** Description: PCB test, module .
**
** Author:
**      
**      xecle <xecle.zhang@infotmic.com.cn>
**      warits <warits.wang@infotmic.com.cn>
**      
** Revision History: 
** ----------------- 
** 1.1  XXX 08/03/2010 XXX	
*****************************************************************************/

#ifndef __IX_GPS_H__
#define __IX_GPS_H__

#define IX_GPS_MAJOR		164
#define IX_GPS_MINOR		0
#define IX_GPS_NAME			"imapx200-gps"
#define IX_DSP_BIN_FILE		"/etc/firmware/igps/igps.bin"
#define IX_DSP_DATA_FILE	"/etc/firmware/igps/aiding.bin"
#define IX_DSP_COP_LEN		0x1c800
#define IX_DSP_COP_ADDR		0x2c000000
#define IX_GPS_AUX_SIZE		0x2000

#define IX_GPS_IOC_MAGIC	'p'

#define IX_GPS_ACCESS_DATA			_IOWR(IX_GPS_IOC_MAGIC, 1, unsigned long)
#define IX_GPS_START				_IOWR(IX_GPS_IOC_MAGIC, 2, unsigned long)
#define IX_GPS_STOP					_IOWR(IX_GPS_IOC_MAGIC, 3, unsigned long)
#define IX_GPS_IOC_MAX				10

#define iGPS_POWER_BIT (1 << 0)

#define gps_dbg(args...) printk(KERN_ERR "iGPS: " args)
//#define gps_dbg(args...) 

#define JUMP_ADDR	0x100100 


enum ix_gps_stat {
	IX_GPS_EMPTY = 0,
	IX_GPS_NMEA_READY,
	IX_GPS_NMEA_READING,
	IX_GPS_NMEA_UPDATING,
};

enum ix_gps_cmds {
	IX_GPS_CMD_SET_AID_ADDR = 0,
	IX_GPS_CMD_SET_DATA,
	IX_GPS_CMD_SET_UTC,
	IX_GPS_CMD_GET_UTC,
	IX_GPS_CMD_NAVIGATION_STOP,
};

enum ix_gps_access_type {
	IX_GPS_INJECT_DSP = 0,
	IX_GPS_INJECT_AIDING,
	IX_GPS_DUMP_AIDING,
};

struct ix_gps_access_desc {
	enum ix_gps_access_type	type;
	uint32_t				start;
	uint32_t				len;
	uint8_t *				buf;
};

struct ix_gps_dev {

	void			* auxdata;
    void            * prog_suspend;
	dma_addr_t		auxdata_dma;
	void __iomem	* copvp;
	void __iomem	* regs;
	struct class	* class;
	wait_queue_head_t	wq;
	spinlock_t		nmea_lock;
	uint8_t			*nmea_pos;
	uint8_t			*nmea_data;
	int				nmea_len;
	int				nmea_readed;
	int				dsp_user;
	int				aid_user;
	uint32_t		state;
	uint32_t		start;
	uint32_t		sender;
	uint32_t		receiver;
};

extern struct ix_gps_dev ix_gps;

static inline void 
ix_gps_set_stat(struct ix_gps_dev *gps, enum ix_gps_stat stat)
{
	spin_lock(gps->nmea_lock);
	gps->state = stat;
	spin_unlock(gps->nmea_lock);
}

static inline enum ix_gps_stat
ix_gps_get_stat(struct ix_gps_dev *gps)
{
	enum ix_gps_stat stat;

	spin_lock(gps->nmea_lock);
	stat = gps->state;
	spin_unlock(gps->nmea_lock);

	return stat;
}

#define gps_writel(v, f) writel(v, ix_gps.regs + f)
#define gps_readl(f) readl(ix_gps.regs + f)

#endif /* __IX_GPS_H__ */
