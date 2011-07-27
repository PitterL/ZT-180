/***************************************************************************** 
 * /linux/arch/arm/mach-imapx200/cpu.c
 * ** 
 * ** Copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved. 
 * ** 
 * ** Use of Infotm's code is governed by terms and conditions 
 * ** stated in the accompanying licensing statement. 
 * ** 
 * ** Description: Inialization of the board related fuctions and devices.
 * **
 * ** Author:
 * **     Alex Zhang   <tao.zhang@infotmic.com.cn>
 * **      
 * ** Revision History: 
 * ** ----------------- 
 * ** 1.2  25/11/2009  Alex Zhang   
 * *****************************************************************************/ 

#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/interrupt.h>
#include <linux/list.h>
#include <linux/timer.h>
#include <linux/init.h>
#include <linux/sysdev.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/io.h>
#include <linux/input.h>
#include <linux/imapx200_keybd.h>
#include <linux/spi/spi.h>
#include <linux/spi/ads7846.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>
#include <asm/mach/irq.h>

#include <mach/hardware.h>
#include <asm/hardware/iomd.h>
#include <asm/setup.h>
#include <asm/irq.h>
#include <asm/mach-types.h>

#include <asm/proc-fns.h>
#include <asm/cpu-single.h>
#include <plat/clock.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/imapx200.h>
#include <mach/idle.h>
#include <plat/imapx.h>
#include <plat/regs-serial.h>
#include <plat/pm.h>
#include <linux/amba/bus.h>


#define DEF_MCR	IMAPX200_MCR_SIRE_IRDA_DISABLE | IMAPX200_MCR_AFCE_AFC_DISABLE
#define DEF_LCR IMAPX200_LCR_DLS_8BIT |IMAPX200_LCR_STOP_ONE_STOP_BIT |IMAPX200_LCR_PEN_PARITY_DISABLE
#define DEF_FCR  IMAPX200_FCR_FIFOE_FIFO_ENABLE | IMAPX200_FCR_TET_TX_THRESHOLD_LEVEL_EMPTY | IMAPX200_FCR_RT_RX_TRIGGER_LEVEL_ONE_CHAR 

extern void imap_mem_reserve(void);
extern struct sys_timer imapx200_init_timer;

static struct platform_device *imapx200_devices[] __initdata = {
	&imapx200_device_nand,
	&imapx200_button_device,
	&imapx200_device_sdi0,
	&imapx200_device_sdi1,
	&imapx200_device_sdi2,
	&imapx200_device_cf,
	&imapx200_device_usbhost11,
	&imapx200_device_usbhost20,
	&imapx200_device_usbotg,
	&imapx200_device_udc,
	&imapx200_device_camera,
	&imapx200_device_lcd,
#ifdef CONFIG_RGB2VGA_OUTPUT_SUPPORT
	&imapx200_device_rgb2vga,
#endif
#ifdef CONFIG_HDMI_OUTPUT_SUPPORT
	&imapx200_device_HDMI,
#endif
	&imapx200_device_mac,
	&imapx200_device_venc,
	&imapx200_device_vdec,
	&imapx200_device_memalloc,
	&imapx200_device_dsp,
	&imapx200_device_gps,
	&imapx200_device_pwm,
	&imapx200_device_iic0,
	&imapx200_device_iic1,
	&imapx200_device_rtc,
	&imapx200_device_iis,
	&imapx200_device_ac97,
	&imapx200_device_ssim0,
	&imapx200_device_ssim1,
	&imapx200_device_ssim2,
	&imapx200_device_ssis,
	&imapx200_device_spi,
//	&imapx200_device_keybd,
	&imapx200_device_bl,
	&imapx200_device_pic0,
	&imapx200_device_pic1,
	&imapx200_device_graphic,
#ifdef IMAPX200_ADS7846_SPI

	&imapx200_device_orientation,
#endif
	&imapx200_battery,
	&imapx200_touch,
	&imapx200_sensor,
	&ums_device,
	&imapx200_android_usb,
	&imapx200_device_backlights,

#ifdef CONFIG_ANDROID_PMEM
	&android_pmem_device,
	//&android_pmem_adsp_device,
#endif
};

