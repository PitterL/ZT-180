
/* linux/arch/arm/plat-imapx200/pm-gpio.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *	http://armlinux.simtec.co.uk/
 *
 * IMAPX200 series GPIO PM code
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#include <linux/kernel.h>
#include <linux/sysdev.h>
#include <linux/init.h>
#include <linux/io.h>
#include <linux/gpio.h>

#include <mach/imapx_gpio.h>
#include <plat/pm.h>

/* PM GPIO helpers */

#define OFFS_CON	(0x00)
#define OFFS_DAT	(0x04)
#define OFFS_UP		(0x08)
#define IMAPX200_BANKS_END 18
struct imapx200_gpio_chip {
	void __iomem		*dat;
	void __iomem		*con;
#ifdef CONFIG_PM
	u32			pm_save[4];
#endif
};


static struct imapx200_gpio_chip imapx200_gpio[] = {
	{
		.dat	= rGPADAT,
		.con	= rGPACON,
	}, {
		.dat	= rGPBDAT,
		.con	= rGPBCON,
	}, {
		.dat	= rGPCDAT,
		.con	= rGPCCON,
	}, {
		.dat	= rGPDDAT,
		.con	= rGPDCON,
	}, {
		.dat	= rGPEDAT,
		.con	= rGPECON,
	}, {
		.dat	= rGPFDAT,
		.con	= rGPFCON,
	}, {
		.dat	= rGPGDAT,
		.con	= rGPGCON,
	}, {
		.dat	= rGPHDAT,
		.con	= rGPHCON,
	}, {
		.dat	= rGPIDAT,
		.con	= rGPICON,
	}, {
		.dat	= rGPJDAT,
		.con	= rGPJCON,
	}, {
		.dat	= rGPKDAT,
		.con	= rGPKCON,
	}, {
		.dat	= rGPLDAT,
		.con	= rGPLCON,
	}, {
		.dat	= rGPMDAT,
		.con	= rGPMCON,
	}, {
		.dat	= rGPNDAT,
		.con	= rGPNCON,
	}, {
		.dat	= rGPODAT,
		.con	= rGPOCON,
	}, {
		.dat	= rGPPDAT,
		.con	= rGPPCON,
	}, {
		.dat	= rGPQDAT,
		.con	= rGPQCON,
	},
};


static void imapx200_gpio_pm_save(struct imapx200_gpio_chip *chip)
{
	chip->pm_save[0] = __raw_readl(chip->dat);
	chip->pm_save[1] = __raw_readl(chip->con);
}

static void imapx200_gpio_pm_resume(struct imapx200_gpio_chip *chip)
{
	u32 old_gpcon = __raw_readl(chip->dat);
	u32 old_gpdat = __raw_readl(chip->con);
	u32 gps_gpcon = chip->pm_save[0];
	u32 gps_gpdat = chip->pm_save[1];
	u32 gpcon;

	/* GPACON only has one bit per control / data and no PULLUPs.
	 * GPACON[x] = 0 => Output, 1 => SFN */

	/* first set all SFN bits to SFN */

	gpcon = old_gpcon | gps_gpcon;
	__raw_writel(gpcon, chip->dat);

	/* now set all the other bits */

	__raw_writel(gps_gpdat, chip->dat);
	__raw_writel(gps_gpcon, chip->con);
}
/**
 * imapx200_pm_save_gpio() - save gpio chip data for suspend
 * @ourchip: The chip for suspend.
 */
static void imapx200_pm_save_gpio(struct imapx200_gpio_chip *ourchip)
{
	imapx200_gpio_pm_save(ourchip);
}

/**
 * imapx200_pm_save_gpios() - Save the state of the GPIO banks.
 *
 * For all the GPIO banks, save the state of each one ready for going
 * into a suspend mode.
 */
void imapx200_pm_save_gpios(void)
{
	unsigned int gpio_nr;

	for (gpio_nr = 0; gpio_nr < IMAPX200_BANKS_END; gpio_nr++) {
		imapx200_pm_save_gpio(&imapx200_gpio);
	}
}

/**
 * imapx200_pm_resume_gpio() - restore gpio chip data after suspend
 * @ourchip: The suspended chip.
 */
static void imapx200_pm_resume_gpio(struct imapx200_gpio_chip *ourchip)
{
	imapx200_gpio_pm_resume(ourchip);
}

void imapx200_pm_restore_gpios(void)
{
	struct imapx200_gpio_chip *ourchip;
	unsigned int gpio_nr;

	for (gpio_nr = 0; gpio_nr < IMAPX200_GPIO_END; gpio_nr++) {
		imapx200_pm_resume_gpio(ourchip);
	}
}
