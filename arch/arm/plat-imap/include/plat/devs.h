/********************************************************************************
** linux-2.6.28.5/arch/arm/plat-imap/include/plat/devs.h
**
** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
**
** Use of Infotm's code is governed by terms and conditions
** stated in the accompanying licensing statement.
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** Author:
**     Raymond Wang   <raymond.wang@infotmic.com.cn>
**
** Revision History:
**     1.1  09/15/2009    Raymond Wang
********************************************************************************/

#include <linux/platform_device.h>

struct imap_uart_resources {
	struct resource		*resources;
	unsigned long		 nr_resources;
};

extern struct imap_uart_resources imapx200_uart_resources[];

extern struct platform_device *imap_uart_devs[];
extern struct platform_device *imap_uart_src[];
extern struct platform_device imapx200_device_nand;
extern struct platform_device imapx200_device_sdi0;
extern struct platform_device imapx200_device_sdi1;
extern struct platform_device imapx200_device_sdi2;
extern struct platform_device imapx200_device_cf;
extern struct platform_device imapx200_device_usbhost11;
extern struct platform_device imapx200_device_usbhost20;
extern struct platform_device imapx200_device_usbotg;
extern struct platform_device imapx200_device_udc;
extern struct platform_device imapx200_device_camera;
extern struct platform_device imapx200_device_lcd;
#ifdef CONFIG_RGB2VGA_OUTPUT_SUPPORT
extern struct platform_device imapx200_device_rgb2vga;
#endif
#ifdef CONFIG_HDMI_OUTPUT_SUPPORT
extern struct platform_device imapx200_device_HDMI;
#endif
extern struct platform_device imapx200_device_osd;
extern struct platform_device imapx200_device_i80;
extern struct platform_device imapx200_device_mac;
extern struct platform_device imapx200_device_venc;
extern struct platform_device imapx200_device_vdec;
extern struct platform_device imapx200_device_memalloc;
extern struct platform_device imapx200_device_dsp;
extern struct platform_device imapx200_device_gps;
extern struct platform_device imapx200_device_pwm;
extern struct platform_device imapx200_device_iic0;
extern struct platform_device imapx200_device_iic1;
extern struct platform_device imapx200_device_rtc;
extern struct platform_device imapx200_device_iis;
extern struct platform_device imapx200_device_ac97;
extern struct platform_device imapx200_device_ssim0;
extern struct platform_device imapx200_device_ssim1;
extern struct platform_device imapx200_device_ssim2;
extern struct platform_device imapx200_device_ssis;
extern struct platform_device imapx200_device_spi;
extern struct platform_device imapx200_device_keybd;
extern struct platform_device imapx200_device_bl;
extern struct platform_device imapx200_device_pic0;
extern struct platform_device imapx200_device_pic1;
extern struct platform_device imapx200_device_graphic;
extern struct amba_device imap_ps2_device;
extern struct platform_device imapx200_button_device;
extern struct platform_device imapx200_device_orientation;
extern struct platform_device imapx200_battery;
extern struct platform_device imapx200_touch;
extern struct platform_device imapx200_sensor;
extern struct platform_device imapx200_device_backlights;
extern struct platform_device ums_device;
extern struct platform_device imapx200_android_usb;

#ifdef CONFIG_ANDROID_PMEM
extern struct platform_device android_pmem_device;
extern struct platform_device android_pmem_adsp_device;
#endif

void __init imapx200_register_device(struct platform_device *dev, void *data);
