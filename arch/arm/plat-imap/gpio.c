/* arch/arm/plat-imapx200/gpiolib.c
 *
 * Copyright 2008 Openmoko, Inc.
 * Copyright 2008 Simtec Electronics
 *      Ben Dooks <ben@simtec.co.uk>
 *      http://armlinux.simtec.co.uk/
 *
 * imapx200 - GPIOlib support 
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/kernel.h>
#include <linux/irq.h>
#include <linux/io.h>

#include <asm-generic/gpio.h>
#include <mach/gpio.h>
#include <plat/regs-gpio.h>

/*
	使用方法：
	gpio_request(IMAPX200_GPI(1), "sensor g1")
	gpio_direction_input(IMAPX200_GPI(1));
	imapx200_gpio_setpull(IMAPX200_GPI(1), 0);
	tmp1 = gpio_get_value(IMAPX200_GPI(1));
 */


//#define IMAPX200_GPIO_DBG

#ifdef IMAPX200_GPIO_DBG
#define imapx200_gpio_dbg(format, arg...)		printk(format , ## arg)
#else
#define imapx200_gpio_dbg(format, arg...)		NULL
#endif	
/* GPIO bank summary:
 *
 * Bank	GPIOs	Style	SlpCon	ExtInt Group
 * A	8	4Bit	Yes	1
 * B	7	4Bit	Yes	1
 * C	8	4Bit	Yes	2
 * D	5	4Bit	Yes	3
 * E	5	4Bit	Yes	None
 * F	16	2Bit	Yes	4 [1]
 * G	7	4Bit	Yes	5
 * H	10	4Bit[2]	Yes	6
 * I	16	2Bit	Yes	None
 * J	12	2Bit	Yes	None
 * K	16	4Bit[2]	No	None
 * L	15	4Bit[2] No	None
 * M	6	4Bit	No	IRQ_EINT
 * N	16	2Bit	No	IRQ_EINT
 * O	16	2Bit	Yes	7
 * P	15	2Bit	Yes	8
 * Q	9	2Bit	Yes	9
 *
 * [1] BANKF pins 14,15 do not form part of the external interrupt sources
 * [2] BANK has two control registers, GPxCON0 and GPxCON1
 */

 
/**
 * struct s3c_gpio_cfg GPIO configuration
 * @cfg_eint: Configuration setting when used for external interrupt source
 * @get_pull: Read the current pull configuration for the GPIO
 * @set_pull: Set the current pull configuraiton for the GPIO
 * @set_config: Set the current configuration for the GPIO
 * @get_config: Read the current configuration for the GPIO
 *
 * Each chip can have more than one type of GPIO bank available and some
 * have different capabilites even when they have the same control register
 * layouts. Provide an point to vector control routine and provide any
 * per-bank configuration information that other systems such as the
 * external interrupt code will need.
 */
struct imapx200_gpio_cfg {
	unsigned int	cfg_eint;

	int (*get_pull)(struct imapx200_gpio_chip *chip, unsigned int offs);
	int (*set_pull)(struct imapx200_gpio_chip *chip, unsigned int offs,int pull);
	int (*set_config)(struct imapx200_gpio_chip *chip, unsigned offs,unsigned config);
};

struct imapx200_gpio_chip {
		void __iomem		*base;
		struct imapx200_gpio_cfg  *config;

		struct gpio_chip	chip;
#ifdef CONFIG_PM
		u32 		pm_save[4];
#endif
	};

int imapx200_gpio_setpull_updown(struct imapx200_gpio_chip *chip,unsigned int off, int pull)
{
	void __iomem *reg = chip->base + 0x08;
	int shift = off;
	u32 pup;

	imapx200_gpio_dbg("imapx200_gpio_setpull_updown : reg = 0x%x, shift = 0x%x,pull = 0x%x\r\n",reg,shift,pull);

	pup = __raw_readl(reg);
	pup &= ~(1 << shift);
	pup |= pull << shift;
	__raw_writel(pup, reg);

	return 0;
}

int imapx200_gpio_getpull_updown(struct imapx200_gpio_chip *chip,unsigned int off)
{
	void __iomem *reg = chip->base + 0x08;
	int shift = off;
	u32 pup = __raw_readl(reg);

	pup >>= shift;
	pup &= 0x1;
	return (__force int)pup;
}

// con
int imapx200_gpio_setcfg_zt(struct imapx200_gpio_chip *chip,
			    unsigned int off, unsigned int cfg)
{
	void __iomem *reg = chip->base + 0x4;
	unsigned int shift = off * 2;
	u32 con;

	con = __raw_readl(reg);
	con &= ~(0x3 << shift);
	con |= (cfg << shift);
	__raw_writel(con, reg);

	return 0;
}


static struct imapx200_gpio_cfg gpio_4bit_cfg_eint0111 = {
	.cfg_eint	= 7,
	.set_pull	= imapx200_gpio_setpull_updown,
	.get_pull	= imapx200_gpio_getpull_updown,
	.set_config	= imapx200_gpio_setcfg_zt,
};

