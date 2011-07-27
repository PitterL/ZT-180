/*
 * arch/arm/mach-imapx200/include/mach/entry-macro.S
 *
 * Low-level IRQ helper macros for IMAP-based platforms
 *
 * This file is licensed under  the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
*/

#define INTPND1		(0x10)
#define INTPND2		(0x34)
#define INTOFFSET	(0x14)

#include <mach/hardware.h>
#include <asm/irq.h>

#if defined (CONFIG_CPU_IMAPX200)
	.macro  get_irqnr_preamble, base, tmp
	.endm

	.macro  arch_ret_to_user, tmp1, tmp2
	.endm

	.macro	get_irqnr_and_base, irqnr, irqstat, base, tmp
	
	ldr	\base, =IMAP_VA_IRQ
	@check INTPND1
	ldr \irqstat, [ \base, # INTPND1 ]
	teq \irqstat, #0

	@otherwise try INTPND2
	ldreq   \irqstat, [ \base, # INTPND2 ]
	teqeq   \irqstat, #0

	ldrne	\tmp,	[ \base, # INTOFFSET]
	movne   \irqnr, \tmp
	.endm

	/* currently don't need an disable_fiq macro */
	.macro	disable_fiq
	.endm

	/* we don't have an irq priority table */
	.macro irq_prio_table
	.endm

#endif

