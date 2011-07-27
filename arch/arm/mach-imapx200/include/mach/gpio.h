/* arch/arm/mach-imap/include/mach/gpio.hi
 *
 * copyright (c) 2009~2014 ShangHai Infotm Ltd all rights reserved.
 * 
 * IMAPX200 - GPIO lib support
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/
#ifndef _IMAPX200_GENERIC_GPIO_H
#define _IMAPX200_GENERIC_GPIO_H

#define gpio_get_value	__gpio_get_value
#define gpio_set_value	__gpio_set_value
#define gpio_cansleep	__gpio_cansleep
#define gpio_to_irq	__gpio_to_irq

/*GPIO bank sizes*/
#define IMAPX200_GPIO_A_NR	(8)
#define IMAPX200_GPIO_B_NR	(5)
#define IMAPX200_GPIO_C_NR	(8)
#define IMAPX200_GPIO_D_NR	(5)
#define IMAPX200_GPIO_E_NR	(16)
#define IMAPX200_GPIO_F_NR	(10)
#define IMAPX200_GPIO_G_NR	(6)
#define IMAPX200_GPIO_H_NR	(4)
#define IMAPX200_GPIO_I_NR	(14)
#define IMAPX200_GPIO_J_NR	(9)
#define IMAPX200_GPIO_K_NR	(16)
#define IMAPX200_GPIO_L_NR	(13)
#define IMAPX200_GPIO_M_NR	(16)
#define IMAPX200_GPIO_N_NR	(13)
#define IMAPX200_GPIO_O_NR	(16)
#define IMAPX200_GPIO_P_NR	(12)
#define IMAPX200_GPIO_Q_NR	(6)
#define IMAPX200_GPIO_R_NR	(16)

/* GPIO bank numbes */

/* CONFIG_IMAP_GPIO_SPACE allows the user to select extra
 * space for debugging purposes so that any accidental
 * change from one gpio bank to another can be caught.
*/
#ifdef CONFIG_IMAP_GPIO_SPACE
#define IMAPX200_GPIO_NEXT(__gpio) \
	((__gpio##_START) + (__gpio##_NR) + CONFIG_IMAP_GPIO_SPACE + 1)
#else
#define IMAPX200_GPIO_NEXT(__gpio) \
	((__gpio##_START) + (__gpio##_NR) + 1)
#endif
enum imap_gpio_number {
	IMAPX200_GPIO_A_START = 0,
	IMAPX200_GPIO_B_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_A),
	IMAPX200_GPIO_C_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_B),
	IMAPX200_GPIO_D_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_C),
	IMAPX200_GPIO_E_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_D),
	IMAPX200_GPIO_F_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_E),
	IMAPX200_GPIO_G_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_F),
	IMAPX200_GPIO_H_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_G),
	IMAPX200_GPIO_I_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_H),
	IMAPX200_GPIO_J_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_I),
	IMAPX200_GPIO_K_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_J),
	IMAPX200_GPIO_L_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_K),
	IMAPX200_GPIO_M_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_L),
	IMAPX200_GPIO_N_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_M),
	IMAPX200_GPIO_O_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_N),
	IMAPX200_GPIO_P_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_O),
	IMAPX200_GPIO_Q_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_P),
	IMAPX200_GPIO_R_START = IMAPX200_GPIO_NEXT(IMAPX200_GPIO_Q),
};

/* IMAPX200 GPIO number definitions. */

#define IMAPX200_GPA(_nr)	(IMAPX200_GPIO_A_START + (_nr))
#define IMAPX200_GPB(_nr)	(IMAPX200_GPIO_B_START + (_nr))
#define IMAPX200_GPC(_nr)	(IMAPX200_GPIO_C_START + (_nr))
#define IMAPX200_GPD(_nr)	(IMAPX200_GPIO_D_START + (_nr))
#define IMAPX200_GPE(_nr)	(IMAPX200_GPIO_E_START + (_nr))
#define IMAPX200_GPF(_nr)	(IMAPX200_GPIO_F_START + (_nr))
#define IMAPX200_GPG(_nr)	(IMAPX200_GPIO_G_START + (_nr))
#define IMAPX200_GPH(_nr)	(IMAPX200_GPIO_H_START + (_nr))
#define IMAPX200_GPI(_nr)	(IMAPX200_GPIO_I_START + (_nr))
#define IMAPX200_GPJ(_nr)	(IMAPX200_GPIO_J_START + (_nr))
#define IMAPX200_GPK(_nr)	(IMAPX200_GPIO_K_START + (_nr))
#define IMAPX200_GPL(_nr)	(IMAPX200_GPIO_L_START + (_nr))
#define IMAPX200_GPM(_nr)	(IMAPX200_GPIO_M_START + (_nr))
#define IMAPX200_GPN(_nr)	(IMAPX200_GPIO_N_START + (_nr))
#define IMAPX200_GPO(_nr)	(IMAPX200_GPIO_O_START + (_nr))
#define IMAPX200_GPP(_nr)	(IMAPX200_GPIO_P_START + (_nr))
#define IMAPX200_GPQ(_nr)	(IMAPX200_GPIO_Q_START + (_nr))
#define IMAPX200_GPR(_nr)	(IMAPX200_GPIO_R_START + (_nr))

/* the end of the IMAPX200 specific gpios */
#define IMAPX200_GPIO_END	(IMAPX200_GPR(IMAPX200_GPIO_R_NR) + 1)
#define IMAP_GPIO_END		IMAPX200_GPIO_END

/* define the number of gpios we need to the one after the GPQ() range */
#define ARCH_NR_GPIOS	(IMAPX200_GPR(IMAPX200_GPIO_R_NR) + 1)

int imapx200_gpio_setpull(unsigned int pin, int pull);
int imapx200_gpio_cfgpin(unsigned int pin, unsigned int config);

#include <asm-generic/gpio.h>


#endif /* _IMAPX2000_GENERIC_GPIO_H */


