#include <linux/linkage.h>
#include <asm/assembler.h>
#include <mach/imap_addr.h>
/*
#undef S3C64XX_VA_GPIO
#define S3C64XX_VA_GPIO (0x0)

#include <plat/regs-gpio.h>
#include <plat/gpio-bank-n.h>
*/
#define LL_UART (IMAP_VA_UART + (0x1000 * CONFIG_IMAP_LOWLEVEL_UART_PORT) + 0x0c)

	.text

	/* imapx200_cpu_save
	 *
	 * Save enough processor state to allow the restart of the pm.c
	 * code after resume.
	 *
	 * entry:
	 *	r0 = pointer to the save block
	*/

ENTRY(imapx200_cpu_save)
	stmfd	sp!, { r4 - r12, lr }

	mrc	p15, 0, r4, c13, c0, 0	@ FCSE/PID
	mrc	p15, 0, r5, c3, c0, 0	@ Domain ID
	mrc	p15, 0, r6, c2, c0, 0	@ Translation Table BASE0
	mrc	p15, 0, r7, c2, c0, 1	@ Translation Table BASE1
	mrc	p15, 0, r8, c2, c0, 2	@ Translation Table Control
	mrc	p15, 0, r9, c1, c0, 0	@ Control register
	mrc	p15, 0, r10, c1, c0, 1	@ Auxiliary control register
	mrc	p15, 0, r11, c1, c0, 2	@ Co-processor access controls

	stmia	r0, { r4 - r13 }	@ Save CP registers and SP

	@@ save our state to ram
	bl	imapx200_pm_cb_flushcache

	@@ call final suspend code
	ldr	r0, =pm_cpu_sleep
	ldr	pc, [r0]
	
	@@ return to the caller, after the MMU is turned on.
	@@ restore the last bits of the stack and return.
resume_with_mmu:
	ldmfd	sp!, { r4 - r12, pc }	@ return, from sp from imapx200_cpu_save

	.data

	/* the next bit is code, but it requires easy access to the
	 * imapx200_sleep_save_phys data before the MMU is switched on, so
	 * we store the code that needs this variable in the .data where
	 * the value can be written to (the .text segment is RO).
	*/

	.global	imapx200_sleep_save_phys
imapx200_sleep_save_phys:
	.word	0

	/* Sleep magic, the word before the resume entry point so that the
	 * bootloader can check for a resumeable image. */

	.word	0x2bedf00d

	/* imapx200_cpu_reusme
	 *
	 * This is the entry point, stored by whatever method the bootloader
	 * requires to get the kernel runnign again. This code expects to be
	 * entered with no caches live and the MMU disabled. It will then
	 * restore the MMU and other basic CP registers saved and restart
	 * the kernel C code to finish the resume code.
	*/

ENTRY(imapx200_cpu_resume)
	msr	cpsr_c, #PSR_I_BIT | PSR_F_BIT | SVC_MODE
	ldr	r2, =LL_UART		/* for debug */

	/* __v6_setup from arch/arm/mm/proc-v6.S, ensure that the caches
	 * are thoroughly cleaned just in case the bootloader didn't do it
	 * for us. */
	mov	r0, #0
	mcr	p15, 0, r0, c7, c14, 0		@ clean+invalidate D cache
	mcr	p15, 0, r0, c7, c5, 0		@ invalidate I cache
	mcr	p15, 0, r0, c7, c15, 0		@ clean+invalidate cache
	mcr	p15, 0, r0, c7, c10, 4		@ drain write buffer
	@@mcr	p15, 0, r0, c8, c7, 0		@ invalidate I + D TLBs
	@@mcr	p15, 0, r0, c7, c7, 0		@ Invalidate I + D caches

	ldr	r0, imapx200_sleep_save_phys
	ldmia	r0, { r4 - r13 }

	mcr	p15, 0, r4, c13, c0, 0	@ FCSE/PID
	mcr	p15, 0, r5, c3, c0, 0	@ Domain ID
	mcr	p15, 0, r6, c2, c0, 0	@ Translation Table BASE0
	mcr	p15, 0, r7, c2, c0, 1	@ Translation Table BASE1
	mcr	p15, 0, r8, c2, c0, 2	@ Translation Table Control
	mcr	p15, 0, r10, c1, c0, 1	@ Auxiliary control register

	mov	r0, #0			@ restore copro access controls
	mcr	p15, 0, r11, c1, c0, 2	@ Co-processor access controls
	mcr 	p15, 0, r0, c7, c5, 4

	ldr	r2, =resume_with_mmu
	mcr	p15, 0, r9, c1, c0, 0		/* turn mmu back on */
	nop
	mov	pc, r2				/* jump back */

	.end