static struct map_desc imapx200_iodesc[] __initdata = {
};


static struct imapx200_uart_clksrc imapx200_serial_clocks[] = {

	[0] = {
		.name		= "pclk",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	},

#if 0 /* HS UART Source is changed from epll to mpll */
	[1] = {
		.name		= "ext_uart",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 0,
	},

#if defined (CONFIG_SERIAL_S3C64XX_HS_UART)

	[2] = {
		.name		= "epll_clk_uart_192m",
		.divisor	= 1,
		.min_baud	= 0,
		.max_baud	= 4000000,
	}
#endif

#endif

};


static struct imapx200_uartcfg imapx200_uartcfgs[] = {
	[0] = {
		.hwport	     = 0,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	},
	[1] = {
		.hwport	     = 1,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	},
	[2] = {
		.hwport	     = 2,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	},
#if 1
	[3] = {
		.hwport	     = 3,
		.flags	     = 0,
		.fcr	     = DEF_FCR,
		.lcr	     = DEF_LCR,
		.mcr	     = DEF_MCR,
		.clocks	     = imapx200_serial_clocks,
		.clocks_size = ARRAY_SIZE(imapx200_serial_clocks),
	}
#endif
};

//keyboard platform data
#ifdef CONFIG_MATRIXKEY_FOR_PRODUCT

// KEY(3,5,KEY_WIN) -> KEY(3,5,0)
// KEY(1,3,KEY_ESC) -> KEY(1,3,158) 
static unsigned int imapx200_matrix_keys[] = {
	KEY(0,0,0),KEY(0,1,0),KEY(0,2,KEY_F10),KEY(0,3,0),KEY(0,4,0),KEY(0,5,0),KEY(0,6,KEY_LEFTCTRL),KEY(0,7,KEY_F5),
	KEY(1,0,KEY_Q),KEY(1,1,KEY_TAB),KEY(1,2,KEY_A),KEY(1,3,158),KEY(1,4,KEY_Z),KEY(1,5,KEY_PAUSE),KEY(1,6,KEY_GRAVE),KEY(1,7,KEY_1),
	KEY(2,0,KEY_W),KEY(2,1,KEY_CAPSLOCK),KEY(2,2,KEY_S),KEY(2,3,0),KEY(2,4,KEY_X),KEY(2,5,KEY_MENU),KEY(2,6,0),KEY(2,7,KEY_2),
	KEY(3,0,KEY_E),KEY(3,1,KEY_F3),KEY(3,2,KEY_D),KEY(3,3,KEY_F4),KEY(3,4,KEY_C),KEY(3,5,0),KEY(3,6,0),KEY(3,7,KEY_3),
	KEY(4,0,KEY_R),KEY(4,1,KEY_T),KEY(4,2,KEY_F),KEY(4,3,KEY_G),KEY(4,4,KEY_V),KEY(4,5,KEY_B),KEY(4,6,KEY_5),KEY(4,7,KEY_4),
	KEY(5,0,KEY_U),KEY(5,1,KEY_Y),KEY(5,2,KEY_J),KEY(5,3,KEY_H),KEY(5,4,KEY_M),KEY(5,5,KEY_N),KEY(5,6,KEY_6),KEY(5,7,KEY_7),
	KEY(6,0,KEY_I),KEY(6,1,KEY_BACKSLASH),KEY(6,2,KEY_K),KEY(6,3,KEY_F6),KEY(6,4,KEY_COMMA),KEY(6,5,0),KEY(6,6,KEY_LEFTBRACE),KEY(6,7,KEY_8),
	KEY(7,0,KEY_O),KEY(7,1,KEY_F7),KEY(7,2,KEY_L),KEY(7,3,0),KEY(7,4,KEY_DOT),KEY(7,5,0),KEY(7,6,KEY_F8),KEY(7,7,KEY_9),
	KEY(8,0,0),KEY(8,1,KEY_INSERT),KEY(8,2,0),KEY(8,3,KEY_SPACE),KEY(8,4,KEY_NUMLOCK),KEY(8,5,0),KEY(8,6,KEY_DELETE),KEY(8,7,0),
	KEY(9,0,0),KEY(9,1,KEY_BACKSPACE),KEY(9,2,0),KEY(9,3,0),KEY(9,4,KEY_ENTER),KEY(9,5,0),KEY(9,6,KEY_F9),KEY(9,7,0),
	KEY(10,0,KEY_BACKSLASH),KEY(10,1,0),KEY(10,2,0),KEY(10,3,KEY_LEFTALT),KEY(10,4,0),KEY(10,5,0),KEY(10,6,0),KEY(10,7,KEY_PRINT),
	KEY(11,0,KEY_P),KEY(11,1,KEY_EQUAL),KEY(11,2,KEY_SEMICOLON),KEY(11,3,KEY_RIGHTBRACE),KEY(11,4,KEY_APOSTROPHE),KEY(11,5,KEY_SLASH),KEY(11,6,KEY_MINUS),KEY(11,7,KEY_0),
	KEY(12,0,0),KEY(12,1,0),KEY(12,2,KEY_UP),KEY(12,3,KEY_DOWN),KEY(12,4,KEY_LEFT),KEY(12,5,KEY_RIGHT),KEY(12,6,0),KEY(12,7,0),
	KEY(13,0,KEY_F2),KEY(13,1,KEY_LEFTSHIFT),KEY(13,2,0),KEY(13,3,KEY_RIGHTBRACE),KEY(13,4,0),KEY(13,5,0),KEY(13,6,KEY_F1),KEY(13,7,KEY_FN),
	
};


