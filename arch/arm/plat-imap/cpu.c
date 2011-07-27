/********************************************************************************
 ** linux-2.6.28.5/arch/arm/plat-imap/cpu.c
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
 **     1.2  25/11/2009    Raymond Wang
 ********************************************************************************/


#include <linux/init.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/sysdev.h>
#include <linux/ioport.h>
#include <linux/serial_core.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/delay.h>

#include <mach/hardware.h>
#include <asm/irq.h>
#include <asm/cacheflush.h>

#include <asm/mach/arch.h>
#include <asm/mach/map.h>

//#include <mach/system-reset.h>

//#include <mach/regs-gpio.h>
#include <plat/cpu.h>
#include <plat/devs.h>
#include <plat/clock.h>
#include <plat/imapx200.h>
#include <plat/fpga_test.h>
#include <plat/imapx.h>
#include <plat/regs-serial.h>

struct chip_tab {
	unsigned long	idcode;
	unsigned long	idmask;
	void		(*map_io)(struct map_desc *mach_desc, int size);
	void		(*init_irq)(void);
	void		(*init_uarts)(struct imapx200_uartcfg *cfg, int no);   //uncertain
	void		(*init_clocks)(int xtal);
	int		(*init)(void);
	const char	*name;
};


/* table of supported CPUs */

static const char name_imapx200[]  = "IMAPX200";

static struct chip_tab cpu_ids[] __initdata = {
	{
		.idcode         = 0x13AB2000,
		.idmask         = 0xfffffff0,
		.map_io         = imdkx200_map_io,
		.init_irq	= imdkx200_init_irq,
		.init_clocks    = imapx200_init_clocks,
		.init_uarts     = imapx200_init_uarts,
		.init           = imapx200_init,
		.name           = name_imapx200
	},
};


/* minimal IO mapping */
static struct map_desc imap_iodesc[] __initdata = {
	{
		.virtual	= (unsigned long)IMAP_VA_GPIO,
		.pfn		= __phys_to_pfn(GPIO_BASE_REG_PA),
		.length		= SZ_1M,
		.type		= MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_IRQ,
		.pfn		= __phys_to_pfn(INTR_BASE_REG_PA),
		.length		=  SZ_1M,
		.type		= MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_UART,
		.pfn		= __phys_to_pfn(UART0_BASE_ADDR),
		.length		= SZ_16K,
		.type           = MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_TIMER,
		.pfn		= __phys_to_pfn(TIMER_BASE_REG_PA),
		.length		= SZ_4K,
		.type           = MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_SYSMGR,
		.pfn		= __phys_to_pfn(SYSMGR_BASE_REG_PA),
		.length		= SZ_1M,
		.type           = MT_DEVICE,
	},{
		.virtual	= (unsigned long)IMAP_VA_FB,
		.pfn		= __phys_to_pfn(LCD_BASE_REG_PA),
		.length		= SZ_16K,
		.type           = MT_DEVICE,
	},


};

static struct chip_tab * __init imap_lookup_cpu(unsigned long idcode)
{
	struct chip_tab *tab;
	int count;

	tab = cpu_ids;
	for (count = 0; count < ARRAY_SIZE(cpu_ids); count++, tab++) {
		if ((idcode & tab->idmask) == tab->idcode)
			return tab;
	}

	return NULL;
}


/* board information */
static struct imap_board *board;
void imap_set_board(struct imap_board *b)
{
	int i;

	board = b;

	if (b->clocks_count != 0) {
		struct clk **ptr = b->clocks;

		for (i = b->clocks_count; i > 0; i--, ptr++)
			if (imap_register_clock(*ptr) < 0)
				printk(KERN_ERR "failed to register clock.\n");
	}
}


/* cpu information */

static struct chip_tab *cpu;

static unsigned long imap_read_idcode_v6(void)
{
	return 0x13AB2000;
}

static unsigned long imap_read_idcode_v5(void)
{
	return 1UL;	/* don't look like an 2400 */
}

static unsigned long imap_read_idcode_v4(void)
{
	return 0UL;
}

void __init imap_init_irq(void)
{
	if (cpu == NULL)
		panic("imap_init_clocks: no cpu setup?\n");

	if (cpu->init_irq == NULL)
		panic("imap_init_clocks: cpu has no clock init\n");
	else
		(cpu->init_irq)();

}