#define IMAPX200_GPIO_CHIP(base_gpio, gpio_config, name, base_gpio_0, nr_gpio)			\
	{								\
		.base = base_gpio,			\
		.config = gpio_config, 				\
		.chip = {						\
			.label		  = name,			\
			.base		  = base_gpio_0,			\
			.ngpio		  = nr_gpio,			\
		},							\
	}

static struct imapx200_gpio_chip gpio_2bit[] = {
	IMAPX200_GPIO_CHIP(IMAP_GPA_BASE,&gpio_4bit_cfg_eint0111,"GPA", IMAPX200_GPA(0), IMAPX200_GPIO_A_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPB_BASE,&gpio_4bit_cfg_eint0111,"GPB", IMAPX200_GPB(0), IMAPX200_GPIO_B_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPC_BASE,&gpio_4bit_cfg_eint0111,"GPC", IMAPX200_GPC(0), IMAPX200_GPIO_C_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPD_BASE,&gpio_4bit_cfg_eint0111,"GPD", IMAPX200_GPD(0), IMAPX200_GPIO_D_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPE_BASE,&gpio_4bit_cfg_eint0111,"GPE", IMAPX200_GPE(0), IMAPX200_GPIO_E_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPF_BASE,&gpio_4bit_cfg_eint0111,"GPF", IMAPX200_GPF(0), IMAPX200_GPIO_F_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPG_BASE,&gpio_4bit_cfg_eint0111,"GPG", IMAPX200_GPG(0), IMAPX200_GPIO_G_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPH_BASE,&gpio_4bit_cfg_eint0111,"GPH", IMAPX200_GPH(0), IMAPX200_GPIO_H_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPI_BASE,&gpio_4bit_cfg_eint0111,"GPI", IMAPX200_GPI(0), IMAPX200_GPIO_I_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPJ_BASE,&gpio_4bit_cfg_eint0111,"GPJ", IMAPX200_GPJ(0), IMAPX200_GPIO_J_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPK_BASE,&gpio_4bit_cfg_eint0111,"GPK", IMAPX200_GPK(0), IMAPX200_GPIO_K_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPL_BASE,&gpio_4bit_cfg_eint0111,"GPL", IMAPX200_GPL(0), IMAPX200_GPIO_L_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPM_BASE,&gpio_4bit_cfg_eint0111,"GPM", IMAPX200_GPM(0), IMAPX200_GPIO_M_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPN_BASE,&gpio_4bit_cfg_eint0111,"GPN", IMAPX200_GPN(0), IMAPX200_GPIO_N_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPO_BASE,&gpio_4bit_cfg_eint0111,"GPO", IMAPX200_GPO(0), IMAPX200_GPIO_O_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPP_BASE,&gpio_4bit_cfg_eint0111,"GPP", IMAPX200_GPP(0), IMAPX200_GPIO_P_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPQ_BASE,&gpio_4bit_cfg_eint0111,"GPQ", IMAPX200_GPQ(0), IMAPX200_GPIO_Q_NR),
	IMAPX200_GPIO_CHIP(IMAP_GPR_BASE,&gpio_4bit_cfg_eint0111,"GPR", IMAPX200_GPR(0), IMAPX200_GPIO_R_NR),
};

static inline struct imapx200_gpio_chip *to_imapx200_gpio(struct gpio_chip *gpc)
{
	return container_of(gpc, struct imapx200_gpio_chip, chip);
}


struct imapx200_gpio_chip *imapx200_gpios[IMAP_GPIO_END];

static inline struct imapx200_gpio_chip *imapx200_gpiolib_getchip(unsigned int chip)
{
	return (chip < IMAP_GPIO_END) ? imapx200_gpios[chip] : NULL;
}


/*
 * 
 */
static inline int imapx200_gpio_do_setpull(struct imapx200_gpio_chip *chip,
				      unsigned int off, int pull)
{
	return (chip->config->set_pull)(chip, off, pull);
}
static inline int imapx200_gpio_do_setcfg(struct imapx200_gpio_chip *chip,
				     unsigned int off, unsigned int config)
{
	return (chip->config->set_config)(chip, off, config);
}


int imapx200_gpio_setpull(unsigned int pin, int pull)
{
	struct imapx200_gpio_chip *chip = imapx200_gpiolib_getchip(pin);
	unsigned long flags;
	int offset, ret;

	if (!chip)
		return -EINVAL;

	offset = pin - chip->chip.base;

	local_irq_save(flags);
	ret = imapx200_gpio_do_setpull(chip, offset, pull);
	local_irq_restore(flags);

	return ret;
}
EXPORT_SYMBOL(imapx200_gpio_setpull);

