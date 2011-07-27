/* arch/arm/mach-imap/include/mach/timex.h
 *
 * Copyright (c) 2003-2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#if 0
#ifndef __ASM_ARCH_TIMEX_H
#define __ASM_ARCH_TIMEX_H

/* CLOCK_TICK_RATE needs to be evaluatable by the cpp, so making it
 * a variable is useless. It seems as long as we make our timers an
 * exact multiple of HZ, any value that makes a 1->1 correspondence
 * for the time conversion functions to/from jiffies is acceptable.
*/


#define CLOCK_TICK_RATE 12000000


#endif /* __ASM_ARCH_TIMEX_H */
#endif
/* linux/include/asm-arm/arch-s3c2410/timex.h
 *
 * Copyright (c) 2003-2005 Simtec Electronics
 *	Ben Dooks <ben@simtec.co.uk>
 *
 * S3C2410 - time parameters
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
*/

#ifndef __ASM_ARCH_TIMEX_H
#define __ASM_ARCH_TIMEX_H

/* CLOCK_TICK_RATE needs to be evaluatable by the cpp, so making it
 * a variable is useless. It seems as long as we make our timers an
 * exact multiple of HZ, any value that makes a 1->1 correspondence
 * for the time conversion functions to/from jiffies is acceptable.
*/
#if defined (CONFIG_CPU_S3C6400) || defined (CONFIG_CPU_S3C6410) 

#define PRESCALER 4
#define DIVIDER 1

/*you have to  confirm  the default clock source frequency. */
#define PCLK_INPUT 	66750000

/* CLOCK_TICK_RATE is the time of a timer count desence, minsung says*/
#define CLOCK_TICK_RATE  (PCLK_INPUT / ((PRESCALER + 1) * DIVIDER))


/*the scaled value (CLOCK_TICK_RATE) must be set under 0.5uS 
    Clock source 66,750,000 /(16*2) = 2,085,937Hz --> 0.47uS resolution [OK]
    but when  PRESCALE x MUX_VAL  =  32*2 = 1,042,968Hz --> 0.958uS resolution, [NG]
           this time you meet the abnormal operation and slower 
    so please check the CLOCK_TICK_RATE > 2,000,000  , by Laputa */
#elif defined (CONFIG_CPU_S3C2450) || defined (CONFIG_CPU_S3C2416) || defined (CONFIG_CPU_S3C2443) 
#define PRESCALE		16
#define MUX4_DIV		S3C2410_TCFG1_MUX4_DIV2
#define MUX4_VAL		2

/*you have to  confirm  the default clock source frequency. */
#define PCLK_INPUT 	66750000
#define CLOCK_TICK_RATE  (PCLK_INPUT / ((PRESCALE+1) * MUX4_VAL))

#else
//#define PRESCALER 	((6-1)/2)
#define PRESCALER 7
#define DIVIDER 1

#define CLOCK_TICK_RATE 12000000

#endif

#endif /* __ASM_ARCH_TIMEX_H */