void __init imap_init_io(struct map_desc *mach_desc, int size)
{
	unsigned long idcode = 0x0;

	/* initialise the io descriptors we need for initialisation */
	iotable_init(imap_iodesc, ARRAY_SIZE(imap_iodesc));

	if (cpu_architecture() >= CPU_ARCH_ARMv6) {
		idcode = imap_read_idcode_v6();
	} else if (cpu_architecture() >= CPU_ARCH_ARMv5) {
		idcode = imap_read_idcode_v5();
	} else if (cpu_architecture() >= CPU_ARCH_ARMv4) {
		idcode = imap_read_idcode_v4();
	} else  {
		panic("Unknown CPU Architecture");
		idcode = 1UL; /* Unknown and error */
	}

	cpu = imap_lookup_cpu(idcode);

	if (cpu == NULL) {
		printk(KERN_ERR "Unknown CPU type 0x%08lx\n", idcode);
		panic("Unknown IMAP CPU");
	}

	printk("CPU %s (id 0x%08lx)\n", cpu->name, idcode);

	if (cpu->map_io == NULL || cpu->init == NULL) {
		printk(KERN_ERR "CPU %s support not enabled\n", cpu->name);
		panic("Unsupported IMAP CPU");
	}

	//	arm_pm_restart = s3c24xx_pm_restart;

	(cpu->map_io)(mach_desc, size);
}

/* uart management */

static int nr_uarts __initdata = 0;

static struct imapx200_uartcfg uart_cfgs[4];

/* imap_init_uartdevs
 *
 * copy the specified platform data and configuration into our central
 * set of devices, before the data is thrown away after the init process.
 *
 * This also fills in the array passed to the serial driver for the
 * early initialisation of the console.
 */
void __init imap_init_uartdevs(char *name, struct imap_uart_resources *res,
		struct imapx200_uartcfg *cfg, int no)
{
	struct platform_device *platdev;
	struct imapx200_uartcfg *cfgptr = uart_cfgs;
	struct imap_uart_resources *resp;
	int uart;

	memcpy(cfgptr, cfg, sizeof(struct imapx200_uartcfg) * no);

	for (uart = 0; uart < no; uart++, cfg++, cfgptr++)
	{
		platdev = imap_uart_src[cfgptr->hwport];

		resp = res + cfgptr->hwport;

		imap_uart_devs[uart] = platdev;

		platdev->name = name;
		platdev->resource = resp->resources;
		platdev->num_resources = resp->nr_resources;

		platdev->dev.platform_data = cfgptr;
	}

	nr_uarts = no;
}

void __init imap_init_uarts(struct imapx200_uartcfg *cfg, int no)
{
	if (cpu == NULL)
		return;

	if (cpu->init_uarts == NULL)
	{
		printk(KERN_ERR "imap_init_uarts: cpu has no uart init\n");
	}
	else
		(cpu->init_uarts)(cfg, no);
}


/* imap_init_clocks
 *
 * Initialise the clock subsystem and associated information from the
 * given master crystal value.
 *
 * xtal  = 0 -> use default PLL crystal value (normally 12MHz)
 *      != 0 -> PLL crystal value in Hz
 */

void __init imap_init_clocks(int xtal)
{
#if defined (FPGA_TEST)
	if (xtal == 0)
		xtal = 12*1000*1000;
#else
	xtal = 40*1000*1000;
#endif
	if (cpu == NULL)
		panic("imap_init_clocks: no cpu setup?\n");

	if (cpu->init_clocks == NULL)
		panic("imap_init_clocks: cpu has no clock init\n");
	else
		(cpu->init_clocks)(xtal);
}





static int __init imap_arch_init(void)
{
	int ret;

	// do the correct init for cpu

	if (cpu == NULL)
		panic("imap_arch_init: NULL cpu\n");

	ret = (cpu->init)();
	if (ret != 0)
		return ret;

	ret = platform_add_devices(imap_uart_devs, nr_uarts);
	if (ret != 0)
		return ret;
	
	printk(KERN_INFO "leaving imap_arch_init\n");
	return ret;
}

arch_initcall(imap_arch_init);

struct sysdev_class imap_sysclass = { 
        .name   = "imap-core",
};

static struct sys_device imap_sysdev = { 
        .cls    = &imap_sysclass,
};


static __init int imap_sysdev_init(void)
{
        sysdev_class_register(&imap_sysclass);
        return sysdev_register(&imap_sysdev);
}

core_initcall(imap_sysdev_init);