//key-remap for product board
 static unsigned int imapx200_matrix_keys_daban[] = {
	KEY(0, 0, 0), KEY(0, 1, 0),
	KEY(0, 2, 0), KEY(0, 3, 0),
	KEY(0, 4, 0), KEY(0, 5, 0),
	KEY(0, 6, 0), KEY(0, 7, KEY_MENU),

	KEY(1, 0, KEY_RIGHTALT), KEY(1, 1, 0),
	KEY(1, 2, 0), KEY(1, 3, 0),
	KEY(1, 4, 0), KEY(1, 5, 0),
	KEY(1, 6, 0), KEY(1, 7, KEY_LEFTALT),

	KEY(2, 0, 0), KEY(2, 1, 0),
	KEY(2, 2, KEY_Z), KEY(2, 3, KEY_CAPSLOCK),
	KEY(2, 4, KEY_A), KEY(2, 5, KEY_S),
	KEY(2, 6, KEY_LEFTCTRL), KEY(2, 7, 0),

	KEY(3, 0, 0), KEY(3, 1, KEY_C),
	KEY(3, 2, KEY_X), KEY(3, 3, KEY_D),
	KEY(3, 4, KEY_Q), KEY(3, 5, KEY_TAB),
	KEY(3, 6, KEY_1), KEY(3, 7, KEY_GRAVE),

	KEY(4, 0, 0), KEY(4, 1, KEY_F),
	KEY(4, 2, KEY_E), KEY(4, 3, KEY_W),
	KEY(4, 4, KEY_2), KEY(4, 5, KEY_3),
	KEY(4, 6, KEY_F1), KEY(4, 7, KEY_ESC),

	KEY(5, 0, KEY_V), KEY(5, 1, KEY_G),
	KEY(5, 2, KEY_T), KEY(5, 3, KEY_R),
	KEY(5, 4, KEY_4), KEY(5, 5, KEY_5),
	KEY(5, 6, KEY_F3), KEY(5, 7, KEY_F2),

	KEY(6, 0, 0), KEY(6, 1, KEY_H),
	KEY(6, 2, KEY_U), KEY(6, 3, KEY_Y),
	KEY(6, 4, KEY_6), KEY(6, 5, KEY_7),
	KEY(6, 6, KEY_F5), KEY(6, 7, KEY_F4),

	KEY(7, 0, 0), KEY(7, 1, KEY_K),
	KEY(7, 2, KEY_J), KEY(7, 3, KEY_I),
	KEY(7, 4, KEY_8), KEY(7, 5, KEY_9),
	KEY(7, 6, KEY_F7), KEY(7, 7, KEY_F6),

	KEY(8, 0, 0), KEY(8, 1, KEY_L),
	KEY(8, 2, KEY_P), KEY(8, 3, KEY_O),
	KEY(8, 4, KEY_0), KEY(8, 5, 0),
	KEY(8, 6, KEY_F9), KEY(8, 7, KEY_F8),

	KEY(9, 0, KEY_M), KEY(9, 1, KEY_SEMICOLON),
	KEY(9, 2, KEY_LEFTBRACE), KEY(9, 3, 0),
	KEY(9, 4, KEY_MINUS), KEY(9, 5, 0),
	KEY(9, 6, KEY_F11), KEY(9, 7, KEY_F10),

	KEY(10, 0, KEY_B), KEY(10, 1, KEY_COMMA),
	KEY(10, 2, KEY_APOSTROPHE), KEY(10, 3, 0),
	KEY(10, 4, 0), KEY(10, 5, KEY_EQUAL),
	KEY(10, 6, KEY_NUMLOCK), KEY(10, 7, KEY_F12),

	KEY(11, 0, 0), KEY(11, 1, 0),
	KEY(11, 2, KEY_N), KEY(11, 3, KEY_DOT),
	KEY(11, 4, 0), KEY(11, 5, 0),
	KEY(11, 6, KEY_PAUSE), KEY(11, 7, KEY_INSERT),

	KEY(12, 0, KEY_SPACE), KEY(12, 1, KEY_SLASH),
	KEY(12, 2, 0), KEY(12, 3, 0),
	KEY(12, 4, KEY_RIGHTBRACE), KEY(12, 5, 0),
	KEY(12, 6, KEY_LEFT), KEY(12, 7, KEY_PRINT),

	KEY(13, 0, KEY_LEFTSHIFT), KEY(13, 1, 0),
	KEY(13, 2, 0), KEY(13, 3, 0),
	KEY(13, 4, 0), KEY(13, 5, 0),
	KEY(13, 6, 0), KEY(13, 7, KEY_RIGHTSHIFT),

	KEY(14, 0, KEY_RIGHT), KEY(14, 1, KEY_DOWN),
	KEY(14, 2, 0), KEY(14, 3, KEY_UP),
	KEY(14, 4, KEY_ENTER), KEY(14, 5, KEY_BACKSLASH),
	KEY(14, 6, KEY_BACKSPACE), KEY(14, 7, KEY_DELETE),

	KEY(15, 0, KEY_FN), KEY(15, 1, 0),
	KEY(15, 2, 0), KEY(15, 3, 0),
	KEY(15, 4, 0), KEY(15, 5, 0),
	KEY(15, 6, 0), KEY(15, 7, KEY_RIGHTCTRL),

	KEY(16, 0, KEY_KP8), KEY(16, 1, KEY_KP9),
	KEY(16, 2, KEY_KPASTERISK), KEY(16, 3, KEY_KPMINUS),
	KEY(16, 4, KEY_KPPLUS), KEY(16, 5, KEY_KPDOT),
	KEY(16, 6, KEY_KPSLASH), KEY(16, 7, 0),

	KEY(17, 0, KEY_KP0), KEY(17, 1, KEY_KP1),
	KEY(17, 2, KEY_KP2), KEY(17, 3, KEY_KP3),
	KEY(17, 4, KEY_KP4), KEY(17, 5, KEY_KP5),
	KEY(17, 6, KEY_KP6), KEY(17, 7, KEY_KP7),
};
#else
//key-remap for develop board
static unsigned int imapx200_matrix_keys[] = {
	KEY(0, 0, KEY_1), KEY(0, 1, KEY_ESC),
	KEY(0, 2, KEY_TAB), KEY(0, 3, KEY_Q),
	KEY(0, 4, KEY_Z), KEY(0, 5, 0),
	KEY(0, 6, KEY_GRAVE), KEY(0, 7, KEY_A),

	KEY(1, 0, KEY_FN), KEY(1, 1, 0),
	KEY(1, 2, KEY_LEFTSHIFT), KEY(1, 3, 0),
	KEY(1, 4, 0), KEY(1, 5, 0),
	KEY(1, 6, KEY_F1), KEY(1, 7, KEY_RIGHTSHIFT),

	KEY(2, 0, KEY_F5), KEY(2, 1, 0),
	KEY(2, 2, 0), KEY(2, 3, KEY_PAUSE),
	KEY(2, 4, KEY_MENU), KEY(2, 5, 0),
	KEY(2, 6, KEY_LEFTCTRL), KEY(2, 7, 0),

	KEY(3, 0, 0), KEY(3, 1, 0),
	KEY(3, 2, KEY_BACKSPACE), KEY(3, 3, 0),
	KEY(3, 4, KEY_ENTER), KEY(3, 5, 0),
	KEY(3, 6, KEY_F9), KEY(3, 7, 0),

	KEY(4, 0, 0), KEY(4, 1, 0),
	KEY(4, 2, 0), KEY(4, 3, 0),
	KEY(4, 4, 0), KEY(4, 5, 0),
	KEY(4, 6, KEY_F2), KEY(4, 7, 0),

	KEY(5, 0, KEY_2), KEY(5, 1, KEY_BACKSLASH),
	KEY(5, 2, KEY_CAPSLOCK), KEY(5, 3, KEY_W),
	KEY(5, 4, KEY_X), KEY(5, 5, 0),
	KEY(5, 6, 0), KEY(5, 7, KEY_S),

	KEY(6, 0, KEY_4), KEY(6, 1, KEY_G),
	KEY(6, 2, KEY_T), KEY(6, 3, KEY_R),
	KEY(6, 4, KEY_V), KEY(6, 5, KEY_B),
	KEY(6, 6, KEY_5), KEY(6, 7, KEY_F),

	KEY(7, 0, KEY_3), KEY(7, 1, KEY_F4),
	KEY(7, 2, KEY_F3), KEY(7, 3, KEY_E),
	KEY(7, 4, KEY_C), KEY(7, 5, 0),
	KEY(7, 6, 0), KEY(7, 7, KEY_D),

	KEY(8, 0, KEY_7), KEY(8, 1, KEY_H),
	KEY(8, 2, KEY_Y), KEY(8, 3, KEY_U),
	KEY(8, 4, KEY_M), KEY(8, 5, KEY_N),
	KEY(8, 6, KEY_6), KEY(8, 7, KEY_J),

	KEY(9, 0, KEY_F10), KEY(9, 1, 0),
	KEY(9, 2, KEY_LEFTMETA), KEY(9, 3, 0),
	KEY(9, 4, 0), KEY(9, 5, 0),
	KEY(9, 6, 0), KEY(9, 7, 0),

	KEY(10, 0, KEY_8), KEY(10, 1, KEY_F6),
	KEY(10, 2, KEY_RIGHTBRACE), KEY(10, 3, KEY_I),
	KEY(10, 4, KEY_COMMA), KEY(10, 5, 0),
	KEY(10, 6, KEY_EQUAL), KEY(10, 7, KEY_K),

	KEY(11, 0, KEY_9), KEY(11, 1, 0),
	KEY(11, 2, KEY_F7), KEY(11, 3, KEY_O),
	KEY(11, 4, KEY_DOT), KEY(11, 5, 0),
	KEY(11, 6, KEY_F8), KEY(11, 7, KEY_L),

	KEY(12, 0, KEY_0), KEY(12, 1, KEY_APOSTROPHE),
	KEY(12, 2, KEY_LEFTBRACE), KEY(12, 3, KEY_P),
	KEY(12, 4, KEY_BACKSLASH), KEY(12, 5, KEY_SLASH),
	KEY(12, 6, KEY_MINUS), KEY(12, 7, KEY_SEMICOLON),

	KEY(13, 0, KEY_PRINT), KEY(13, 1, KEY_LEFTALT),
	KEY(13, 2, 0), KEY(13, 3, 0),
	KEY(13, 4, 0), KEY(13, 5, 0),
	KEY(13, 6, 0), KEY(13, 7, 0),

	KEY(14, 0, 0), KEY(14, 1, 0),
	KEY(14, 2, 0), KEY(14, 3, 0),
	KEY(14, 4, 0), KEY(14, 5, 0),
	KEY(14, 6, KEY_INSERT), KEY(14, 7, 0),

	KEY(15, 0, 0), KEY(15, 1, KEY_DOWN),
	KEY(15, 2, 0), KEY(15, 3, KEY_KPSLASH),
	KEY(15, 4, KEY_LEFT), KEY(15, 5, KEY_RIGHT),
	KEY(15, 6, KEY_KPDOT), KEY(15, 7, KEY_UP),

	KEY(16, 0, KEY_KPPLUS), KEY(16, 1, KEY_SPACE),
	KEY(16, 2, KEY_KPASTERISK), KEY(16, 3, KEY_KPMINUS),
	KEY(16, 4, KEY_NUMLOCK), KEY(16, 5, KEY_KP9),
	KEY(16, 6, KEY_DELETE), KEY(16, 7, KEY_KP8),

	KEY(17, 0, KEY_KP0), KEY(17, 1, KEY_KP1),
	KEY(17, 2, KEY_KP2), KEY(17, 3, KEY_KP3),
	KEY(17, 4, KEY_KP4), KEY(17, 5, KEY_KP5),
	KEY(17, 6, KEY_KP6), KEY(17, 7, KEY_KP7),
};
#endif