int imapx200_gpio_cfgpin(unsigned int pin, unsigned int config)
{
	struct imapx200_gpio_chip *chip = imapx200_gpiolib_getchip(pin);
	unsigned long flags;
	int offset;
	int ret;

	if (!chip)
		return -EINVAL;

	offset = pin - chip->chip.base;

	local_irq_save(flags);
	ret = imapx200_gpio_do_setcfg(chip, offset, config);
	local_irq_restore(flags);

	return ret;
}


static __init void imapx200_gpiolib_track(struct imapx200_gpio_chip *chip)
{
	unsigned int gpn;
	int i;

	gpn = chip->chip.base;
	for (i = 0; i < chip->chip.ngpio; i++, gpn++) {
		BUG_ON(gpn > ARRAY_SIZE(imapx200_gpios));
		imapx200_gpios[gpn] = chip;
	}
}


/* Default routines for controlling GPIO, based on the original imapx20024XX
 * GPIO functions which deal with the case where each gpio bank of the
 * chip is as following:
 *
 * base + 0x00: Control register, 2 bits per gpio
 *	        gpio n: 2 bits starting at (2*n)
 *		00 = input, 01 = output, others mean special-function
 * base + 0x04: Con register, 1 bit per gpio
 *		bit n: data bit n
*/
//GPADAT : one bit 
//GPACON : two bit
//GPAPUD : one bit
static int imapx200_gpiolib_input(struct gpio_chip *chip, unsigned offset)
{
	struct imapx200_gpio_chip *ourchip = to_imapx200_gpio(chip);
	void __iomem *base = ourchip->base;
	unsigned long flags;
	unsigned long con;

	local_irq_save(flags);

	con = __raw_readl(base + 0x04); //CON
	con &= ~(3 << (offset * 2));

	__raw_writel(con, base + 0x04); // CON

	local_irq_restore(flags);
	return 0;
}

static int imapx200_gpiolib_output(struct gpio_chip *chip,
			      unsigned offset, int value)
{
	struct imapx200_gpio_chip *ourchip = to_imapx200_gpio(chip);
	void __iomem *base = ourchip->base;
	unsigned long flags;
	unsigned long dat;
	unsigned long con;

	local_irq_save(flags);

	dat = __raw_readl(base + 0x00);
	dat &= ~(1 << offset);
	if (value)
		dat |= 1 << offset;
	__raw_writel(dat, base + 0x00);

	con = __raw_readl(base + 0x04);
	con &= ~(3 << (offset * 2));
	con |= 1 << (offset * 2);

	__raw_writel(con, base + 0x04);
	__raw_writel(dat, base + 0x00);

	local_irq_restore(flags);
	return 0;
}

static void imapx200_gpiolib_set(struct gpio_chip *chip,
			    unsigned offset, int value)
{
	struct imapx200_gpio_chip *ourchip = to_imapx200_gpio(chip);
	void __iomem *base = ourchip->base;
	unsigned long flags;
	unsigned long dat;

	local_irq_save(flags);

	dat = __raw_readl(base + 0x0);
	dat &= ~(1 << offset);
	if (value)
		dat |= 1 << offset;
	__raw_writel(dat, base + 0x00);

	local_irq_restore(flags);
}

static int imapx200_gpiolib_get(struct gpio_chip *chip, unsigned offset)
{
	struct imapx200_gpio_chip *ourchip = to_imapx200_gpio(chip);
	unsigned long val;

	val = __raw_readl(ourchip->base + 0x00);
	val >>= offset;
	val &= 1;

	return val;
}

__init void imapx200_gpiolib_add(struct imapx200_gpio_chip *chip)
{
	struct gpio_chip *gc = &chip->chip;
	int ret;

	BUG_ON(!chip->base);
	BUG_ON(!gc->label);
	BUG_ON(!gc->ngpio);

	if (!gc->direction_input)
		gc->direction_input = imapx200_gpiolib_input;
	if (!gc->direction_output)
		gc->direction_output = imapx200_gpiolib_output;
	if (!gc->set)
		gc->set = imapx200_gpiolib_set;
	if (!gc->get)
		gc->get = imapx200_gpiolib_get;

#ifdef CONFIG_PM_1
	if (chip->pm != NULL) {
		if (!chip->pm->save || !chip->pm->resume)
			printk(KERN_ERR "gpio: %s has missing PM functions\n",
			       gc->label);
	} else
		printk(KERN_ERR "gpio: %s has no PM function\n", gc->label);
#endif

	/* gpiochip_add() prints own failure message on error. */
	ret = gpiochip_add(gc);
	if (ret >= 0)
		imapx200_gpiolib_track(chip);
}


static __init int imapx200_gpiolib_init(void)
{
	int i = 0;
	
	for (i = 0 ; i< ARRAY_SIZE(gpio_2bit); i++) {
		imapx200_gpiolib_add(&gpio_2bit[i]);
	}

	return 0;
}

core_initcall(imapx200_gpiolib_init);