struct imapx200_keybd_platform_data imapx200_keybd_info = {
	/* code map for the matrix keys */
	.matrix_key_rows	= 18,
	.matrix_key_cols	= 8,
	.matrix_key_map		= imapx200_matrix_keys,
	.matrix_key_map_size	= ARRAY_SIZE(imapx200_matrix_keys),
};
#ifdef IMAPX200_ADS7846_SPI
/*
 * ads7846 touchscreen
 */
static int ads7843_pendown_state(void)
{
	return !((__raw_readl(rGPGDAT) >> 2) & 1);      /* Touchscreen PENIRQ */
}
static struct ads7846_platform_data ads_info = {
	.model                  = 7846,
	.x_min                  = 150,
	.x_max                  = 3830,
	.y_min                  = 190,
	.y_max                  = 3830,
	.swap_xy		= 1,
	.vref_delay_usecs       = 100,
	.x_plate_ohms           = 580,
	.y_plate_ohms           = 410,
	.pressure_max           = 15000,
	.debounce_max           = 1,
	.debounce_rep           = 0,
	.debounce_tol           = (~0),
	.get_pendown_state      = ads7843_pendown_state,
};

static void __init imapx200_add_device_ts(void)
{
	volatile unsigned int tmp = 0;
	
	//EINT5
	tmp = __raw_readl(rEINTCON);
	tmp &= ~(7 << 20);
	tmp |=(7 << 20);
	tmp &= ~(0x7<<16);
	tmp |= (0x2<<16);
	__raw_writel(tmp , rEINTCON);
	//filter enable 3f count
	tmp = __raw_readl(rEINTFLTCON1);
	tmp &= ~(0xff<<0);
	tmp |= (0x1<<7)| (0x3f<<0);
	tmp |= (0xff<<8);
	__raw_writel(tmp, rEINTFLTCON1);


}


/*
 * spi device
 */
static struct spi_board_info imapx200_spi_devices[] = {
	{
		.modalias       = "ads7846",
		.chip_select    = 0,
		.max_speed_hz   = 125000 * 26,  /* (max sample rate @ 3V) * (cmd + data + overhead) */
		.bus_num        = 0,
		.platform_data  = &ads_info,
		.irq            = IRQ_EINT5,
	},
};
#endif


static void imapx200_idle(void)
{
	unsigned long tmp;
	/* ensure our idle mode is to go to idle */

	/* Set WFI instruction to SLEEP mode */
#if 1
	tmp = __raw_readl(rGPOW_CFG);
	tmp &= ~(0x7);
	tmp |= 0x1;
//	__raw_writel(tmp, rGPOW_CFG);
	cpu_do_idle();
#endif
}

struct sysdev_class imapx200_sysclass = { 
	.name           = "imapx200-core",
};

static struct sys_device imapx200_sysdev = { 
	.cls            = &imapx200_sysclass,
};

static int __init imapx200_core_init(void)
{
	return sysdev_class_register(&imapx200_sysclass);
}
core_initcall(imapx200_core_init);

void __init imapx200_init_irq(void)
{
	imap_init_irq();
}

void __init imdkx200_map_io(struct map_desc *mach_desc, int size)
{
	/* register our io-tables */
	iotable_init(mach_desc, size);
	/* rename any peripherals used differing from the s3c2412 */
	imap_idle = imapx200_idle;
}

void __init imapx200_init_uarts(struct imapx200_uartcfg *cfg, int no) 
{
	imap_init_uartdevs("imapx200-uart", imapx200_uart_resources, cfg, no);

	//      imap_device_lcd.name = "imap-lcd";
	//      imap_device_nand.name = "imap-nand";
}

int __init imapx200_init(void)
{
	int ret;

	printk("imapx200: Initialising architecture\n");

	ret = sysdev_register(&imapx200_sysdev);

	if(ret != 0)
		printk(KERN_ERR "failed to register sysdev for iampx200\n");
	return ret;
}
/*
static struct imap_board imapx200_board __initdata = { 
	.devices        = imapx200_devices,
	.devices_count  = ARRAY_SIZE(imapx200_devices)
};
*/

static void __init imapx200_map_io(void)
{
	imap_init_io(imapx200_iodesc, ARRAY_SIZE(imapx200_iodesc));
	imap_init_clocks(0);
	imap_init_uarts(imapx200_uartcfgs, ARRAY_SIZE(imapx200_uartcfgs));
	imap_mem_reserve();
	imap_pm_init();
}


static void __init imapx200_fixup(struct machine_desc *desc, struct tag *tags,
		char **cmdline, struct meminfo *mi)
{
	mi->nr_banks = 1;
	mi->bank[0].start = 0x40000000;
	mi->bank[0].size = 
    #if defined(CONFIG_BOARD_B0)
	    256*1024*1024
	#endif	
    #if defined(CONFIG_BOARD_E3)||defined(CONFIG_BOARD_E4)
	    256*1024*1024
	#endif
    #if defined(CONFIG_BOARD_E5)
	    512*1024*1024
	#endif	
    #if defined(CONFIG_BOARD_F0)
	    256*1024*1024
	#endif	
    #if defined(CONFIG_BOARD_G0)  
	    512*1024*1024
    #endif
    #if defined(CONFIG_BOARD_G0_3G)  
	    512*1024*1024
    #endif
    #if defined(CONFIG_BOARD_H0)  
	    512*1024*1024
    #endif
    #if defined(CONFIG_BOARD_I0)  
	    512*1024*1024
    #endif
    #if defined(CONFIG_BOARD_J0)  
	    512*1024*1024
    #endif
    #if defined(CONFIG_BOARD_K0)  
	    512*1024*1024
    #endif    
    ;
	mi->bank[0].node = 0;
}


static void __init imapx200_machine_init(void)
{
	//spi_register_board_info(imapx200_spi_devices, ARRAY_SIZE(imapx200_spi_devices));
	platform_add_devices(imapx200_devices, ARRAY_SIZE(imapx200_devices));
	//amba_device_register(&imap_ps2_device, &(imap_ps2_device.res));
#if defined(CONFIG_KEYBOARD_IMAPX200)	
	imapx200_set_keybd_info(&imapx200_keybd_info);
#endif	
#ifdef IMAPX200_ADS7846_SPI
	imapx200_add_device_ts();
#endif
}

#if defined(CONFIG_MACH_IMAPX200)
MACHINE_START(IMAPX200, "IMAPX200")
#endif
	.phys_io	= UART0_BASE_ADDR,
	.io_pg_offst	= (((u32)UART0_BASE_ADDR) >> 18) & 0xfffc,
	.boot_params	= IMAPX200_SDRAM_PA + 0x100,

	.init_irq	= imdkx200_init_irq,
	.map_io		= imapx200_map_io,
	.fixup		= imapx200_fixup,
	.timer		= &imapx200_init_timer,
	.init_machine	= imapx200_machine_init,
MACHINE_END	
